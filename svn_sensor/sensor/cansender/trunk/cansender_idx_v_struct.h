/******************************************************************************
* File Name          : cansender_idx_v_struct.h
* Date First Issued  : 09/09/2016
* Board              : Sensor
* Description        : Translate parameter index into pointer into struct
*******************************************************************************/
/*
These structs serve all 'Tension_a" function types (which is the one used with
AD7799 on the POD board).
*/

#include <stdint.h>
#include "common_can.h"

#ifndef __CANSENDER_IDX_V_STRUCT
#define __CANSENDER_IDX_V_STRUCT


/* The parameter list supplies the CAN IDs for the hardware filter setup. */
#define CANFILTMAX	8	// Max number of CAN IDs in parameter list

// Naming convention--"cid" - CAN ID
 struct CANSENDERLC
 {
	uint32_t size;		// Number of items in struct
 	uint32_t crc;		// crc-32 placed by loader
	uint32_t version;	// struct version number
	uint32_t hbct;		// Heartbeat count of time (ms) between msgs
	uint32_t cid_hb;	// CANID: Hearbeat msg sends running count
	uint32_t cid_poll;	// CANID: Poll this cansender
	uint32_t cid_pollr;	// CANID: Response to POLL
 };

/* **************************************************************************************/
int cansender_idx_v_struct_copy(struct CANSENDERLC* p, uint32_t* ptbl);
/* @brief	: Copy the flat array in high flash with parameters into the struct
 * @param	: p = pointer struct with parameters to be loaded
 * @param	: ptbl = pointer to flat table array
 * return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 * ************************************************************************************** */

#endif
