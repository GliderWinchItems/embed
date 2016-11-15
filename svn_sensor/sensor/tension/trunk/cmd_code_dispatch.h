/******************************************************************************
* File Name          : cmd_code_dispatch.h
* Date First Issued  : 10/03/2016
* Board              :
* Description        : Dispatch to handler based on command code number
*******************************************************************************/
/*
*/

#include <stdint.h>

#ifndef __CMD_CODE_DISPATCH
#define __CMD_CODE_DISPATCH

#include "../../../../svn_common/trunk/common_can.h"
#include "tension_a_functionS.h"

/* ************************************************************************************** */
unsigned int cmd_code_dispatch(struct CANRCVBUF* pcan, struct TENSIONFUNCTION* p);
/* @brief	: Handle command code msg
 * @param	: pcan = pointer to command msg received
 * @param	: p = pointer to struct with values for this instance
 * @return	:  0 = OK; AND *pcan setup with return msg
 *		: -1 = code not in table, and pcan->cd.uc[0]
 *		: 255 = dlc less than 1
 *		: 254 = code not in table
 * ************************************************************************************** */

#endif
