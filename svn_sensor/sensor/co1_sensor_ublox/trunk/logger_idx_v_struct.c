/******************************************************************************
* File Name          : logger_idx_v_struct.c
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

If the number of entries in this file differs the initialization of 'logger.c' will
bomb with a mis-match error.
*/
#include <stdint.h>
#include "logger_idx_v_struct.h"
#include "can_log.h"
#include "db/gen_db.h"

#define NULL 0

/* **************************************************************************************
 * int logger_idx_v_struct_copy2(struct LOGGERLC* p, uint32_t* ptbl);
 * @brief	: Copy the flat array in high flash with parameters into the struct
 * @param	: p = pointer struct with parameters to be loaded
 * @param	: ptbl = pointer to flat table array
 * return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 * ************************************************************************************** */
int logger_idx_v_struct_copy(struct LOGGERLC* p, uint32_t* ptbl)
{

/* NOTE: values that are not uint32_t  */
p->size         = ptbl[0];		 		/* 0 Tension: Number of elements in the following list */
p->crc          = ptbl[LOGGER_LIST_CRC];		/* 1 Tension: CRC for logger list */
p->version      = ptbl[LOGGER_LIST_VERSION];		/* 2 Version number */	
p->hbct		= ptbl[LOGGER_HEARTBEAT1_CT];		/* 3 Heartbeat count of time (ms) between msgs */
p->cid_loghb_ctr= ptbl[LOGGER_HEARTBEAT_MSG];		/* 4 CANID: Hearbeat sends running count of logged msgs */

return PARAM_LIST_CT_LOGGER;
}


