/******************************************************************************
* File Name          : CAN_poll_loop.h
* Date First Issued  : 04/04/2015
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


#endif 

