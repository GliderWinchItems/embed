/******************************************************************************
* File Name          : canwinch_setup_F4_discovery.h
* Date First Issued  : 06-15-2015
* Board              : F4 discovery
* Description        : Setup initializtion of CAN1 and/or CAN2
*******************************************************************************/
/* 
*/

#ifndef __CANWINCH_SETUP_F4_DISC
#define __CANWINCH_SETUP_F4_DISC

#include "../../../svn_common/trunk/common_can.h"
#include "../../../svn_common/trunk/can_driver.h"

/******************************************************************************/
struct CLOCKS* canwinch_setup_F4_discovery_clocks(void);
/* @brief 	: Supply 'clock' params that match CAN setup params
 * @return	:  pointer to struct
*******************************************************************************/
struct CAN_CTLBLOCK* canwinch_setup_F4_discovery(const struct CAN_INIT* pinit, u32 cannum);
/* @brief 	: Provide CAN1 initialization parameters: winch app, F103, pod board
 * @param	: pinit = pointer to msg buffer counts for this CAN
 * @param	: cannum = 1 or 2 for CAN1 or CAN2
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
