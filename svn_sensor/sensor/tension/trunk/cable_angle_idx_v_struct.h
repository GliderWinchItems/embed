/******************************************************************************
* File Name          : cable_angle_idx_v_struct.h
* Date First Issued  : 05/28/2016
* Board              :
* Description        : Translate parameter index into pointer into struct
*******************************************************************************/
/*

*/

#include <stdint.h>
#include "common_can.h"

#ifndef __CABLE_ANGLE_IDX_V_STRUCT
#define __CABLE_ANGLE_IDX_V_STRUCT

 
 // Cable Angle 
// Naming convention--"cid" - CAN ID
 struct CABLEANGLELC
 {
	uint32_t size;			// Number of items in struct
 	uint32_t crc;			// crc-32 placed by loader
	uint32_t version;		// struct version number
	uint32_t hbct;			// Heartbeat ct: ticks between sending msgs
	uint32_t drum;			// Drum number
	uint32_t f_pollbit;		// Instance bit (bit position)
	uint32_t p_pollbit;		// Poll response bit (bit position)
	float	minten;			// Minimum tension for a valid reading
	uint32_t rate_ct;		// Number of tension msgs between cable_angle msgs
	uint32_t alarm_repeat;		// Number of alarm msgs repeated
	float comp[4];			// Coefficients for computation
	uint32_t cid_cangl_msg;		// CANID-Fully calibrated tension msg
	uint32_t cid_cangl_poll;	// CANID-MC poll msg
	uint32_t cid_gps_sync;		// CANID-GPS time sync msg
	uint32_t cid_heartbeat;		// CANID-Heartbeat msg
	uint32_t useme;			// Bits for using this instance
 };
 
//struct PARAMIDPTR {
//	uint16_t id;
//	void*	ptr;
//};

extern struct CABLEANGLELC cangl;	// struct with CABLE ANGLE function parameters
extern const void* cable_angle_idx_v_ptr[];

#endif
