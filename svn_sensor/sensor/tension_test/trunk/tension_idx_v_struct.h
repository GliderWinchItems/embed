/******************************************************************************
* File Name          : tension_idx_v_struct.h
* Date First Issued  : 07/15/2015
* Board              :
* Description        : Translate parameter index into pointer into struct
*******************************************************************************/

#include <stdint.h>
#include "common_can.h"

#ifndef __TENSION_IDX_V_STRUCT
#define __TENSION_IDX_V_STRUCT


/* --- struct that holds parameters for TENSION function ----------------------- */
/*
INSERT INTO PARAM_LIST VALUES ('TENSION_LIST_SIZE'	   , 1, 'TYP_U32','%08X', 	'TENSION',	'Tension: size: number of items in list');
INSERT INTO PARAM_LIST VALUES ('TENSION_LIST_CRC'	   , 2, 'TYP_U32','%08X', 	'TENSION',	'Tension: crc: CRC for tension list');
INSERT INTO PARAM_LIST VALUES ('TENSION_LIST_VERSION'      , 3, 'TYP_S32','%d', 	'TENSION',	'Tension: version: Version number for Tension List');
INSERT INTO PARAM_LIST VALUES ('TENSION_AD7799_1_OFFSET'   , 4, 'TYP_S32','%d', 	'TENSION',	'Tension: offset: AD7799 #1 offset');
INSERT INTO PARAM_LIST VALUES ('TENSION_AD7799_1_SCALE'    , 5, 'TYP_FLT','%f', 	'TENSION',	'Tension: scale: AD7799 #1 Scale (convert to kgf)');
INSERT INTO PARAM_LIST VALUES ('TENSION_THERM1_CONST_B'    , 6, 'TYP_FLT','%f', 	'TENSION',	'Tension: Thermistor1 param: B: constant B');
INSERT INTO PARAM_LIST VALUES ('TENSION_THERM1_R_SERIES'   , 7, 'TYP_FLT','%f', 	'TENSION',	'Tension: Thermistor1 param: RS: Series resistor, fixed (K ohms)');
INSERT INTO PARAM_LIST VALUES ('TENSION_THERM1_R_ROOMTMP'  , 8, 'TYP_FLT','%f', 	'TENSION',	'Tension: Thermistor1 param: R0: Thermistor room temp resistance (K ohms)');
INSERT INTO PARAM_LIST VALUES ('TENSION_THERM1_REF_TEMP'   , 9, 'TYP_FLT','%f', 	'TENSION',	'Tension: Thermistor1 param: TREF: Reference temp for thermistor');
INSERT INTO PARAM_LIST VALUES ('TENSION_THERM1_TEMP_OFFSET',10, 'TYP_FLT','%f', 	'TENSION',	'Tension: Thermistor1 param: offset: Thermistor temp offset correction (deg C)');
INSERT INTO PARAM_LIST VALUES ('TENSION_THERM1_TEMP_SCALE' ,11, 'TYP_FLT','%f', 	'TENSION',	'Tension: Thermistor1 param: B: scale:  Thermistor temp scale correction');
INSERT INTO PARAM_LIST VALUES ('TENSION_THERM2_CONST_B'    ,12, 'TYP_FLT','%f', 	'TENSION',	'Tension: Thermistor2 param: RS: constant B');
INSERT INTO PARAM_LIST VALUES ('TENSION_THERM2_R_SERIES'   ,13, 'TYP_FLT','%f', 	'TENSION',	'Tension: Thermistor2 param: Series resistor, fixed (K ohms)');
INSERT INTO PARAM_LIST VALUES ('TENSION_THERM2_R_ROOMTMP'  ,14, 'TYP_FLT','%f', 	'TENSION',	'Tension: Thermistor2 param: TREF: Thermistor room temp resistance (K ohms)');
INSERT INTO PARAM_LIST VALUES ('TENSION_THERM2_REF_TEMP'   ,15, 'TYP_FLT','%f', 	'TENSION',	'Tension: Thermistor2 param: Reference temp for thermistor');
INSERT INTO PARAM_LIST VALUES ('TENSION_THERM2_TEMP_OFFSET',16, 'TYP_FLT','%f', 	'TENSION',	'Tension: Thermistor2 param: offset: hermistor temp offset correction (deg C)');
INSERT INTO PARAM_LIST VALUES ('TENSION_THERM2_TEMP_SCALE' ,17, 'TYP_FLT','%f', 	'TENSION',	'Tension: Thermistor2 param: scale: Thermistor temp scale correction');
INSERT INTO PARAM_LIST VALUES ('TENSION_THERM1_COEF_0',	    18, 'TYP_FLT','%f', 	'TENSION',	'Tension: Thermistor1 param: comp_t1[0]: Load-Cell polynomial coefficient 0 (offset)');
INSERT INTO PARAM_LIST VALUES ('TENSION_THERM1_COEF_1',	    19, 'TYP_FLT','%f', 	'TENSION',	'Tension: Thermistor1 param: comp_t1[1]: Load-Cell polynomial coefficient 1 (scale)');
INSERT INTO PARAM_LIST VALUES ('TENSION_THERM1_COEF_2',	    20, 'TYP_FLT','%f', 	'TENSION',	'Tension: Thermistor1 param: comp_t1[2]: Load-Cell polynomial coefficient 2 (x^2)');
INSERT INTO PARAM_LIST VALUES ('TENSION_THERM1_COEF_3',	    21, 'TYP_FLT','%f', 	'TENSION',	'Tension: Thermistor1 param: comp_t1[3]: Load-Cell polynomial coefficient 3 (x^3)');
INSERT INTO PARAM_LIST VALUES ('TENSION_THERM2_COEF_0',	    22, 'TYP_FLT','%f', 	'TENSION',	'Tension: Thermistor2 param: comp_t2[0]: Load-Cell polynomial coefficient 0 (offset)');
INSERT INTO PARAM_LIST VALUES ('TENSION_THERM2_COEF_1',	    23, 'TYP_FLT','%f', 	'TENSION',	'Tension: Thermistor2 param: comp_t2[1]: Load-Cell polynomial coefficient 1 (scale)');
INSERT INTO PARAM_LIST VALUES ('TENSION_THERM2_COEF_2',	    24, 'TYP_FLT','%f', 	'TENSION',	'Tension: Thermistor2 param: comp_t2[2]: Load-Cell polynomial coefficient 2 (x^2)');
INSERT INTO PARAM_LIST VALUES ('TENSION_THERM2_COEF_3',	    25, 'TYP_FLT','%f', 	'TENSION',	'Tension: Thermistor2 param: comp_t2[3]: Load-Cell polynomial coefficient 3 (x^3)');
INSERT INTO PARAM_LIST VALUES ('TENSION_HEARTBEAT_CT'      ,26, 'TYP_U32','%u', 	'TENSION',	'Tension: hbct: Heart-Beat Count of time ticks between autonomous msgs');
INSERT INTO PARAM_LIST VALUES ('TENSION_DRUM_NUMBER'       ,27, 'TYP_U32','%u', 	'TENSION',	'Tension: drum: Drum system number for this function instance');
INSERT INTO PARAM_LIST VALUES ('TENSION_DRUM_POLL_BIT'     ,28, 'TYP_U32','%02x', 	'TENSION',	'Tension: f_pollbit: Drum system poll 1st byte bit for function instance');
INSERT INTO PARAM_LIST VALUES ('TENSION_DRUM_FUNCTION_BIT' ,29, 'TYP_U32','%02x', 	'TENSION',	'Tension: p_pollbit: Drum system poll 2nd byte bit for this type of function');
INSERT INTO PARAM_LIST VALUES ('TENSION_CANPRM_TENSION'    ,30,	'TYP_CANID','%x',  	'TENSION',  	'Tension: cid_ten_msg:  canid msg Tension');
INSERT INTO PARAM_LIST VALUES ('MSG_TIME_POLL'             ,32,	'TYP_CANID','%x', 	'TENSION',  	'Tension: cid_ten_msg:  canid MC: Time msg/Group polling');
INSERT INTO PARAM_LIST VALUES ('TENSION_TIMESYNC'          ,32,	'TYP_CANID','%x',  	'TENSION',  	'Tension: cid_gps_sync: canid time: GPS time sync distribution');
*/

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
 struct TENSIONLC
 {
	uint32_t size;			// Number of items in struct
 	uint32_t crc;			// crc-32 placed by loader
	uint32_t version;		// struct version number
	struct AD7799PARAM ad;		// Parameters for one AD7799 (w thermistors)
	uint32_t hbct;			// Heartbeat ct: ticks between sending msgs
	uint32_t drum;			// Drum number
	uint32_t f_pollbit;		// Instance bit (bit position)
	uint32_t p_pollbit;		// Poll response bit (bit position)
	uint32_t cid_ten_msg;		// CANID-Fully calibrated tension msg
	uint32_t cid_ten_poll;		// CANID-MC poll msg
	uint32_t cid_gps_sync;		// CANID-GPS time sync msg
 };
 
struct PARAMIDPTR {
	uint16_t id;
	void*	ptr;
};

extern struct TENSIONLC ten_a;	// struct with TENSION function parameters
extern const void* idx_v_ptr[];

#endif
