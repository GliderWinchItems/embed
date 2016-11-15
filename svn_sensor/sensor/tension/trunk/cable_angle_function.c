/******************************************************************************
* File Name          : cable_angle_function.c
* Date First Issued  : 05/28/2016
* Board              : f103
* Description        : Cable angle (using tension and sheave load-pin readings)
*******************************************************************************/

#include <stdint.h>
#include "can_hub.h"
#include "DTW_counter.h"
#include "ad7799_filter_ten2.h"
#include "cable_angle_idx_v_struct.h"
#include "db/gen_db.h"
#include "tim3_ten2.h"
#include "../../../../svn_common/trunk/common_highflash.h"
#include "iir_filter_l.h"
#include "cable_angle_function.h"

/* This alias is up front where it is easy to find it. */
#define MY_FUNCTION_TYPE	FUNCTION_TYPE_CABLE_ANGLE 


/* CAN control block pointer. */
extern struct CAN_CTLBLOCK* pctl1;

/* Pointer to CAN hub block/buffer. */
static struct CANHUB* phub_tension;

/* tension parameters (SRAM working copy) */
extern struct CABLEANGLELC cangl;	// 

/* tim3 tick counter for next heart-beat CAN msg */
static u32 hb_t;	

/* Pointer into high flash for command can id assigned to this function. */
uint32_t* pcanid_cmd_func_cangl;	// Pointer into high flash for command can id

/* Mask for first two bytes of a poll msg */
static uint16_t poll_mask; 

/* **************************************************************************************
 * static void send_can_msg(uint32_t canid, uint8_t status, uint32_t* pv); 
 * @brief	: Setup CAN msg with reading
 * @param	: canid = CAN ID
 * @param	: status = status of reading
 * @param	: pv = pointer to a 4 byte value (little Endian) to be sent
 * ************************************************************************************** */
static void send_can_msg(uint32_t canid, uint8_t status, uint32_t* pv)
{
	struct CANRCVBUF can;
	can.id = canid;
	can.dlc = 5;			// Set return msg payload count
	can.cd.uc[0] = status;
	can.cd.uc[1] = (*pv >>  0);	// Add 4 byte value to payload
	can.cd.uc[2] = (*pv >>  8);
	can.cd.uc[3] = (*pv >> 16);
	can.cd.uc[4] = (*pv >> 24);
	can_hub_send(&can, phub_tension);// Send CAN msg to 'can_hub'
	hb_t = tim3_ten2_ticks + cangl.hbct;	 // Reset heart-beat time duration each time msg sent
	return;
}
/* **************************************************************************************
 * int cable_angle_function_init_struct(uint32_t const* ptbl);
 * @brief	: Populate the parameter struct (in SRAM) with the four byte parameter values
 * @param	: ptbl = pointer to table (in flash) of parameter values
 * return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 * ************************************************************************************** */
static int cable_angle_function_init_struct(uint32_t const* ptbl)
{
	/* Point to table of struct element pointers */ 
	uint32_t** pptr = (uint32_t**)&cable_angle_idx_v_ptr[0];	// 
	uint32_t ctr = 0;	// counter for checking size of struct vs table of values

	/* Copy four byte values in the table in flash into the struct in sram. */
	while ( (*pptr != NULL) && (ctr <= sizeof (struct CABLEANGLELC)) )
	{ 
		**pptr++ = *ptbl++;
		ctr += 1;
	}
	/* struct element count versus size of table of values */
	if (*pptr != NULL) return 0;	// Something wrong with table
	if (ctr == 0) return ctr;	// Or something else wrong.
	if (ctr != (PARAM_LIST_CT_TENSION_a + 1)) return -ctr;	// Return for count mismatch
	return ctr;	// Success!
}
/* **************************************************************************************
 * int cable_angle_function_init(void);
 * @brief	: Initialize
 * return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 *		: -999 = table size for command CAN IDs unreasonable
 *		: -998 = command can id for this function was not found
 * ************************************************************************************** */
extern void* __paramflash3;	// High flash address of 3st parameter table (.ld defined)
extern void* __paramflash0a;	// High flash address of command CAN id table (.ld defined)

int cable_angle_function_init(void)
{
	int ret;
	u32 i;

	/* Set pointer to table in highflash. */
	// TODO routine to find latest updated table in this flash section
	uint32_t* ptbl = (uint32_t*)&__paramflash3;// (__paramflash3 supplied by .ld file)

	/* PARAM_LIST size for this function should match 1st location. */


	/* Copy table entries to struct in sram from high flash. */
	ret = cable_angle_function_init_struct(ptbl);

	/* Setup mask for checking if this function responds to a poll msg. */
	poll_mask = (cangl.p_pollbit << 8) || (cangl.f_pollbit & 0xff);

	/* First heartbeat time */
	hb_t = tim3_ten2_ticks + cangl.hbct;	

	/* Add this function (tension_a) to the "hub-server" msg distribution. */
	phub_tension    = can_hub_add_func();	// Set up port/connection to can_hub
	
	/* Find command CAN id for this function in table. (__paramflash0a supplied by .ld file) */
	struct FLASHH2* p0a = (struct FLASHH2*)&__paramflash0a;

	/* Check for reasonable size value in table */
	if ((p0a->size == 0) || (p0a->size > NUMCANIDS2)) return -999;

// TODO get the CAN ID for the ldr from low flash and compare to the loader
// CAN id in this table.

	/* Check if function type code in the table matches our function */
	for (i = 0; i < p0a->size; i++)
	{ 
		if (p0a->slot[i].func == MY_FUNCTION_TYPE)
		{
			pcanid_cmd_func_cangl = &p0a->slot[i].canid; // Save pointer
			return ret;
		}
	}
	return -998;	// Argh! Table size reasonable, but didn't find it.
}

/* ######################################################################################
 * void cable_angle_function_poll(struct CANRCVBUF* pcan);
 * @brief	: Handle incoming CAN msgs
 * @param	; pcan = pointer to CAN msg buffer
 * @return	: Nothing for the nonce
 * ###################################################################################### */
extern unsigned int tim3_ten2_ticks;	// Running count of timer ticks
/* NOTE ***
   This routine is called from CAN_poll_loop.c which runs under I2C1_ER_IRQ
*/
void cable_angle_function_poll(struct CANRCVBUF* pcan)
{
	float ft = 0;

	/* Check for need to send  heart-beat. */
	 if ( ( (int)tim3_ten2_ticks - (int)hb_t) > 0  )	// Time to send heart-beat
	{
		//                        CAN id, status of reading, reading pointer
		send_can_msg(cangl.cid_heartbeat, 0x00, (uint32_t*)&ft ); 
	}

	if (pcan == NULL) return;

	/* Check for group poll. */
	if (pcan->id == cangl.cid_cangl_msg)
	{ // Here, group poll msg.  Check if our drum system is included
		if ((pcan->cd.uc[0] & POLL_FUNC_BIT_CABLE_ANGLE) != 0)
		{ // Here, yes.  See if our function is included
			if ((pcan->cd.us[0] & poll_mask) != 0)
			{ // Here, yes.  Send our precious msg.
				pcan->cd.uc[0] = 0;	// Status byte TODO
//$				send_can(cangl.cid_ten_msg, 0, ad7799_ten_last_good_reading);
				return;
			}
		}
	}
	
	/* Check for cable angle function command. */
	if (pcan->id == *pcanid_cmd_func_cangl)
	{ // Here, we have a command msg for this function instance. 
		// TODO handle command code
	}
	return;
}

