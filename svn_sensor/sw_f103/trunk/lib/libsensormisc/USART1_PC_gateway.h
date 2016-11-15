/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : USART1_PC_gateway.h
* Author             : deh
* Date First Issued  : 07/23/2013
* Board              : RxT6
* Description        : PC<->gateway 
*******************************************************************************/

#ifndef __USART1_PC_GATEWAY
#define __USART1_PC_GATEWAY

#include "common_misc.h"
//#include "common_can.h"
#include "../../../../svn_common/trunk/common_can.h"


/* **************************************************************************************/
int USART1_PC_msg_get(struct PCTOGATEWAY* ptr);
/* @brief	: Build message from PC
 * @param	: ptr = Pointer to msg buffer (see common_can.h)
 * @return	: 0 = msg not ready; 1 = completed; ptr->ct hold byte count
 * ************************************************************************************** */
void USART1_toPC_msg(u8* pin, int ct);
/* @brief	: Send msg to PC in binary with framing and byte stuffing 
 * @param	: pin = Pointer to bytes to send to PC
 * @param	: ct = byte count to send (does not include frame bytes, no stuffing bytes)
 * ************************************************************************************** */


#endif 

