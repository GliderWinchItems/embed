/******************************************************************************
* File Name          : CAN_poll_loop.h
* Date First Issued  : 09/09/2016
* Board              : f103
* Description        : Poll the functions for the tension app
*******************************************************************************/

#ifndef __CAN_POLL_LOOP
#define __CAN_POLL_LOOP

#include "common_misc.h"
#include "../../../../svn_common/trunk/common_can.h"

/* **************************************************************************************/
void CAN_poll_loop_trigger(void);
/* @brief	: Trigger low level interrupt to run 'CAN_poll'
 * ************************************************************************************** */
int CAN_poll_loop_init(void);
/* @brief	: Initialize
 * @return	: 0 = OK, negative = fail
 * ************************************************************************************** */

#endif 

