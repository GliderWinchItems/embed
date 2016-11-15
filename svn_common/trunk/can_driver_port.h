/******************************************************************************
* File Name          : can_driver_port.h
* Date First Issued  : 05-29-2015
* Board              : F103
* Description        : CAN port pin setup for 'can_driver.c'
*******************************************************************************/
/* 
*/

#ifndef __CAN_DRIVER_PORT
#define __CAN_DRIVER_PORT

#include "../../svn_discoveryf4/sw_discoveryf4/trunk/lib/libopencm3/cm3/common.h"
#include "common_can.h"


/******************************************************************************/
int can_driver_port(u8 port, u8 cannum );
/* @brief 	: Setup CAN pins and hardware
 * @param	: port = port code number
 * @param	: cannum = CAN num (0 or 1, for CAN1 or CAN2)
 * @return	: 0 = success; not zero = failed miserably.
 *		: -1 = cannum: not CAN1 
*******************************************************************************/

#endif
