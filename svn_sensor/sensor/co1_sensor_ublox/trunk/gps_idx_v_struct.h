/******************************************************************************
* File Name          : gps_idx_v_struct.h
* Date First Issued  : 08/08/2016
* Board              : Sensor w SD card 
* Description        : Translate parameter index into pointer into struct
*******************************************************************************/
/*

*/

#include <stdint.h>
#include "common_can.h"

#ifndef __GPS_IDX_V_STRUCT
#define __GPS_IDX_V_STRUCT

// Naming convention--"cid" - CAN ID
 struct GPSLC
 {
	uint32_t size;		// 0 Number of items in struct
 	uint32_t crc;		// 1 crc-32 placed by loader
	uint32_t version;	// 2 struct version number
	uint32_t hbct_tim;	// 3 Time (ms) between unix time msgs
	uint32_t hbct_llh;	// 4 Time (ms) between burst of lat lon height msgs
	uint32_t hbct_llh_delay;// 5 Heartbeat count (ms) between gps lat/lon/ht msgs burst
	uint32_t cid_hb_tim;	// 6 CANID: Heartbeat unix time
	uint32_t cid_hb_llh;	// 7 CANID: Heartbeat fix (3 msgs) lat/lon/ht
	uint32_t disable_sync;	// 8 0 = enable time sync msgs; 1 = disable
	uint32_t cid_timesync;	// 9 CANID: Time Sync msg
};

/* **************************************************************************************/
int gps_idx_v_struct_copy(struct GPSLC* p, uint32_t* ptbl);
/* @brief	: Copy the flat array in high flash with parameters into the struct
 * @param	: p = pointer struct with parameters to be loaded
 * @param	: ptbl = pointer to flat table array
 * return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 * ************************************************************************************** */
#endif
