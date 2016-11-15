/*****************************************************************************
* File Name          : tilt_func.h
* Date First Issued  : 01/22/2015
* Board              : Discovery F4
* Description        : Tilt function
*******************************************************************************/


#ifndef __TILTFUNC
#define __TILTFUNC

#include <stdint.h>
#include "common_can.h"

/******************************************************************************/
void tilt_func_init(uint32_t port);
/* @brief	: Initialize
 * @param	: port = Serial port output number
*******************************************************************************/
void tilt_func_poll(struct CANRCVBUF *pcan);
/* @brief	: 
 * @param	: pcan = pointer to CAN msg
*******************************************************************************/

#endif

