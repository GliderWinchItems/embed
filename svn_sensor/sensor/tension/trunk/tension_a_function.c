/******************************************************************************
* File Name          : tension_a_function.c
* Date First Issued  : 03/26/2015, 05/27/2016
* Board              : f103
* Description        : Tension function
*******************************************************************************/

#include <stdint.h>
#include "can_hub.h"
#include "DTW_counter.h"
#include "ad7799_filter_ten.h"
#include "tension_idx_v_struct.h"
#include "db/gen_db.h"
#include "tension_idx_v_struct.h"
#include "tim3_ten.h"
#include "../../../../svn_common/trunk/common_highflash.h"
#include "iir_filter_l.h"

/* This alias is up front where it is easy to find it. */
#define MY_FUNCTION_TYPE_A2	FUNCTION_TYPE_TENSION_a2 


/* CAN control block pointer. */
extern struct CAN_CTLBLOCK* pctl1;

/* Pointer to CAN hub block/buffer. */
static struct CANHUB* phub_tension;

/* tension parameters (SRAM working copy) */
/* The following are a working structs in SRAM, initialized from flat
   tables with four byte entries located in separate areas of flash. */
struct TENSIONLC ten_a;		// struct with TENSION function parameters for 1st AD7799

/* tim3 tick counter for next heart-beat CAN msg */
static u32 hb_t;	

/* Pointer into high flash for command can id assigned to this function. */
uint32_t* pcanid_cmd_func_a;	// Pointer into high flash for command can id

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
	hb_t = tim3_ticks + ten_a.hbct;	 // Reset heart-beat time duration each time msg sent
	return;
}
/* **************************************************************************************
 * int tension_a_function_init_struct(uint32_t const* ptbl);
 * @brief	: Populate the parameter struct (in SRAM) with the four byte parameter values
 * @param	: ptbl = pointer to table (in flash) of parameter values
 * return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 * ************************************************************************************** */
static int tension_a_function_init_struct(uint32_t const* ptbl)
{
	/* Point to table of struct element pointers */ 
	uint32_t** pptr = (uint32_t**)&tension_a_idx_v_ptr[0];	// 
	uint32_t* ptmp;
	uint32_t ctr = 0;	// counter for checking size of struct vs table of values

	/* Copy four byte values in the table in flash into the struct in sram. */
	while ( (*pptr != NULL) && (ctr <= sizeof (struct TENSIONLC)) )
	{ 
		ptmp = *pptr++ + ((uint32_t*)&ten_a - (uint32_t*)&ten_a); // Pointer into ten_a2 struct
		*ptmp = *ptbl++;
		ctr += 1;
	}
	/* struct element count versus size of table of values */
	if (*pptr != NULL) return 0;	// Something wrong with table
	if (ctr == 0) return ctr;	// Or something else wrong.
	if (ctr != (PARAM_LIST_CT_TENSION_a + 1)) return -ctr;	// Return for count mismatch
	return ctr;	// Success!
}
/* **************************************************************************************
 * int tension_a_function_init(void);
 * @brief	: Initialize
 * return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 *		: -999 = table size for command CAN IDs unreasonable
 *		: -998 = command can id for this function was not found
 * ************************************************************************************** */
extern void* __paramflash1;	// High flash address of 1st parameter table (.ld defined)
extern void* __paramflash0a;	// High flash address of command CAN id table (.ld defined)

int tension_a_function_init(void)
{
	int ret;
	u32 i;

	/* Set pointer to table in highflash. */
	// TODO routine to find latest updated table in this flash section
	uint32_t* ptbl = (uint32_t*)&__paramflash1;// (__paramflash1 supplied by .ld file)

	/* PARAM_LIST size for this function should match 1st location. */

//	if (*ptbl != (PARAM_LIST_CT_TENSION_a + 1)) return *ptbl;

	/* Copy table entries to struct in sram from high flash. */
	ret = tension_a_function_init_struct(ptbl);

	/* Setup IIR filter structs with parameters from table. */
	ad7799_filter_ten_iir_init(ten_a.iir_poll.k, ten_a.iir_poll.scale, 0, 0);
	ad7799_filter_ten_iir_init(ten_a.iir_hb.k,   ten_a.iir_hb.scale,   1, 0);

	/* Setup mask for checking if this function responds to a poll msg. */
	poll_mask = (ten_a.p_pollbit << 8) || (ten_a.f_pollbit & 0xff);

	/* First heartbeat time */
	hb_t = tim3_ticks + ten_a.hbct;	

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
		if (p0a->slot[i].func == MY_FUNCTION_TYPE_A2)
		{
			pcanid_cmd_func_a = &p0a->slot[i].canid; // Save pointer
			return ret;
		}
	}
	return -998;	// Argh! Table size reasonable, but didn't find it.
}
/* **************************************************************************************
 * static float tension_a_scalereading(void);
 * @brief	: Apply offset, scale and corrections to tension_a, last reading
 * return	: float with value
 * ************************************************************************************** */
float tension_a_scalereading(void)
{
	int ntmp1;
	long long lltmp;
	float scaled1;

	lltmp = cic[0][0].llout_save;
	ntmp1 = lltmp/(1<<22);
	ntmp1 += ten_a.ad.offset * 4;
	scaled1 = ntmp1;
	scaled1 *= ten_a.ad.scale;
	return scaled1;
}
/* ######################################################################################
 * void tension_a_function_poll(struct CANRCVBUF* pcan);
 * @brief	: Handle incoming CAN msgs
 * @param	; pcan = pointer to CAN msg buffer
 * @return	: Nothing for the nonce
 * ###################################################################################### */
extern unsigned int tim3_ticks;	// Running count of timer ticks
/* NOTE ***
   This routine is called from CAN_poll_loop.c which runs under I2C1_ER_IRQ
*/
void tension_a_function_poll(struct CANRCVBUF* pcan)
{
	float ft;

	/* Check for need to send  heart-beat. */
	 if ( ( (int)tim3_ticks - (int)hb_t) > 0  )	// Time to send heart-beat
	{ 
		ft = tension_a_scalereading(); // Get scaled reading and store float as a int
		//                        CAN id, status of reading, reading pointer
		send_can_msg(ten_a.cid_heartbeat, 0x00, (uint32_t*)&ft ); 
	}

	if (pcan == NULL) return;

	/* Check for group poll. */
	if (pcan->id == ten_a.cid_ten_poll)
	{ // Here, group poll msg.  Check if poll and function bits are for us
		if ((pcan->cd.us[0] & poll_mask) != 0)
		{ // Here, yes.  Send our precious msg.
			pcan->cd.uc[0] = 0;	// Status byte TODO
//$			send_can(ten_a.cid_ten_msg, 0, ad7799_ten_last_good_reading);
			return;
		}
	}
	
	/* Check for tension function command. */
	if (pcan->id == *pcanid_cmd_func_a)
	{ // Here, we have a command msg for this function instance. 
		// TODO handle command code
	}
	return;
}

