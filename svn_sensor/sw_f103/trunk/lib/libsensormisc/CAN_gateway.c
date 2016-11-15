/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : CAN_gateway.c
* Author             : deh
* Date First Issued  : 07/23/2013
* Board              : RxT6
* Description        : CAN<->gateway
*******************************************************************************/
#include "CAN_gateway.h"

/* **************************************************************************************
 * int CAN_gateway_send(struct CANRCVBUF* pg);
 * @brief	: Setup CAN message for sending
 * @param	: pg = Pointer to message buffer (see common_can.h)
 * @return	: 0 = OK; -1 = error
 * ************************************************************************************** */
u32 CAN_gateway_unitid;	// Current id in filter

int CAN_gateway_send(struct CANRCVBUF* pg)
{
	/* Extract UNITD for filtering incoming msgs. */
//	u32 unitidtmp = (pg->id & CAN_UNITID_MASK);
		
	/* Setup CAN filters to see only incoming msgs from for this unit. */
//	if (unitidtmp == CAN_gateway_unitid) // Do we need an update?
//	{ // Here, yes.  
//		CAN_gateway_unitid = unitidtmp;
//		can_filter_unitid_sys(CAN_gateway_unitid);
//	}
	
	/* Check number of bytes in payload and limit to 8. */
	if ((pg->dlc & 0x0f) > 8) pg->dlc = ((pg->dlc & ~0x0f) | 8);

	/* Add msg to CAN outgoing buffer. */
	can_msg_put_sys(pg);

	return 0;
}

