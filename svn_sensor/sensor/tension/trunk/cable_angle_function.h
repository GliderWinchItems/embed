/******************************************************************************
* File Name          : cable_angle_function.h
* Date First Issued  : 05/28/2016
* Board              : f103
* Description        : Cable angle (using tension and sheave load-pin readings)
*******************************************************************************/

#ifndef __CABLE_ANGLE_FUNCTION
#define __CABLE_ANGLE_FUNCTION

#include <stdint.h>
#include "common_misc.h"
#include "common_can.h"

 // -----------------------------------------------------------------------------------------------

/* **************************************************************************************/
int cable_angle_function_init(void);
/* @brief	: Initialize
 * return	:  + = table size count of mismatch
 *		:  0 = OK
 *		: -1 = count is zero 
 * ************************************************************************************** */
void cable_angle_function_poll(struct CANRCVBUF* pcan);
/* @brief	: Handle incoming CAN msgs
 * @param	; pcan = pointer to CAN msg buffer
 * @return	: Nothing for the nonce
 * ************************************************************************************** */


#endif 

