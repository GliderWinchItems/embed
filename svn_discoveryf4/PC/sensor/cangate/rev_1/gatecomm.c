/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : gatecomm.c
* Author	     : deh
* Date First Issued  : 09/19/2013
* Board              : PC
* Description        : Routines related to communications between the gateway and PC
*******************************************************************************/
/* 

This code originated from--
~/svn/sensor/sw_f103/trunk/lib/libsensormisc/USART1_PC_gateway
These routines were included in canldr.c, and modified, mostly by removing the USART calls
and making the appropriate changes for the PC "loop" that waits for interrupts.

After debugging, these routines were lifted and placed in 'gatecomm.c' to reduce the
clutter in 'canldr.c'.

*/


#include "gatecomm.h"

static int PC_prep_msg(u8* pout, int outsize, u8* ps, int size);


/* **************************************************************************************
 * int gateway_msg_build(struct PCTOGATEWAY* ptr, u8 c);
 * @brief	: Build message from PC
 * @param	: ptr = Pointer to msg buffer (see common_can.h)
 * @param	: c = incoming byte
 * @return	: 0 = msg not ready; 1 = completed; ptr->ct hold byte count
 * ************************************************************************************** */
/* Note: It is up to the caller to have the struct initialized, initially and after the 
message has been "consumed." */
int gateway_msg_build(struct PCTOGATEWAY* ptr, u8 c)
{
	switch (c)
	{
	case CAN_PC_FRAMEBOUNDARY:	// Possible end of message
		if (ptr->prev == CAN_PC_ESCAPE)
		{ // Here, previous byte was an escape byte
			*ptr->p++ = c;
		}
		else
		{ // Here, frame without preceding escape means End of Message
/* NOTE: if checksum added, then change the following so ptr->ct does not include checksum */
/* How about making ptr->ct negative if there is a checksum error. */
			ptr->ct = ptr->p - &ptr->c[0]; // Number of bytes in message
			return 1;	// $$$$ COMPLETE $$$$
		}			
		break;

		case CAN_PC_ESCAPE: // Possible escape data byte, or escape for next byte.
		if (ptr->prev == CAN_PC_ESCAPE)
		{
			*ptr->p++ = c;	// Here, new byte is a data byte
			c = ~CAN_PC_ESCAPE;
		}
		break;

		default: // All other bytes come here.
		*ptr->p++ = c;
		break;
	}
		/* Check for buffer overflow. */
	if ( ptr->p >= (&ptr->c[0] + PCTOGATEWAYSIZE ) ) ptr->p -= 1; // Hold at end

	ptr->prev = c;	// Save previous char for byte stuffing check.
	return 0;
}
/* **************************************************************************************
 * void gateway_msg_intg(struct PCTOGATEWAY* p);
 * @brief	: Initialize struct for building a gateway message from the PC
 * @param	: Pointer to gatewau message wrapper (see common_can.h)
 * ************************************************************************************** */
void gateway_msg_intg(struct PCTOGATEWAY* p)
{
	p->p = &p->c[0];		// Pointer that will store incoming de-stuffed data bytes
	p->crc = 0;			// CRC initial value goes here, if we implement CRC'ing
	p->prev = ~CAN_PC_ESCAPE;	// Begin with received byte not an escape.
	return;
}
/* **************************************************************************************
 * static u8* putout(u8* pout, u8 c, u8* pstart, int n);
 * @brief	: Place byte in output, with bufferoverflow limit
 * @param	: pout = pointer to buffer with output file
 * @param	: c = byte to go into output buffer
 * @param	: pstart = pointer to beginning of buffer
 * @return	: n = number of bytes in output buffer
 * ************************************************************************************** */
static u8* putout(u8* pout, u8 c, u8* pend)
{
	*pout++ = c;
	if (pout >= pend ) pout --; // Prevent buffer overrun...jic
	return pout;
}
/* **************************************************************************************
 * static int PC_prep_msg(u8* pout, int outsize, u8* ps, int size);
 * @brief	: Prepare msg to gateway in binary with framing and byte stuffing 
 * @param	: pout = pointer to buffer with output file
 * @param	: outsize = number of bytes output buffer can hold
 ^ @param	: pstart = pointer to beginning of buffer
 * @param	: size = byte count to send (does not include code byte, frame bytes, or stuffing bytes)
 * @return	: number of bytes in output buffer
 * ************************************************************************************** */
static int PC_prep_msg(u8* pout, int outsize, u8* ps, int size)
{
	int i;
	u8 *pstart = pout;
	u8 *pend = (pout+outsize);

	/* Set up CAN msg with byte stuffing in USART buffer. */
	for (i = 0; i < size; i++)
	{
		if ((*ps == CAN_PC_FRAMEBOUNDARY) || (*ps == CAN_PC_ESCAPE)  )
		{
			pout = putout(pout, CAN_PC_ESCAPE, pend);
		}
		pout = putout(pout, *ps, pend);
		ps++;
	}
	pout = putout(pout, CAN_PC_FRAMEBOUNDARY, pend); // Set up End/Beginning of msg byte

	return (pout - pstart);	// Return byte out in output buffer
}
/* **************************************************************************************
 * int PCtoCAN_msg(u8* pout, int outsize, struct CANRCVBUF* ps);
 * @brief	: Send CAN msg to PC
 * @param	: Pointer to CAN message as received (see common_can.h)
 * ************************************************************************************** */
int PCtoCAN_msg(u8* pout, int outsize, struct CANRCVBUF* ps)
{
/* Note: The msg to the PC is sent with 8 bytes of payload even though the CAN msg payload
   count (dlc & 0x0f) may be less than 8.  Since the CAN->PC data load is light this simplificatin
   is not costly in terms of time. */
	
	return PC_prep_msg(pout, outsize, (u8*)ps, sizeof(struct CANRCVBUF));
}
/* **************************************************************************************
 * int PCtoGateway_msg(u8* pout, int outsize, u8* ps, int size);
 * @brief	: Send PC msg to Gateway
 * @param	: Pointer to gate.c message
 * @param	: size = byte count to send (does not include frame bytes, no stuffing bytes)
 * ************************************************************************************** */
int PCtoGateway_msg(u8* pout, int outsize, u8* ps, int size)
{
	return PC_prep_msg(pout, outsize, ps, size);
}
/* **************************************************************************************
 * int CANcompress(struct PCTOGATECOMPRESSED* pout, struct CANRCVBUF* pin);
 * @brief	: Send Gateway msg to PC
 * @param	: pout = pointer to output w compressed msg
 * @param	: pin = pointer to input with uncompressed msg
 * @return	: 0 = compressed; 1 = input not compressible
 * ************************************************************************************** */
int CANcompress(struct PCTOGATECOMPRESSED* pout, struct CANRCVBUF* pin)
{
	int i;
	/* IDE bit on calls for an extended address format.  */
	if ((pin->id & 0x04) != 0)
	{
		return 1;
	}

	/* Here, we can smoosh things together. */
	pout->cm[0] = ((pin->id >> 16) & 0xe0);		// Get low 3 bits of 11 bit address
	pout->cm[1] = ((pin->id >> 24) & 0xff);		// Get high 8 bits of 11 bit address
	pout->cm[0] |= ((pin->id << 3) & (1 << 4));	// Add incoming RTR bit
	pout->cm[0] |= (pin->dlc & 0xf);		// Data length

	pout->ct = (pin->dlc & 0xf);			// Number of payload bytes
	for (i = 0; i < pout->ct; i++)			// Copy payload bytes
		pout->cm[i+2] = pin->cd.u8[i];
	pout->ct += 2;					// Total count of compressed msg

	return 0;
}
/* **************************************************************************************
 * void CANuncompress(struct CANRCVBUF* pout,  struct PCTOGATECOMPRESSED* pin);
 * @brief	: Send Gateway msg to PC
 * @param	: pout = pointer to output w uncompressed msg
 * @param	: pin = pointer to input with compressed msg
 * @return	: none
 * ************************************************************************************** */
void CANuncompress(struct CANRCVBUF* pout,  struct PCTOGATECOMPRESSED* pin)
{
	u32 i;
	pout->id  = pin->cm[1] << 24;			// Hi order 8 bits of id
	pout->id |= ( (pin->cm[0] & 0xe0) << 16);	// Low order 3 bits of id
	pout->id |= ((pin->cm[0] & 0x10) >> 3) & 0x02;	// RTR bit
	pout->dlc = (pin->cm[0] & 0x0f);		// Payload ct
	for (i = 0; i < pout->dlc; i++)			// Copy payload bytes
	{
		pout->cd.u8[i] = pin->cm[i+2];	
	}
	return;
}
/* **************************************************************************************
 * void CANcopyuncompressed(struct CANRCVBUF* pout,  struct PCTOGATEWAY* pin);
 * @brief	: Copy incoming byte message (uncompressed) to CAN struct msg
 * @param	: pout = pointer to output 
 * @param	: pin = pointer to input
 * @return	: none
 * ************************************************************************************** */
void CANcopyuncompressed(struct CANRCVBUF* pout,  struct PCTOGATEWAY* pin)
{
	u32 i;
	pout->id  = ( (pin->c[3] << 24) | (pin->c[2] << 16) | (pin->c[1] <<  8) | pin->c[0] );
	pout->dlc = ( (pin->c[7] << 24) | (pin->c[6] << 16) | (pin->c[5] <<  8) | pin->c[4] );
	if ((pout->dlc & 0xf) > 8) pout->dlc = 8;
	for (i = 0; i < (pout->dlc & 0xf); i++)			// Copy payload bytes
		pout->cd.u8[i] = pin->c[i+8];	

	return;
}
/* **************************************************************************************
 * void CANcopycompressed(struct CANRCVBUF* pout,  struct PCTOGATEWAY* pin);
 * @brief	: Copy incoming byte message (compressed) to CAN struct msg
 * @param	: pout = pointer to output 
 * @param	: pin = pointer to input
 * @return	: none
 * ************************************************************************************** */
void CANcopycompressed(struct CANRCVBUF* pout,  struct PCTOGATEWAY* pin)
{
	u32 i;
	pout->id  = pin->c[1] << 24;			// Hi order 8 bits of id
	pout->id |= ( (pin->c[0] & 0xe0) << 16);	// Low order 3 bits of id
	pout->id |= ((pin->c[0] & 0x10) >> 3) & 0x02;	// RTR bit
	pout->dlc = (pin->c[0] & 0x0f);			// Payload ct
	if ((pout->dlc & 0xf) > 8) pout->dlc = 8;
	for (i = 0; i < pout->dlc; i++)			// Copy payload bytes
	{
		pout->cd.u8[i] = pin->c[i+2];	
	}
	return;
}
/* **************************************************************************************
 * int CANsendmsg(struct CANRCVBUF* pin);
 * @brief	: Send CAN msg to gateway.
 * @param	: pin = pointer to struct that is ready to go
 * @return	: 0 = OK;
 * ************************************************************************************** */

extern int pf;		/* port file descriptor (canldr.c) */
int CANsendmsg(struct CANRCVBUF* pin)
{	
	int size;
	struct PCTOGATECOMPRESSED pctogate;
	u8 c[128];	// Framed and byte stuffed chars that go to the serial port

	/* Compress the msg if possible */
	if (CANcompress(&pctogate, pin) == 0)
	{ // Here, compressed.
		size = PC_prep_msg(&c[0], 128, &pctogate.cm[0], (int)pctogate.ct);
	}
	else
	{ // Here, extended address format
		size = PC_prep_msg(&c[0], 128, (u8*)pin, (int)sizeof(struct CANRCVBUF));
	}
	write(pf,&c[0],size);	// Send it out the serial port.
int i; printf("CT:%d %d ",(int)pctogate.ct,size); for (i=0;i<size;i++) printf("%02x ",c[i]); printf("\n");

	return 0;
}

