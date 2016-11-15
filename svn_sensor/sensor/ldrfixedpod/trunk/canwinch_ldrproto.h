/******************************************************************************
* File Name          : canwinch_ldrproto.h
* Date First Issued  : 07/26/2013
* Board              : RxT6
* Description        : Loader protocol work
*******************************************************************************/

#ifndef __CANWINCH_LDRPROTO
#define __CANWINCH_LDRPROTO

#include "../../../../svn_common/trunk/common_misc.h"
#include "../../../../svn_common/trunk/common_can.h"
#include "common.h"
#include "../../../../svn_common/trunk/can_driver.h"

/* **************************************************************************************/
void canwinch_ldrproto_init(u32 iamunitnumber);
/* @brief	: Initialization for loader
 * @param	: Unit number 
 * ************************************************************************************** */
void canwinch_ldrproto_poll(void);
/* @param	: pctl = pointer control block for CAN module being used
 * @brief	: If msg is for this unit, then do something with it.
 * ************************************************************************************** */




/* Switch that shows if we have a program loading underway and should not jump to an app */
extern int ldr_phase;


#endif 

