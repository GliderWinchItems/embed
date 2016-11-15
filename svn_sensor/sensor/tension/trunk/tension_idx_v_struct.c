/******************************************************************************
* File Name          : tension_idx_v_struct.c
* Date First Issued  : 07/15/2015,06/04/2016
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
#include "tension_idx_v_struct.h"
#include "tension_a_functionS.h"
#include "db/gen_db.h"

#define NULL 0

/* **************************************************************************************
 * int tension_idx_v_struct_copy2(struct TENSIONLC* p, uint32_t* ptbl);
 * @brief	: Copy the flat array in high flash with parameters into the struct
 * @param	: p = pointer struct with parameters to be loaded
 * @param	: ptbl = pointer to flat table array
 * return	:  + = table size
 *		:  0 = error
 *		:  - = -(size of struct table count)
 * ************************************************************************************** */
int tension_idx_v_struct_copy(struct TENSIONLC* p, uint32_t* ptbl)
{
	union {
	float f;
	uint32_t ui;
	int32_t n;
	}u;

/* NOTE: values that are not uint32_t  */
p->size            = ptbl[0];		 			  /*  0 Tension: Number of elements in the following list */
p->crc             = ptbl[TENSION_a_LIST_CRC];		 	  /*  1 Tension: CRC for tension list */
p->version         = ptbl[TENSION_a_LIST_VERSION];		  /*  2 Version number */	
p->ad.offset       = (int)ptbl[TENSION_a_AD7799_1_OFFSET];	  /*  3 Tension: AD7799 offset */
u.ui = ptbl[TENSION_a_AD7799_1_SCALE]; 		p->ad.scale        = u.f;/*  4 Tension: AD7799 #1 Scale (convert to kgf) */
u.ui = ptbl[TENSION_a_THERM1_CONST_B]; 		p->ad.tp[0].B      = u.f;/*  5 Tension: Thermistor1 param: constant B */
u.ui = ptbl[TENSION_a_THERM1_R_SERIES];		p->ad.tp[0].RS     = u.f;/*  6 Tension: Thermistor1 param: Series resistor, fixed (K ohms) */
u.ui = ptbl[TENSION_a_THERM1_R_ROOMTMP];	p->ad.tp[0].R0     = u.f;/*  7 Tension: Thermistor1 param: Thermistor room temp resistance (K ohms) */
u.ui = ptbl[TENSION_a_THERM1_REF_TEMP]; 	p->ad.tp[0].TREF   = u.f;/*  8 Tension: Thermistor1 param: Reference temp for thermistor */
u.ui = ptbl[TENSION_a_THERM1_TEMP_OFFSET]; 	p->ad.tp[0].offset = u.f;/*  9 Tension: Thermistor1 param: Thermistor temp offset correction (deg C) */
u.ui = ptbl[TENSION_a_THERM1_TEMP_SCALE];	p->ad.tp[0].scale  = u.f;/* 10 Tension: Thermistor1 param: Thermistor temp scale correction */
u.ui = ptbl[TENSION_a_THERM2_CONST_B]; 		p->ad.tp[1].B      = u.f;/* 11 Tension: Thermistor2 param: constant B */
u.ui = ptbl[TENSION_a_THERM2_R_SERIES]; 	p->ad.tp[1].RS     = u.f;/* 12 Tension: Thermistor2 param: Series resistor, fixed (K ohms) */
u.ui = ptbl[TENSION_a_THERM2_R_ROOMTMP]; 	p->ad.tp[1].R0     = u.f;/* 13 Tension: Thermistor2 param: Thermistor room temp resistance (K ohms) */
u.ui = ptbl[TENSION_a_THERM2_REF_TEMP];		p->ad.tp[1].TREF   = u.f;/* 14 Tension: Thermistor2 param: Thermistor temp offset correction (deg C) */
u.ui = ptbl[TENSION_a_THERM2_TEMP_OFFSET]; 	p->ad.tp[1].offset = u.f;/* 15 Tension: Thermistor2 param: Thermistor temp scale correction (deg C) */
u.ui = ptbl[TENSION_a_THERM2_TEMP_SCALE]; 	p->ad.tp[1].scale  = u.f; /* 16 Tension: Thermistor2 param: Thermistor temp scale correction */
u.ui = ptbl[TENSION_a_THERM1_COEF_0]; 	p->ad.comp_t1[0] = u.f;   /* 17 Tension: Thermistor1 param: Load-Cell polynomial coefficient 0 (offset) */
u.ui = ptbl[TENSION_a_THERM1_COEF_1];	p->ad.comp_t1[1] = u.f;	 /* 18 Tension: Thermistor1 param: Load-Cell polynomial coefficient 1 (scale) */
u.ui = ptbl[TENSION_a_THERM1_COEF_2];	p->ad.comp_t1[2] = u.f;  /* 19 Tension: Thermistor1 param: Load-Cell polynomial coefficient 2 (x^2) */
u.ui = ptbl[TENSION_a_THERM1_COEF_3];	p->ad.comp_t1[3] = u.f;  /* 20 Tension: Thermistor1 param: Load-Cell polynomial coefficient 3 (x^3) */
u.ui = ptbl[TENSION_a_THERM2_COEF_0];	p->ad.comp_t2[0] = u.f;	 /* 21 Tension: Thermistor2 param: Load-Cell polynomial coefficient 0 (offset) */
u.ui = ptbl[TENSION_a_THERM2_COEF_1];	p->ad.comp_t2[1] = u.f;	 /* 22 Tension: Thermistor2 param: Load-Cell polynomial coefficient 1 (scale) */
u.ui = ptbl[TENSION_a_THERM2_COEF_2];	p->ad.comp_t2[2] = u.f;	 /* 23 Tension: Thermistor2 param: Load-Cell polynomial coefficient 2 (x^2) */
u.ui = ptbl[TENSION_a_THERM2_COEF_3];	p->ad.comp_t2[3] = u.f;	 /* 24 Tension: Thermistor2 param: Load-Cell polynomial coefficient 3 (x^3) */
p->hbct            = ptbl[TENSION_a_HEARTBEAT_CT]; 		 /* 25 Tension: hbct: Heart-Beat Count of time ticks between autonomous msgs */
p->drum            = ptbl[TENSION_a_DRUM_NUMBER]; 		 /* 26 Tension: drum: Drum system number for this function instance */
p->f_pollbit       = ptbl[TENSION_a_DRUM_FUNCTION_BIT]; 	 /* 27 Tension: bit: f_pollbit: Drum system poll 1st byte bit for function instance */
p->p_pollbit       = ptbl[TENSION_a_DRUM_POLL_BIT]; 		 /* 28 Tension: bit: p_pollbit: Drum system poll 2nd byte bit for this type of function */
p->cid_ten_msg     = ptbl[TENSION_a_CANPRM_TENSION]; 		 /* 29 Tension: CANID: cid_ten_msg:  canid msg Tension */
p->cid_ten_poll    = ptbl[TENSION_a_MSG_TIME_POLL]; 		 /* 30 Tension: CANID: cid_ten_msg:  canid MC: Time msg/Group polling */
p->cid_gps_sync    = ptbl[TENSION_a_TIMESYNC]; 			 /* 31 Tension: CANID: cid_gps_sync: canid time: GPS time sync distribution */
p->cid_heartbeat   = ptbl[TENSION_a_HEARTBEAT]; 		 /* 32 Tension: CANID: Heartbeat msg */
p->cid_tst_ten_a   = ptbl[TENSION_a_CANIDTEST]; 		 /* 33 Tension: CANID: Test */
p->iir[0].k        = ptbl[TENSION_a_IIR_POLL_K]; 		 /* 34 Tension: Divisor for IIR filter: polled msg */
p->iir[0].scale    = ptbl[TENSION_a_IIR_POLL_SCALE]; 		 /* 35 Tension: Scale for IIR filter: polled msg */
p->iir[1].k        = ptbl[TENSION_a_IIR_HB_K]; 			 /* 36 Tension: Divisor for IIR filter: heart-beat */
p->iir[1].scale    = ptbl[TENSION_a_IIR_HB_SCALE]; 	 	 /* 37 Tension: Scale for IIR filter: heart-beat */
p->useme           = ptbl[TENSION_a_USEME]; 			 /* 38 Tension: Bits for using this instance */
p->iir_z_recal.k    = ptbl[TENSION_a_IIR_Z_RECAL_K]; 		 /* 39 Tension: IIR Filter factor: divisor sets time constant: zero recalibration */
p->iir_z_recal.scale= ptbl[TENSION_a_IIR_Z_RECAL_SCALE]; 	 /* 40 Tension: IIR Filter scale : upscaling (due to integer math): zero recalibration */
p->z_recal_ct      = ptbl[TENSION_a_Z_RECAL_CT];		 /* 41 Tension: ADC conversion counts between zero recalibrations */
u.ui = ptbl[TENSION_a_LIMIT_HI];	p->limit_hi = u.f;	 /* 42 Tension: Exceeding this limit (+) means invalid reading */
u.ui = ptbl[TENSION_a_LIMIT_LO];	p->limit_lo = u.f;	 /* 43 Tension: Exceeding this limit (-) means invalid reading */
p->code_CAN_filt[0] = ptbl[TENSION_a_CANID_HW_FILT1];		 /* 44 Tension: CAN ID 1 for setting up CAN hardware filter */
p->code_CAN_filt[1] = ptbl[TENSION_a_CANID_HW_FILT2];		 /* 45 Tension: CAN ID 2 for setting up CAN hardware filter */
p->code_CAN_filt[2] = ptbl[TENSION_a_CANID_HW_FILT3];		 /* 46 Tension: CAN ID 3 for setting up CAN hardware filter */
p->code_CAN_filt[3] = ptbl[TENSION_a_CANID_HW_FILT4];		 /* 47 Tension: CAN ID 4 for setting up CAN hardware filter */
p->code_CAN_filt[4] = ptbl[TENSION_a_CANID_HW_FILT5];		 /* 48 Tension: CAN ID 5 for setting up CAN hardware filter */
p->code_CAN_filt[5] = ptbl[TENSION_a_CANID_HW_FILT6];		 /* 49 Tension: CAN ID 6 for setting up CAN hardware filter */
p->code_CAN_filt[6] = ptbl[TENSION_a_CANID_HW_FILT7];		 /* 50 Tension: CAN ID 7 for setting up CAN hardware filter */
p->code_CAN_filt[7] = ptbl[TENSION_a_CANID_HW_FILT8];		 /* 51 Tension: CAN ID 8 for setting up CAN hardware filter */

return PARAM_LIST_CT_TENSION_a;
}


