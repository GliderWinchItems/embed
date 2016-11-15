/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : CAN_gateway.h
* Author             : deh
* Date First Issued  : 07/23/2013
* Board              : RxT6
* Description        : CAN<->gateway
*******************************************************************************/

#ifndef __CAN_GATEWAY
#define __CAN_GATEWAY

#include "common_misc.h"
#include "common_can.h"
#include "canwinch_pod_common_systick2048.h"
/* **************************************************************************************/
int CAN_gateway_send(struct CANRCVBUF* pg);
/* @brief	: Setup CAN message for sending
 * @param	: pg = Pointer to message buffer (see common_can.h)
 * @return	: 0 = OK; -1 = error
 * ************************************************************************************** */

extern u32 CAN_gateway_unitid;	// Current id in filter

#endif 

