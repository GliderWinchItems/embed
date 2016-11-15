/******************************************************************************
* File Name          : logger_idx_v_struct.h
* Date First Issued  : 08/08/2016
* Board              : Sensor w SD card 
* Description        : Translate parameter index into pointer into struct
*******************************************************************************/
/*

*/

#include <stdint.h>
#include "common_can.h"

#ifndef __LOGGER_IDX_V_STRUCT
#define __LOGGER_IDX_V_STRUCT


 // Tension 
// Naming convention--"cid" - CAN ID
 struct LOGGERLC
 {
	uint32_t size;		// 0 Number of items in struct
 	uint32_t crc;		// 1 crc-32 placed by loader
	uint32_t version;	// 2 struct version number
	uint32_t hbct;		// 3 Heartbeat count (ms) between msgs
	uint32_t cid_loghb_ctr;	// 4 CANID: Heartbeat running count
};

/* **************************************************************************************/
int logger_idx_v_struct_copy(struct LOGGERLC* p, uint32_t* ptbl);
/* @brief	: Copy the flat array in high flash with parameters into the struct
 * @param	: p = pointer struct with parameters to be loaded
 * @param	: ptbl = pointer to flat table array
 * return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 * ************************************************************************************** */
#endif
