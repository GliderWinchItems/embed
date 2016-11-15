/******************************************************************************
* File Name          : canwinch_pod_common_systick2048_printerr.h
* Date First Issued  : 05/12/2015
* Board              : 
* Description        : print error counts for 'canwinch_pod_common_systick2048'
*******************************************************************************/

#ifndef __CANWINCH_COMMON_2048_PRINTERR
#define __CANWINCH_COMMON_2048_PRINTERR

#include "../../../../../svn_discoveryf4/sw_discoveryf4/trunk/lib/libopencm3/cm3/common.h"
#include "../../../../svn_common/trunk/common_can.h"
//#include "canwinch_pod_common_systick2048.h"

/******************************************************************************/
void canwinch_pod_common_systick2048_printerr_header(void);
/* @brief	: print header/description for all the error counts
 ******************************************************************************/
void canwinch_pod_common_systick2048_printerr(struct CANWINCHPODCOMMONERRORS* p);
/* @brief	: print all the error counts
 * @param	: p = pointer to struct with error counts
 ******************************************************************************/

#endif

