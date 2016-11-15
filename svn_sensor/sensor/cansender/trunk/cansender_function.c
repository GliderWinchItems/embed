/******************************************************************************
* File Name          : cansender_function.c
* Date First Issued  : 05/29/2016
* Board              : f103
* Description        : Tension function_a for both AD7799s  Capital "S" for plural!
*******************************************************************************/

#include "cansender_function.h"
#include <stdint.h>
#include "can_hub.h"
#include "DTW_counter.h"
#include "db/gen_db.h"
#include "tim3_ten2.h"
#include "../../../../svn_common/trunk/common_highflash.h"
#include "temp_calc_param.h"
#include "SENSORpinconfig.h"
#include "can_driver_filter.h"

/* CAN control block pointer. */
extern struct CAN_CTLBLOCK* pctl1;

/* Pointer to flash area with parameters  */
extern void* __paramflash0a;	// High flash address of command CAN id table (.ld defined)
extern void* __paramflash1;	// High flash address of 1st parameter table (.ld defined)
extern void* __paramflash2;	// High flash address of 2nd parameter table (.ld defined)

/* Holds parameters and associated computed values and readings for each instance. */
struct CANSENDERFUNCTION send_f;

const uint32_t* pparamflash = (uint32_t*)&__paramflash1;

/* Base pointer adjustment for idx->struct table. */
struct CANSENDERLC* plc;

/* Highflash command CAN id table lookup mapping. */
const uint32_t myfunctype = FUNCTION_TYPE_CANSENDER;
/* **************************************************************************************
 * static void send_can_msg(uint32_t canid, uint8_t status, uint32_t* pv, struct CANSENDERFUNCTION* p); 
 * @brief	: Setup CAN msg with reading
 * @param	: canid = CAN ID
 * @param	: status = status of reading
 * @param	: pv = pointer to a 4 byte value (little Endian) to be sent
 * @param	: p = pointer to a bunch of things for this function instance
 * ************************************************************************************** */
static void send_can_msg(uint32_t canid, uint8_t status, uint32_t* pv, struct CANSENDERFUNCTION* p)
{
	struct CANRCVBUF can;
	can.id = canid;
	can.dlc = 5;			// Set return msg payload count
	can.cd.uc[0] = status;
	can.cd.uc[1] = (*pv >>  0);	// Add 4 byte value to payload
	can.cd.uc[2] = (*pv >>  8);
	can.cd.uc[3] = (*pv >> 16);
	can.cd.uc[4] = (*pv >> 24);
	can_hub_send(&can, p->phub_cansender);// Send CAN msg to 'can_hub'
	return;
}
/* **************************************************************************************
 * int cansender_function_init_all(void);
 * @brief	: Initialize all 'tension_a' functions
 * @return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 *		: -999 = table size for command CAN IDs unreasonablevoid
 *		: -998 = command can id for this function was not found
 *		: -997 = Add CANHUB failed
 *		: -996 = Adding CAN IDs to hw filter failed
 *
 * static int cansender_function_init(struct CANSENDERFUNCTION* p );
 * @brief	: Initialize function
 * @param	: p = pointer to things needed for this function
 * @return	: Same as above
 * ************************************************************************************** */
//  Declaration
static int cansender_function_init(struct CANSENDERFUNCTION* p );

int cansender_function_init_all(void)
{
	int ret;

	ret = cansender_function_init(&send_f);
	if (ret < 0) return ret;

	return ret;
}
/* *********************************************************** */
/* Do the initialization mess for a single tension_a function. */
/* *********************************************************** */
static int cansender_function_init(struct CANSENDERFUNCTION* p )
{
	int ret;
	int ret2;
	u32 i;

	/* Set pointer to table in highflash.  Base address provided by .ld file */
// TODO routine to find latest updated table in this flash section
	p->pparamflash = (uint32_t*)pparamflash;

	/* Copy table entries to struct in sram from high flash. */
	ret = cansender_idx_v_struct_copy(&p->send_a, p->pparamflash);

	/* First heartbeat time */
	// Convert heartbeat time (ms) to timer ticks (recompute for online update)
	p->hbct_ticks = (p->send_a.hbct * tim3_ten2_rate) / 1000;
	p->hb_t = tim3_ten2_ticks + p->hbct_ticks;	

	/* Add this function (cansender) to the "hub-server" msg distribution. */
	p->phub_cansender = can_hub_add_func();	// Set up port/connection to can_hub
	if (p->phub_cansender == NULL) return -997;	// Failed

	/* Add CAN IDs to incoming msgs passed by the CAN hardware filter. */ 
	ret2 = can_driver_filter_add_param_tbl(&p->send_a.cid_poll, 1, CANFILTMAX, CANID_DUMMY);
	if (ret2 != 0) return -996;	// Failed
	
	/* Find command CAN id for this function in table. (__paramflash0a supplied by .ld file) */
	struct FLASHH2* p0a = (struct FLASHH2*)&__paramflash0a;

	/* Check for reasonable size value in table */
	if ((p0a->size == 0) || (p0a->size > NUMCANIDS2)) return -999;

// TODO get the CAN ID for the ldr from low flash and compare to the loader
// CAN id in this table.

	/* Check if function type code in the table matches our function */
	for (i = 0; i < p0a->size; i++)
	{ 
		if (p0a->slot[i].func == myfunctype)
		{
			p->pcanid_cmd_func = &p0a->slot[i].canid; // Save pointer
			return ret;
		}
	}
	return -998;	// Argh! Table size reasonable, but didn't find it.
}
/* ######################################################################################
 * int cansender_function_poll(struct CANRCVBUF* pcan, struct CANSENDERFUNCTION* p);
 * @brief	: Handle incoming CAN msgs ### under interrupt ###
 * @param	; pcan = pointer to CAN msg buffer
 * @param	: p = pointer to struct with "everything" for this instance of cansender_a function
 * @return	: 0 = No msgs sent; 1 = msgs were sent and loaded into can_hub buffer
 * ###################################################################################### */
//extern unsigned int tim3_ten2_ticks;	// Running count of timer ticks
/* *** NOTE ***
   This routine is called from CAN_poll_loop.c which runs under I2C1_ER_IRQ
*/
int cansender_function_poll(struct CANRCVBUF* pcan, struct CANSENDERFUNCTION* p)
{
	int ret = 0;
	uint32_t test = 0x12345678;

	/* Check for need to send  heart-beat. */
	 if ( ( (int)tim3_ten2_ticks - (int)p->hb_t) > 0  )	// Time to send heart-beat?
	{ // Here, yes.
		/* Send heartbeat and compute next hearbeat time count. */
		//      Args:  CAN id, status of reading, reading pointer instance pointer
		send_can_msg(p->send_a.cid_hb, p->status_byte, &p->running_ct, p);
		p->hbct_ticks = (p->send_a.hbct * tim3_ten2_rate) / 1000; // Convert ms to timer ticks
		p->hb_t = tim3_ten2_ticks + p->hbct_ticks;	 // Reset heart-beat time duration each time msg sent 
		p->running_ct += 1;
		ret = 1;
	}

	/* Check if any incoming msgs. */
	if (pcan == NULL) return ret;

	/* Check for group poll, and send msg if it is for us. */
	if (pcan->id == p->send_a.cid_poll) // Correct ID?
//if (pcan->id == 0x00200000) // ##### TEST #########
	{ // Here, group poll msg.  Check if poll and function bits are for us
		/* Send cansender msg and re-compute next hearbeat time count. */
		//      Args:  CAN id, status of reading, reading pointer instance pointer
		send_can_msg(p->send_a.cid_pollr, p->status_byte, &test, p); 
		return 1;
	}
	
	/* Check for cansender function command. */
	if (pcan->id == *p->pcanid_cmd_func)
	{ // Here, we have a command msg for this function instance. 
		// TODO handle command code, set return code if sending msg
		return 1;
	}
	return ret;
}

