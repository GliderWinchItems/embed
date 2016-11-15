/******************************************************************************
* File Name          : canwinch_setup_F103_Olimex.h
* Date First Issued  : 08-12-2015
* Board              : F103
* Description        : Setup initializtion of CAN1 for winch, F103 Olimex board
*******************************************************************************/
/* 
*/

#ifndef __CANWINCH_SETUP_F103_Olimex
#define __CANWINCH_SETUP_F103_Olimex

#include "../../../../svn_common/trunk/common_can.h"
#include "../../../../svn_common/trunk/can_driver.h"

/******************************************************************************/
struct CLOCKS* canwinch_setup_F103_Olimex_clocks(void);
/* @brief 	: Supply 'clock' params that match CAN setup params
 * @return	:  pointer to struct
*******************************************************************************/
struct CAN_CTLBLOCK* canwinch_setup_F103_Olimex(const struct CAN_INIT* pinit, u32 canid);
/* @brief 	: Provide CAN1 initialization parameters: winch app, F103, pod board
 * @param	: pinit = pointer to msg buffer counts for this CAN
 * @param	: canid = CAN id used for reset msg to this unit
 * @return	: Pointer to control block for this CAN
 *		:  Pointer->ret = return code
 *		:  NULL = cannum not 1 or 2, calloc of control block failed
 *		:   0 success
 *		:  -1 cannum: CAN number not 1 or 2
 *		:  -2 calloc of linked list failed
 *		:  -3 RX0 get buffer failed
 *		:  -4 RX1 get buffer failed
 *		:  -5 port pin setup failed
 *		:  -6 CAN initialization mode timed out
 *		:  -7 Leave initialization mode timed out
*******************************************************************************/

#endif
