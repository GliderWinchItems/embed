/******************************************************************************
* File Name          : tension_idx_v_struct.c
* Date First Issued  : 07/15/2015
* Board              :
* Description        : Translate parameter index into pointer into struct
*******************************************************************************/
#include <stdint.h>
#include "tension_idx_v_struct.h"

struct TENSIONLC ten_a;	// struct with TENSION function parameters

/* Array: paramter index versus pointer into parameter struct. */
//__attribute__ ((section(".text")))
void const * idx_v_ptr[] = {\
    &ten_a.size,		/*  0 Number of elements in the following list */
    &ten_a.crc,			/*  1 Tension_1: CRC for tension list */
    &ten_a.version,		/*  2 Version number */
    &ten_a.ad.offset,		/*  3 TENSION_AD7799_1_OFFSET,	Tension: AD7799 offset */
    &ten_a.ad.scale,		/*  4 TENSION_AD7799_1_SCALE,	Tension: AD7799 #1 Scale (convert to kgf) */
    &ten_a.ad.tp[0].B,		/*  5 TENSION_THERM1_CONST_B,	Tension: Thermistor1 param: constant B */
    &ten_a.ad.tp[0].R0,		/*  6 TENSION_THERM1_R_SERIES,	Tension: Thermistor1 param: Series resistor, fixed (K ohms) */
    &ten_a.ad.tp[0].RS,		/*  7 TENSION_THERM1_R_ROOMTMP,	Tension: Thermistor1 param: Thermistor room temp resistance (K ohms) */
    &ten_a.ad.tp[0].TREF,	/*  8 TENSION_THERM1_REF_TEMP,	Tension: Thermistor1 param: Reference temp for thermistor */
    &ten_a.ad.tp[0].offset,	/*  9 TENSION_THERM1_TEMP_OFFSET,Tension: Thermistor1 param: Thermistor temp offset correction (deg C) */
    &ten_a.ad.tp[0].scale,	/* 10 TENSION_THERM1_TEMP_SCALE,Tension: Thermistor1 param: Thermistor temp scale correction */
    &ten_a.ad.tp[1].B,		/* 11 TENSION_THERM2_CONST_B,	Tension: Thermistor2 param: constant B */
    &ten_a.ad.tp[1].RS,		/* 12 TENSION_THERM2_R_SERIES,	Tension: Thermistor2 param: Series resistor, fixed (K ohms) */
    &ten_a.ad.tp[1].R0,		/* 13 TENSION_THERM2_R_ROOMTMP,	Tension: Thermistor2 param: Thermistor room temp resistance (K ohms) */
    &ten_a.ad.tp[1].TREF,	/* 14 TENSION_THERM2_TEMP_OFFSET,Tension: Thermistor2 param: Thermistor temp offset correction (deg C) */
    &ten_a.ad.tp[1].offset,	/* 15 TENSION_THERM2_TEMP_OFFSET,Tension: Thermistor2 param: Thermistor temp offset correction (deg C) */
    &ten_a.ad.tp[1].scale,	/* 16 TENSION_THERM2_TEMP_SCALE,Tension: Thermistor2 param: Thermistor temp scale correction */
    &ten_a.ad.comp_t1[0],	/* 17 TENSION_THERM1_LC_COEF_0,	Tension: Thermistor1 param: Load-Cell polynomial coefficient 0 (offset) */
    &ten_a.ad.comp_t1[1],	/* 18 TENSION_THERM1_LC_COEF_1,	Tension: Thermistor1 param: Load-Cell polynomial coefficient 1 (scale) */
    &ten_a.ad.comp_t1[2],	/* 19 TENSION_THERM1_LC_COEF_2,	Tension: Thermistor1 param: Load-Cell polynomial coefficient 2 (x^2) */
    &ten_a.ad.comp_t1[3],	/* 20 TENSION_THERM1_LC_COEF_3,	Tension: Thermistor1 param: Load-Cell polynomial coefficient 3 (x^3) */
    &ten_a.ad.comp_t2[0],	/* 21 TENSION_THERM2_LC_COEF_0,	Tension: Thermistor2 param: Load-Cell polynomial coefficient 0 (offset) */
    &ten_a.ad.comp_t2[1],	/* 22 TENSION_THERM2_LC_COEF_1,	Tension: Thermistor2 param: Load-Cell polynomial coefficient 1 (scale) */
    &ten_a.ad.comp_t2[2],	/* 23 TENSION_THERM2_LC_COEF_2,	Tension: Thermistor2 param: Load-Cell polynomial coefficient 2 (x^2) */
    &ten_a.ad.comp_t2[3],	/* 24 TENSION_THERM2_LC_COEF_3,	Tension: Thermistor2 param: Load-Cell polynomial coefficient 3 (x^3) */
    &ten_a.hbct,		/* 25 TENSION_HEARTBEAT_CT	Tension: hbct: Heart-Beat Count of time ticks between autonomous msgs */
    &ten_a.drum,		/* 26 TENSION_DRUM_NUMBER	Tension: drum: Drum system number for this function instance */
    &ten_a.f_pollbit,		/* 27 TENSION_DRUM_FUNCTION_BIT	Tension: bit: f_pollbit: Drum system poll 1st byte bit for function instance */
    &ten_a.p_pollbit,		/* 28 TENSION_DRUM_POLL_BIT	Tension: bit: p_pollbit: Drum system poll 2nd byte bit for this type of function */
    &ten_a.cid_ten_msg,		/* 29 TENSION_DRUM_POLL_BIT	Tension: CANID: cid_ten_msg:  canid msg Tension */
    &ten_a.cid_ten_poll,	/* 30 TENSION_DRUM_FUNCTION_BIT	Tension: CANID: cid_ten_msg:  canid MC: Time msg/Group polling */
    &ten_a.cid_gps_sync,	/* 31 TENSION_TIMESYNC		Tension: CANID: cid_gps_sync: canid time: GPS time sync distribution */
    NULL			/* 00 End of list: (since valid pointers must not be NULL)  */
};


