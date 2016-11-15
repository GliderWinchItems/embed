/******************************************************************************
* File Name          : cable_angle_idx_v_struct.c
* Date First Issued  : 05/28/2016
* Board              :
* Description        : Translate parameter index into pointer into struct
*******************************************************************************/
/* This file is manually generated and must match the idx_v_val.c file that is
generated with the java program and database.

The reference for the sequence of items in this file is the database table--
PARAM_LIST
And for making making changes to *values* manually,
PARAM_VAL_INSERT.sql

If the number of entries in this file differs the initialization of 'cable_angle_function.c' will
bomb with a mis-match error.
*/

#include <stdint.h>
#include "cable_angle_idx_v_struct.h"
#define NULL 0

/* The following is a working struct in SRAM, initialized from a flat
   table of four byte entries located in flash. */
struct CABLEANGLELC cangl;  // struct with CABLE_ANGLE parameters

/* Array: paramter index versus pointer into parameter struct. This table
   is used to move the four byte entries in the flash table into the 
   working area (generally a struct) in sram. */
const void* cable_angle_idx_v_ptr[] = {\
	&cangl.size,		// Number of items in struct
 	&cangl.crc,		// crc-32 placed by loader
	&cangl.version,		// struct version number
	&cangl.hbct,		// Heartbeat ct: ticks between sending msgs
	&cangl.drum,		// Drum number
	&cangl.f_pollbit,	// Instance bit (bit position)
	&cangl.p_pollbit,	// Poll response bit (bit position)
	&cangl.minten,		// Minimum tension for a valid reading
	&cangl.rate_ct,		// Number of tension msgs between cable_angle msgs
	&cangl.alarm_repeat,	// Number of alarm msgs repeated
	&cangl.comp[4],		// Coefficients for computation
	&cangl.cid_cangl_msg,	// CANID-Fully calibrated tension msg
	&cangl.cid_cangl_poll,	// CANID-MC poll msg
	&cangl.cid_gps_sync,	// CANID-GPS time sync msg
	&cangl.cid_heartbeat,	// CANID-Heartbeat msg
	&cangl.useme,		// Bits for using this instance
    NULL			/* 00 End of list: (since valid pointers must not be NULL)  */
};

