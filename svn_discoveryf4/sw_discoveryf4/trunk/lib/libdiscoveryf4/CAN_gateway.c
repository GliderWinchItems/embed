/******************************************************************************
* File Name          : CAN_gateway.c
* Date First Issued  : 07/23/2013
* Board              : RxT6
* Description        : CAN<->gateway
*******************************************************************************/
#include "CAN_gateway.h"
#include "canwinch_ldr.h"

/* **************************************************************************************
 * int CAN_gateway_send(struct CANRCVBUF* pg);
 * @brief	: Setup CAN message for sending
 * @param	: pg = Pointer to message buffer (see common_can.h)
 * @return	: 0 = OK; -1 = dlc greater than 8; -2 = illegal extended address
 * ************************************************************************************** */

int CAN_gateway_send(struct CANRCVBUF* pg)
{
	/* Check number of bytes in payload and limit to 8. */
	if ((pg->dlc & 0x0f) > 8) 
		return -1;	// Payload ct too big
	
	/* Check if an illegal id combination */
	if ( ((pg->id & 0x001ffff9) != 0) && ((pg->id & 0x04) == 0) ) 
	{ // Here, in the additional 18 extended id bits one or more are on, but IDE flag is for standard id (11 bits)
		return -2; // Illegal id
	}

	/* Add msg to CAN outgoing buffer. */
	can_msg_put_ldr(pg);

	return 0;
}

