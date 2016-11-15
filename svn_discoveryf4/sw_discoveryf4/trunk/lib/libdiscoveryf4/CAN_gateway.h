/******************************************************************************
* File Name          : CAN_gateway.h
* Date First Issued  : 07/23/2013
* Board              : RxT6
* Description        : CAN<->gateway
*******************************************************************************/

#ifndef __CAN_GATEWAY
#define __CAN_GATEWAY

#include "common_misc.h"
#include "common_can.h"
/* **************************************************************************************/
int CAN_gateway_send(struct CANRCVBUF* pg);
/* @brief	: Setup CAN message for sending
 * @param	: pg = Pointer to message buffer (see common_can.h)
 * @return	: 0 = OK; -1 = dlc greater than 8; -2 = illegal extended address
 * ************************************************************************************** */

#endif 

