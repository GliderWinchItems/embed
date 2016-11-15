/******************************************************************************
* File Name          : gps_idx_v_struct.c
* Date First Issued  : 08/08/2016
* Board              :
* Description        : Translate parameter index into pointer into struct
*******************************************************************************/
/* This file is manually generated and must match the idx_v_val.c file that is
generated with the java program and database.

The reference for the sequence of items in this file is the database table--
PARAM_LIST
And for making making changes to *values* manually,
PARAM_VAL_INSERT.sql

If the number of entries in this file differs the initialization of 'gps.c' will
bomb with a mis-match error.
*/
#include <stdint.h>
#include "gps_idx_v_struct.h"
#include "can_log.h"
#include "db/gen_db.h"

#define NULL 0

/* **************************************************************************************
 * int gps_idx_v_struct_copy2(struct GPSLC* p, uint32_t* ptbl);
 * @brief	: Copy the flat array in high flash with parameters into the struct
 * @param	: p = pointer struct with parameters to be loaded
 * @param	: ptbl = pointer to flat table array
 * return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 * ************************************************************************************** */
int gps_idx_v_struct_copy(struct GPSLC* p, uint32_t* ptbl)
{
/* NOTE: values that are not uint32_t  */
p->size         = ptbl[0];		 	/* 0 Tension: Number of elements in the following list */
p->crc          = ptbl[GPS_LIST_CRC];		/* 1 Tension: CRC for gps list */
p->version      = ptbl[GPS_LIST_VERSION];	/* 2 Version number */	
p->hbct_tim	= ptbl[GPS_HEARTBEAT_TIME_CT];	/* 3 (ms) between unix time msgs */
p->hbct_llh	= ptbl[GPS_HEARTBEAT_LLH_CT];	/* 4 (ms) between burst of lat lon height msgs */
p->hbct_llh_delay= ptbl[GPS_HEARTBEAT_LLH_DELAY_CT];	/* 5 (ms) between lat/lon and lon/ht msgs */
p->cid_hb_tim	= ptbl[GPS_HEARTBEAT_TIME];	/* 6 CANID: Heartbeat time */
p->cid_hb_llh	= ptbl[GPS_HEARTBEAT_LLH];	/* 7 CANID: Heartbeat lattitude longitude height */
p->disable_sync	= ptbl[GPS_DISABLE_SYNCMSGS];	/* 8 time sync msgs: 0 = enable  1 = disable */
p->cid_timesync	= ptbl[GPS_TIME_SYNC_MSG];	/* 9 CANID: Time sync msg */

return PARAM_LIST_CT_GPS;
}


