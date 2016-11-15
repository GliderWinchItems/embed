/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : PC_gateway_comm.c
* Author             : 
* Date First Issued  : 10/04/2013
* Board              : Not specific to PC or stm32
* Description        : PC<->gateway 
*******************************************************************************/
 #include "PC_gateway_comm.h"

/* **************************************************************************************
 * int PC_msg_get(struct PCTOGATEWAY* ptr, u8 c);
 * @brief	: Build message from incoming bytes
 * @param	: ptr = Pointer to msg buffer (see common_can.h)
 * @param	: c = byte to add msg being built
 * @return	:  1 = completed; ptr->ct hold byte count
 *              :  0 = msg not ready; 
 *              : -1 = completed, but bad checksum
 *  		: -2 = completed, but too few bytes to be a valid CAN msg
 * ************************************************************************************** */
/* Note: It is up to the caller to have the struct initialized, initially and after the 
message has been "consumed."  When there are errors this routine re-initializes.  */

/* Mostly for debugging, and hardware testing. */
static u32 PC_chksum_ct_err;	// Count checksum errors
static u32 PC_toofew_ct_err;	// Count msg frames with too few bytes

int PC_msg_get(struct PCTOGATEWAY* ptr, u8 c)
{			
	int i;
	switch (c)
	{
	case CAN_PC_FRAMEBOUNDARY:	// Possible end of message
		if (ptr->prev == CAN_PC_ESCAPE)
		{ // Here, previous byte was an escape byte
			*ptr->p++ = c;
		}
		else
		{ // Here, frame without preceding escape means End of Message
			i = ptr->p - &ptr->c[0] - 1; // Number of bytes received in this frame less chksum
			ptr->ct = i;	// Save for others

			if (i < 3)		// Too few bytes to comprise a valid msg?
			{ // Here yes.
				PC_msg_initg(ptr);	// Initialize struct for the next message
				PC_toofew_ct_err += 1;	// Running count of this type of error.
				return -2;	// Return error code.
			}
								
			/* Check checksum. */	
			if ( (CANgenchksum(&ptr->c[0], i)) == ptr->c[i])
			{ // Here checksum good
/* If incoming, complete & valid, messages are to be buffered, this is where the index for the
  buffer array would be advanced. */
				return 1;	// $$$$ COMPLETE & SUCCESS $$$$
			}
			else
			{ // Here, failed
				PC_msg_initg(ptr);	// Initialize struct for the next message
				PC_chksum_ct_err += 1;	// Running count of checksum errors
				return -1;	// Return error code.
			}
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
 * void PC_msg_init(struct CANPCWRAP* p);
 * @brief	: Initialize struct for building a CAN message from the PC
 * @param	: Pointer to CAN message wrapper (see common_can.h)
 * ************************************************************************************** */
void PC_msg_init(struct CANPCWRAP* p)
{
	p->p = &p->can.c[0];		// Pointer that will store incoming de-stuffed data bytes
	p->chk = CHECKSUM_INITIAL;	// Checksum initial value
	p->prev = ~CAN_PC_ESCAPE;	// Begin with received byte that is not an escape.
	return;
}
/* **************************************************************************************
 * void PC_msg_initg(struct PCTOGATEWAY* p);
 * @brief	: Initialize struct for building a gateway message from the PC
 * @param	: Pointer to gatewau message wrapper (see common_can.h)
 * ************************************************************************************** */
void PC_msg_initg(struct PCTOGATEWAY* p)
{
	p->p = &p->c[0];		// Pointer that will store incoming de-stuffed data bytes
	p->chk = CHECKSUM_INITIAL;	// Checksum initial value
	p->prev = ~CAN_PC_ESCAPE;	// Begin with received byte not an escape.
	return;
}
/* **************************************************************************************
 * int PC_msg_prep(u8* pout, int outsize, u8* pin, int ct);
 * @brief	: Convert input bytes into output with byte stuffing, checksum and framing
 * @param	: pout = pointer to bytes with stuffing and chksum added 
 * @param	: outsize = size of output buffer (to prevent overflow if too small)
 * @param	: pin = Pointer to bytes to send
 * @param	: ct = byte count to send (does not include frame bytes, chksum, or stuffing bytes)
 * @return	: number of bytes in prepped message
 * ************************************************************************************** */
/*
Note, in the very worst-case the output may be over (2*ct+3) the size of the input.
*/
int PC_msg_prep(u8* pout, int outsize, u8* pin, int ct)
{
	u8 *p1 = pin;	// Redundant?
	u8 *p2 = pout;	// Working pointer
	u8 *p2e = pout + outsize; // End of output buffer pointer
	u8 chk;		// Checksum computed on input bytes
	int i;

	/* Compute chksum on input message. */
	chk = CANgenchksum(pin, ct);
	
	/* Set up CAN msg with byte stuffing in output buffer. */
	for (i = 0; i < ct; i++)
	{
		if ((*p1 == CAN_PC_FRAMEBOUNDARY) || (*p1 == CAN_PC_ESCAPE) )
		{
			*p2++ = (CAN_PC_ESCAPE); // Precede the following char with an escape byte
			if (p2 >= p2e) p2--;  // Prevent some bozo from overrunning the buffer.
		}
		*p2++ = *p1++;		// Place the real byte with precision.
		if (p2 >= p2e) p2 -= 1; // Prevent buffer overflow at the hands of a thoughtless bozo.
	}

	/* Set up chksum byte stuffing in output buffer. */
	if ((chk == CAN_PC_FRAMEBOUNDARY) || (chk == CAN_PC_ESCAPE) )
	{
		*p2++ = (CAN_PC_ESCAPE); // Precede following char with escape
		if (p2 >= p2e) p2--;  	// Prevent some jerk from overrunning the buffer.
	}
	*p2++ = chk;			// Quietly place the checksum.
	if (p2 >= p2e) p2--; 		// Prevent buffer overflow by some nefarious person.

	*p2++ = (CAN_PC_FRAMEBOUNDARY);	// Set up End of Frame byte

	return (p2 - pout);		// Return number of bytes in output
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
 * u8 CANgenchksum(u8* p, int ct);
 * @brief	: Generate a one byte checksum
 * @param	: p = pointer to array to be checksummed
 * @param	: ct = number of bytes to be checksumned
 * @return	: Checksum
 * ************************************************************************************** */
u8 CANgenchksum(u8* p, int ct)
{
	int i = 0;
	u32 x = CHECKSUM_INITIAL;
	for (i = 0; i < ct; i++)
		x += *p++;
	x += (x >> 16);	// Add carries into high half word
	x += (x >> 16);	// Add carry if previous add generated a carry
	x += (x >> 8);  // Add high byte of low half word
	x += (x >> 8);  // Add carry if previous add generated a carry
	return x;
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
	u32 i,j;
	pout->id  = ( (pin->c[3] << 24) | (pin->c[2] << 16) | (pin->c[1] <<  8) | pin->c[0] );
	pout->dlc = ( (pin->c[7] << 24) | (pin->c[6] << 16) | (pin->c[5] <<  8) | pin->c[4] );
	j = (pout->dlc & 0xf);
	if (j > 7) j = 7;		// Limit to max in case of garbage
	for (i = 0; i < j; i++)			// Copy payload bytes
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
	if (pout->dlc > 7) pout->dlc = 7;		// Limit to max in case of garbage
	for (i = 0; i < pout->dlc; i++)			// Copy payload bytes
	{
		pout->cd.u8[i] = pin->c[i+2];	
	}
	return;
}
/* **************************************************************************************
 * int CANmsgvalid(struct CANRCVBUF* p);
 * @brief	: Check that struct looks like a valid CAN msg
 * @param	: p = pointer to input
 * @return	: 0 - OK, not zero = bad
 * ************************************************************************************** */
int CANmsgvalid(struct CANRCVBUF* p)
{
	/* Are the positions in the id that are always zero, zero? */
	if ((p->id & ~(CAN_DATATYPE_MASK | CAN_UNITID_MASK | 0x7) ) != 0) return 1;

	/* Is the number of payload bytes within range? */
	if ( (p->dlc & 0xf) > 8)	return 1;

	return 0;
}


