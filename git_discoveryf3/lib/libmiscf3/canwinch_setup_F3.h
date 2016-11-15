/******************************************************************************
* File Name          : canwinch_setup_F3.h
* Date First Issued  : 01/21/2015
* Board              : F373
* Description        : Setup initializtion of CAN1
*******************************************************************************/
/* 
*/

#ifndef __CANWINCH_SETUP_F3
#define __CANWINCH_SETUP_F3

//#include "/home/deh/gitrepo/svn_common/trunk/can_driver.h"
#include "libopencm3/cm3/common.h"
#include "../../../svn_common/trunk/can_driver.h"
#include "../../../svn_common/trunk/common_can.h"

/******************************************************************************/
struct CLOCKSF3* canwinch_setup_F3_clocks(void);
/* @brief 	: Supply 'clock' params that match CAN setup params
 * @return	:  pointer to struct
*******************************************************************************/
struct CAN_CTLBLOCK* canwinch_setup_F3(const struct CAN_INIT* pinit, u32 cannum);
/* @brief 	: Provide CAN1 initialization parameters: winch app, F103, pod board
 * @param	: pinit = pointer to msg buffer counts for this CAN
 * @param	: cannum = 1  for CAN1
 * @return	: Pointer to control block for this CAN
 *		:  Pointer->ret = return code
 *		:  NULL = cannum not 1 or 2, calloc of control block failed
 *		:   0 success
 *		:  -1 cannum: CAN number not 1 or 2
 *		:  -2 calloc of linked list failed
 *		:  -3 RX0 get buffer failed
 *		:  -4 RX1 get buffer failed
 *		:  -6 CAN initialization mode timed out
 *		:  -7 Leave initialization mode timed out
 *		: -12 port pin setup CAN1
 *		: -13 port pin setup CAN2
*******************************************************************************/

#endif
