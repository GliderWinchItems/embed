/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : xb_api_tx.c
* Hackee             : deh
* Date First Issued  : 03/24/2012
* Board              : STM32F103VxT6_pod_mm with XBee
* Description        : Transmit with API mode
*******************************************************************************/

#include "libusartstm32/usartallproto.h"
#include "libopenstm32/usart.h"
#include "libopenstm32/common.h"
#include "xb_common.h"

/* ************************************************************
 * void xb_send_frame(char * p, int count);
 * @brief	: Prepare frame and send
 * @argument	: p = pointer to bytes to be sent
 * @argument	: count = number of bytes in frame less header
***************************************************************/
/* Page 58 of XBee manual--
Data bytes that need to be escaped:
   • 0x7E – Frame Delimiter
   • 0x7D – Escape
   • 0x11 – XON
   • 0x13 – XOFF
 Example - Raw UART Data Frame (before escaping interfering bytes):
              0x7E 0x00 0x02 0x23 0x11 0xCB
 0x11 needs to be escaped which results in the following frame:
 0x7E 0x00 0x02 0x23 0x7D 0x31 0xCB
Note: In the above example, the length of the raw data (excluding the checksum) is 0x0002 and
the checksum of the non-escaped data (excluding frame delimiter and length) is calculated as:
0xFF - (0x23 + 0x11) = (0xFF - 0x34) = 0xCB.
*/

void xb_send_frame(char * p, int count)
{
	int i;
	char c = 0;
	unsigned char checksum = 0;	// Initialize checksum

	/* Set header */
	USART2_txdma_putc(XBFRAMEDELIMITER);	// Start of frame
	USART2_txdma_putc(count >> 8); 		// Byte count (MSB)
	USART2_txdma_putc(count);		// Byte count (LSB)

	/* Add data */
	for (i = 0; i < count; i++)
	{
		c = *(p+i);
		checksum += c;

		/* Check if this character has to be escaped */
		if ((c == XBFRAMEDELIMITER) || (c == XBESCAPE) || (c == XBXON) || (c == XBXOFF))
		{ // Here, do an escape sequence for this char
			USART2_txdma_putc(XBESCAPE);		// Add Escape byte
			USART2_txdma_putc((c ^ XBXOR) );	// Add char xor'd
		}
		else
		{ // Here no escape sequence necessary
			USART2_txdma_putc(c);			// Add char straight-up
		}
	}

	/* Finalize checksum */
	checksum = (0xff - checksum);

	/* Check if checksum has to be escaped */
	if ((checksum == XBFRAMEDELIMITER) || (checksum == XBESCAPE) || (checksum == XBXON) || (checksum == XBXOFF))
	{ // Here, do an escape sequence for this char
		USART2_txdma_putc(XBESCAPE);		// Add to output buffer: Escape byte
		USART2_txdma_putc((checksum ^ XBXOR) );	// Add to output buffer: xor'd
	}
	else
	{ // Here no escape sequence necessary
		USART2_txdma_putc(checksum);		// Add to output buffer: unmodified
	}

	/* Start it sending */
	USART2_txdma_send();

	return;
}
/* ************************************************************
 * void xb_tx16(u16 addr, char * q, u16 count);
 * @brief	: Send RF packet, 16 bit addressing
 * @argument	: addr = address (0xffff = broadcast)
 * @argument	: q = pointer to bytes to be sent
 * @argument	: count = number of data bytes to send (addr, et al., not included)
***************************************************************/
void xb_tx16(u16 addr, char * q, u16 count)
{
	int i;
	char zz[112];		// Frame to be sent (max size plus "a little")
	char *p = &zz[0];	// Build frame on local stack

	*p++ = 0x01;			// TX 16
	*p++= 0;			// Frame ID.  0 = disable response frame
	*p++ = (u8)(addr >> 8); 		// MSB of destination address (broadcast = 0xffff)
	*p++ = (u8)(addr); 			// LSB of destination address
	*p++ = 0x01;			// Options: disable ACK

	if (count > 100) count = 100;	// Limit to max, jic we got a bogus count
	for (i = 0; i < count; i++)	// Copy in data bytes
		*p++ = *q++;
	xb_send_frame(zz, (count+5) );	// Prepare and send frame

	return;
}
/* ************************************************************
 * void xb_tx64(unsigned long long addr, char * q, int count);
 * @brief	: Send RF packet, 16 bit addressing
 * @argument	: addr = address (0xffff = broadcast)
 * @argument	: q = pointer to bytes to be sent
 * @argument	: count = number of data bytes to send (addr, et al., not included)
 ***************************************************************/
void xb_tx64(unsigned long long addr, char * q, u16 count)
{
	int i;
	char zz[116];		// Frame to be sent (max size plus "a little")
	char *p = &zz[0];	// Build frame on local stack

	*p++ = 0x00;			// TX 64 code
	*p++ = 0;			// Frame ID.  0 = disable response frame
	*p++ = addr >> 56;		// MSB of destination address
	*p++ = addr >> 48;		// 
	*p++ = addr >> 40;		// 
	*p++ = addr >> 32;		// 
	*p++ = addr >> 24;		// 
	*p++ = addr >> 16;		// 
	*p++ = addr >>  8;		//
	*p++ = addr;			// LSB of destination address
	*p++ = 0x01;			// Options: disable ACK

	if (count > 100) count = 100;	// Limit to max, jic we got a bogus count
	for (i = 0; i < count; i++)	// Copy in data bytes
		*p++ = *q++;
	xb_send_frame(zz, (count+11) );	// Prepare and send frame

	return;
}
/* ************************************************************
 * void xb_api_at(char * pAT);
 * @brief	: Send AT command to module (when in api mode)
 * @argument	: pointer to string with AT command, e.g. "DL 1")
 ***************************************************************/
void xb_api_at(char * pAT)
{
	int i = 0;
	char zz[116];			// Frame to be sent (max size plus "a little")
	char *p = &zz[0];		// Build frame on local stack

	*p++ = 0x08;			// API code for AT command
	*p++ = 0;			// Frame ID

	while ( (*p !=0 ) && (i++ <=  100) )
		*p++ = *pAT++;
	xb_send_frame(zz, (i + 2) ); // Prepare and send frame	
	return;
}
