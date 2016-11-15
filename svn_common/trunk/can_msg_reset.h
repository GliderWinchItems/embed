/******************************************************************************
* File Name          : can_msg_reset.h
* Date First Issued  : 06/27/2015
* Board              : F4 F103 with can_driver
* Description        : CAN msg forces system reset
*******************************************************************************/

#ifndef __CAN_MSG_RESET
#define __CAN_MSG_RESET

#include "../../../../svn_common/trunk/common_can.h"
/*******************************************************************************/
int can_msg_reset_init (struct CAN_CTLBLOCK* pctl, u32 canid);
/* @brief	: Initializes RX msgs to check for reset
 * @param	: pctl = pointer to CAN control block 
 * @param	: canid = CAN id used for reset msg to this unit
 * @return	: 0 = OK, -1 failed: RX ptr was not NULL
********************************************************************************/

extern void (*can_msg_reset_ptr)(void* pctl, struct CAN_POOLBLOCK* pblk);	// Pointer for extending RX0,1 interrupt processing

#endif
