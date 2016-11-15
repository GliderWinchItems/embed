/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : xb_api_rx.c
* Hackee             : deh
* Date First Issued  : 03/24/2012
* Board              : STM32F103VxT6_pod_mm with XBee
* Description        : Receive XBee frame
*******************************************************************************/

#include "libusartstm32/usartallproto.h"
#include "libopenstm32/usart.h"
#include "libmiscstm32/systick_delay.h"
#include "xb_common.h"

#define MAXFRAMESIZE	124
static u16 	xb_frame_bytect;	// Number of data bytes in frame
static u16 	state;			// State for building frame
static u16	state1;			// State, another one
static u16	ctr;			// Byte count as frame builds
static u16	size;			// Size of buffer
static char 	*pframe;		// Pointer into frame being built
static u32	checksum;		// Checksum (duh)



/* ************************************************************
 * static int place_byte(unsigned char c);
 * @brief	: Send command(s) to XB

 * @return	: byte count of frame received--
 *		:   0 = frame not ready
 * 		:   n = actual byte count of frame
 *		:   - = error in frame
***************************************************************/
static int place_byte(unsigned char c)
{
	switch (state1)
	{
	case 0: // MSB of byte count
		xb_frame_bytect = (c << 8);
		state1 = 1;
		return 0;

	case 1: // LSB of byte count
		xb_frame_bytect |= (c & 0xff);
		// Check for reasonable range 
		if ((xb_frame_bytect == 0) || (xb_frame_bytect >= size))
			{ state1 = 0; state = 0; return -1; } // Error: reset for next frame
		// Here the received byte count is reasonable
		checksum = 0;	// Initialize checksum
		ctr = 0;	// Count of bytes stored
		state1 = 2;	// Next phase is storing data
		return 0;	// Return frame not complete code

	case 2: // Data bytes
		*pframe++ = c;	// Store in caller's buffer
		checksum += c;	// Build checksum
		ctr += 1;	// Count'em cowboy
		if (ctr >= xb_frame_bytect) // Have we stored all the data bytes?
		{ // Here, yes.  The next byte (should) will be the checksum
			state1 = 3;
		}
		return 0;	// Return frame not complete code
		
	case 3:	// Checksum check
		state = 0;	// Reset for next frame
		state1 = 0; 	// Reset for next frame

		if ( ((c + checksum) & 0xff) == 0xff) // Does ours match?
		{ // Here, checksum is good.

			return xb_frame_bytect;	// Positive byte tells caller frame is ready and OK.	
		}
		return -3;	// Let caller know the checksum failed
	}
	return -9;	// Should not come here!
}

/* ************************************************************
 * int xb_getframe(char *p, u16 size1);
 * @brief	: Send command(s) to XB
 * @param	: p = pointer to buffer to receive frame
 * @param	: size = number of bytes in buffer 
 * @return	: byte count of frame received--
 *		:   0 = frame not ready
 * 		:   n = actual byte count of frame
 *		:   - = error in frame
***************************************************************/
int xb_getframe(char *p, u16 size1)
{
	unsigned char c;	// Next char
	unsigned int ct;	// Number of chars in USART input buffer
	
	/* The routine is polled by 'main'. */
	if ( (ct = USART2_rxdma_getcount() ) == 0) // Any received chars available?
		return 0;			// No, frame not ready

	
	/* Build the frame one char at a time */
	while (ct-- > 0) 	// We may run out of chars before frame completes
	{
		c = USART2_rxdma_getchar();	// Get next char from circular buffer
		switch (state)
		{
		case 0:	// Looking for start of frame (SOF) not preceded by an escape byte
			if (c == XBFRAMEDELIMITER)	// Is this the start? (0x7e)
			{ // Here, yes.  This must be SOF since it wasn't preceeded by an escape byte
				state = 1;		// Frame started, so handle escaped chars
				state1 = 0;		// Initial state for frame sequence
				pframe = p;		// Save pointer to caller's buffer
				size = size1;		// Save buffer size (to prevent overrun)
				return 0;
			}
			// Here, no we expected SOF but got something else.
			if (c == XBESCAPE)	// Escape means that if the following is 0x7e it is not SOF
				state = 3;
			return -99;
			break;

		case 1:	// Frame has started
			if (c == XBFRAMEDELIMITER)	// Is this the start? (0x7e)
			{ // Here, yes.  This must be SOF since it wasn't preceeded by an escape byte
				state = 1;		// Frame started, so handle escaped chars
				state1 = 0;		// Initial state for frame sequence
				pframe = p;		// Save pointer to caller's buffer
				size = size1;		// Save buffer size (to prevent overrun)
				return -90;
				break;
			}

			if (c == XBESCAPE) 	// Is this an escape byte?
			{ // Here, yes.  The next byte will be the xor'd data byte
				state = 2;
				break;
			}
			return place_byte(c);  // If this byte completes the frame, return the count.
			break;

		case 2:	// This byte is the xor'd data byte
			state = 1;
			return place_byte( (c ^ XBXOR) );	// Undo escaped byte
			break;			

		case 3: // Looking for start of frame, but prior char was an escape so ignore this one.
			state = 0;
			break;

		}
	}
	return 0;
}


