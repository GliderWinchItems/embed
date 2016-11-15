/******************************************************************************
* File Name          : CAN_error_msgs.h
* Date First Issued  : 12/17/2013
* Board              : Discovery F4
* Description        : PC<->gateway--error msg monitoring
*******************************************************************************/

#ifndef __CAN_ERROR_MSGS
#define __CAN_ERROR_MSGS

#include "common_misc.h"
#include "common_can.h"

struct CANERR2
{
	int	idx;
	u32	ct;
};

/* ************************************************************************************** */
void Errors_USB_PC_get_msg_mode(int x);
/* @brief	: Count errors
 * @param	: Subroutine return from: USB_PC_get_msg_mode
 * ************************************************************************************** */
void Errors_CAN_gateway_send(int x);
/* @brief	: Count errors
 * @param	: Subroutine return from: USB_PC_get_msg_mode
 * ************************************************************************************** */
struct CANERR2 Errors_get_count(void);
/* @brief	: Return next error count number and error count
 * @return	: array index; error count
 * ************************************************************************************** */
void Errors_misc(int x);
/* @brief	: Count errors
 * @param	: Subroutine return for "one-off" counts
 * ************************************************************************************** */

#endif 

