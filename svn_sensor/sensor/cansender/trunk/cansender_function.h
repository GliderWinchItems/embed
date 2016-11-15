/******************************************************************************
* File Name          : cansender_function.h
* Date First Issued  : 05/29/2016
* Board              : f103
* Description        : Tension function_a for both AD7799s
*******************************************************************************/

#ifndef __CANSENDER_FUNCTION
#define __CANSENDER_FUNCTION

#include <stdint.h>
#include "common_misc.h"
#include "../../../../svn_common/trunk/common_can.h"
#include "cansender_idx_v_struct.h"

/* This is the working struct */
struct CANSENDERFUNCTION
{
	/* The following is the sram copy of the fixed (upper flash) parameters */
	struct CANSENDERLC send_a;		// Flash table copied to sram struct
	/* The following are working/computed values */
	struct CANHUB* phub_cansender;	// Pointer: CAN hub buffer
	void* pcansender_idx_struct;	// Pointer to table of pointers for idx->struct 
	void* pparamflash;		// Pointer to flash area with flat array of parameters
	uint32_t* pcanid_cmd_func;	// Pointer into high flash for command can id
	uint32_t hb_t;			// tim3 tick counter for next heart-beat CAN msg
	uint32_t hbct_ticks;		// ten_a.hbct (ms) converted to timer ticks
	uint32_t running_ct;		// Running count of something
	uint8_t status_byte;		// Reading status byte
};

/* **************************************************************************************/
int cansender_function_init_all(void);
/* @brief	: Initialize all 'tension_a' functions
 * @return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 *		: -999 = table size for command CAN IDs unreasonablevoid
 *		: -998 = command can id for this function was not found
 * ************************************************************************************** */
int cansender_function_poll(struct CANRCVBUF* pcan, struct CANSENDERFUNCTION* p);
/* @brief	: Handle incoming CAN msgs ### under interrupt ###
 * @param	; pcan = pointer to CAN msg buffer
 * @param	: p = pointer to struct with "everything" for this instance of tension_a function
 * @return	: 0 = No msgs sent; 1 = msgs were sent and loaded into can_hub buffer
 * ************************************************************************************** */


/* Holds parameters and associated computed values and readings for each instance. */
extern struct CANSENDERFUNCTION send_f;

#endif 

