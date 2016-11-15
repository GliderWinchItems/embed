/******************************************************************************
* File Name          : tension_idx_v_struct.h
* Date First Issued  : 07/15/2015
* Board              :
* Description        : Translate parameter index into pointer into struct
*******************************************************************************/
/*
These structs serve all 'Tension_a" function types (which is the one used with
AD7799 on the POD board).
*/

#include <stdint.h>
#include "common_can.h"
#include "iir_filter_l.h"

#ifndef __TENSION_IDX_V_STRUCT
#define __TENSION_IDX_V_STRUCT

#define NIIR	2	// Number of IIR filters for one AD779

/* POD board currently only supports two AD7799 tension functions. */
#define NUMTENSIONFUNCTIONS	2	// Number of tension functions

/* The parameter list supplies the CAN IDs for the hardware filter setup. */
#define CANFILTMAX	8	// Max number of CAN IDs in parameter list


 // Thermistor parameters for converting ADC readings to temperature
  struct THERMPARAM
 {   //                   	   default values    description
	float B;		//	3380.0	// Thermistor constant "B" (see data sheets: http://www.murata.com/products/catalog/pdf/r44e.pdf)
	float RS;		//	10.0	// Series resistor, fixed (K ohms)
	float R0;		//	10.0	// Thermistor room temp resistance (K ohms)
	float TREF;		//	298.0	// Reference temp for thermistor
	float offset;		//      0.0	// Therm temp correction offset	1.0 Therm correction scale
	float scale;		//      1.0	// Therm temp correction scale	1.0 Therm correction scale
 };
 
// Each AD7799 has the following parameters
 struct AD7799PARAM
 {
	int32_t  offset;	// AD7799 offset (note: fixed)
	float 	 scale;		// AD7799 scale	 (note: float)
	struct THERMPARAM tp[2];// Two thermistor parameter sets
	float	comp_t1[4];	// AD7799 Temp compensation for thermistor 1
	float	comp_t2[4];	// AD7799 Temp compensation for thermistor 2
 };

 // Tension 
// Naming convention--"cid" - CAN ID
 struct TENSIONLC
 {
	uint32_t size;			// Number of items in struct
 	uint32_t crc;			// crc-32 placed by loader
	uint32_t version;		// struct version number
	struct AD7799PARAM ad;		// Parameters for one AD7799 (w thermistors)
	uint32_t hbct;			// Heartbeat ct: ticks between sending msgs
	uint32_t drum;			// Drum number
	uint32_t f_pollbit;		// Instance bit (bit position)(2nd byte)
	uint32_t p_pollbit;		// Poll response bit (bit position)(1st byte)
	uint32_t cid_ten_msg;		// CANID-Fully calibrated tension msg
	uint32_t cid_ten_poll;		// CANID-MC poll msg
	uint32_t cid_gps_sync;		// CANID-GPS time sync msg
	uint32_t cid_heartbeat;		// CANID-Heartbeat msg
	uint32_t cid_tst_ten_a;		// CANID-for testing/debugging .sql files
	struct IIR_L_PARAM iir[NIIR];	// IIR Filter for IIR filters 
	uint32_t useme;			// Bits for using this instance
	struct IIR_L_PARAM iir_z_recal;	// IIR Filter: zero recalibration
	uint32_t z_recal_ct;		// Conversion counts between zero recalibrations
	float limit_hi;			// Exceeding this limit (+) means bogus reading
	float limit_lo;			// Exceeding this limit (-) means bogus reading
	uint32_t code_CAN_filt[CANFILTMAX];// List of CAN ID's for setting up hw filter
 };
 

/* **************************************************************************************/
int tension_idx_v_struct_copy(struct TENSIONLC* p, uint32_t* ptbl);
/* @brief	: Copy the flat array in high flash with parameters into the struct
 * @param	: p = pointer struct with parameters to be loaded
 * @param	: ptbl = pointer to flat table array
 * return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 * ************************************************************************************** */
#endif
