/******************************************************************************
* File Name          : cansender_idx_v_struct.c
* Date First Issued  : 09/09/2016
* Board              :
* Description        : Translate parameter index into pointer into struct
*******************************************************************************/
/* This file is manually generated and must match the idx_v_val.c file that is
generated with the java program and database.

The reference for the sequence of items in this file is the database table--
PARAM_LIST
And for making making changes to *values* manually,
PARAM_VAL_INSERT.sql

If the number of entries in this file differs the initialization of 'tension.c' will
bomb with a mis-match error.
*/
#include <stdint.h>
#include "cansender_idx_v_struct.h"
#include "cansender_function.h"
#include "db/gen_db.h"

#define NULL 0

/* **************************************************************************************
 * int cansender_idx_v_struct_copy2(struct TENSIONLC* p, uint32_t* ptbl);
 * @brief	: Copy the flat array in high flash with parameters into the struct
 * @param	: p = pointer struct with parameters to be loaded
 * @param	: ptbl = pointer to flat table array
 * return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 * ************************************************************************************** */
int cansender_idx_v_struct_copy(struct CANSENDERLC* p, uint32_t* ptbl)
{
/* NOTE: values that are not uint32_t  */
p->size    	= ptbl[0];		 		/*  0 Tension: Number of elements in the following list */
p->crc     	= ptbl[CANSENDER_LIST_CRC];		/*  1 Tension: CRC for tension list */
p->version	= ptbl[CANSENDER_LIST_VERSION];		/*  2 Version number */	
p->hbct		= ptbl[CANSENDER_HEARTBEAT_CT];		/*  3 Heartbeat count of time (ms) between msgs */
p->cid_hb	= ptbl[CANSENDER_HEARTBEAT_MSG];	/*  4 CANID: Hearbeat msg sends running count */
p->cid_poll	= ptbl[CANSENDER_POLL];			/*  5 CANID: Poll this cansender */
p->cid_pollr	= ptbl[CANSENDER_POLL_R];	  	/*  6 CANID: Response to POLL */

return PARAM_LIST_CT_CANSENDER;
}


