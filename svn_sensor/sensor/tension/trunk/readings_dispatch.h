/******************************************************************************
* File Name          : readings_dispatch.h
* Date First Issued  : 10/03/2016
* Board              :
* Description        : Dispatch to handler based on payload [1]
*******************************************************************************/

#include <stdint.h>

#ifndef __READINGS_DISPATCH
#define __READINGS_DISPATCH

#include "../../../../svn_common/trunk/common_can.h"
#include "tension_a_functionS.h"

/* ************************************************************************************** */
unsigned int readings_dispatch(struct CANRCVBUF* pcan, struct TENSIONFUNCTION* p);
 /* @brief	: Handle command code msg
 * @param	: pcan = pointer to command msg received
 * @param	: p = pointer to struct with values for this instance
 * @return	: 0 = OK & dlc and payload setup
 * 		: -2 = not in table, and cd.uc[1]--
 *		: 253 = dlc less than 2 (no readings code)
 *		: 252 = code not in table
 * ************************************************************************************** */

#endif
