/******************************************************************************
* File Name          : yogurt_idx_v_struct.c
* Date First Issued  : 08/08/2015
* Board              : Olimex
* Description        : Translate parameter index into pointer into struct
*******************************************************************************/
#include <stdint.h>
#include "yogurt_idx_v_struct.h"

struct YOGURTTHERMS thm;	// struct with TENSION function parameters

/* Array: paramter index versus pointer into parameter struct. */
void const * idx_v_ptr[] = { \
    &thm.size,		/*  0 Size of list of items */
    &thm.crc,		/*  1 Tension_1: CRC for yogurt list */
    &thm.version,	/*  2 Version number */
    &thm.tp[0].B,	/*  3 YOGURT_THERM1_CONST_B,	Yogurt: Thermistor1 param: constant B */
    &thm.tp[0].R0,	/*  4 YOGURT_THERM1_R_SERIES,	Yogurt: Thermistor1 param: Series resistor, fixed (K ohms) */
    &thm.tp[0].RS,	/*  5 YOGURT_THERM1_R_ROOMTMP,	Yogurt: Thermistor1 param: Thermistor room temp resistance (K ohms) */
    &thm.tp[0].TREF,	/*  6 YOGURT_THERM1_REF_TEMP,	Yogurt: Thermistor1 param: Reference temp for thermistor */
    &thm.tp[0].poly[0],	/*  7 YOGURT_THERM1_LC_COEF_0,	Yogurt: Thermistor1 param: Load-Cell polynomial coefficient 0 (offset) */
    &thm.tp[0].poly[1],	/*  8 YOGURT_THERM1_LC_COEF_1,	Yogurt: Thermistor1 param: Load-Cell polynomial coefficient 1 (scale) */
    &thm.tp[0].poly[2],	/*  9 YOGURT_THERM1_LC_COEF_2,	Yogurt: Thermistor1 param: Load-Cell polynomial coefficient 2 (x^2) */
    &thm.tp[0].poly[3],	/* 10 YOGURT_THERM1_LC_COEF_3,	Yogurt: Thermistor1 param: Load-Cell polynomial coefficient 3 (x^3) */
    \
    &thm.tp[1].B,	/* 11 YOGURT_THERM2_CONST_B,	Yogurt: Thermistor2 param: constant B */
    &thm.tp[1].R0,	/* 12 YOGURT_THERM2_R_SERIES,	Yogurt: Thermistor2 param: Series resistor, fixed (K ohms) */
    &thm.tp[1].RS,	/* 13 YOGURT_THERM2_R_ROOMTMP,	Yogurt: Thermistor2 param: Thermistor room temp resistance (K ohms) */
    &thm.tp[1].TREF,	/* 14 YOGURT_THERM2_REF_TEMP,	Yogurt: Thermistor2 param: Reference temp for thermistor */
    &thm.tp[1].poly[0],	/* 15 YOGURT_THERM2_LC_COEF_0,	Yogurt: Thermistor2 param: Load-Cell polynomial coefficient 0 (offset) */
    &thm.tp[1].poly[1],	/* 16 YOGURT_THERM2_LC_COEF_1,	Yogurt: Thermistor2 param: Load-Cell polynomial coefficient 1 (scale) */
    &thm.tp[1].poly[2],	/* 17 YOGURT_THERM2_LC_COEF_2,	Yogurt: Thermistor2 param: Load-Cell polynomial coefficient 2 (x^2) */
    &thm.tp[1].poly[3],	/* 18 YOGURT_THERM2_LC_COEF_3,	Yogurt: Thermistor2 param: Load-Cell polynomial coefficient 3 (x^3) */
    \
    &thm.tp[2].B,	/* 19 YOGURT_THERM3_CONST_B,	Yogurt: Thermistor3 param: constant B */
    &thm.tp[2].R0,	/* 20 YOGURT_THERM3_R_SERIES,	Yogurt: Thermistor3 param: Series resistor, fixed (K ohms) */
    &thm.tp[2].RS,	/* 21 YOGURT_THERM3_R_ROOMTMP,	Yogurt: Thermistor3 param: Thermistor room temp resistance (K ohms) */
    &thm.tp[2].TREF,	/* 22 YOGURT_THERM3_REF_TEMP,	Yogurt: Thermistor3 param: Reference temp for thermistor */
    &thm.tp[2].poly[0],	/* 23 YOGURT_THERM3_LC_COEF_0,	Yogurt: Thermistor3 param: Load-Cell polynomial coefficient 0 (offset) */
    &thm.tp[2].poly[1],	/* 24 YOGURT_THERM3_LC_COEF_1,	Yogurt: Thermistor3 param: Load-Cell polynomial coefficient 1 (scale) */
    &thm.tp[2].poly[2],	/* 25 YOGURT_THERM3_LC_COEF_2,	Yogurt: Thermistor3 param: Load-Cell polynomial coefficient 2 (x^2) */
    &thm.tp[2].poly[3],	/* 26 YOGURT_THERM3_LC_COEF_3,	Yogurt: Thermistor3 param: Load-Cell polynomial coefficient 3 (x^3) */
    \
    &thm.tp[3].B,	/* 27 YOGURT_THERM4_CONST_B,	Yogurt: Thermistor4 param: constant B */
    &thm.tp[3].R0,	/* 28 YOGURT_THERM4_R_SERIES,	Yogurt: Thermistor4 param: Series resistor, fixed (K ohms) */
    &thm.tp[3].RS,	/* 29 YOGURT_THERM4_R_ROOMTMP,	Yogurt: Thermistor4 param: Thermistor room temp resistance (K ohms) */
    &thm.tp[3].TREF,	/* 30 YOGURT_THERM4_REF_TEMP,	Yogurt: Thermistor4 param: Reference temp for thermistor */
    &thm.tp[3].poly[0],	/* 31 YOGURT_THERM4_LC_COEF_0,	Yogurt: Thermistor4 param: Load-Cell polynomial coefficient 0 (offset) */
    &thm.tp[3].poly[1],	/* 32 YOGURT_THERM4_LC_COEF_1,	Yogurt: Thermistor4 param: Load-Cell polynomial coefficient 1 (scale) */
    &thm.tp[3].poly[2],	/* 33 YOGURT_THERM4_LC_COEF_2,	Yogurt: Thermistor4 param: Load-Cell polynomial coefficient 2 (x^2) */
    &thm.tp[3].poly[3],	/* 34 YOGURT_THERM4_LC_COEF_3,	Yogurt: Thermistor4 param: Load-Cell polynomial coefficient 3 (x^3) */
    \
    &thm.htcl[0].heat,	/* 35 Yogurt: Pasteur: Shell set-point temperature (deg F) heat to this temp */
    &thm.htcl[0].dur,	/* 36 Yogurt: Pasteur: Time duration at temp (hours.frac_hours) */
    &thm.htcl[0].cool,	/* 37 Yogurt: Pasteur: Shell end-point temperature (deg F) cool to this temp */
    \
    &thm.htcl[1].heat,	/* 38 Yogurt: Ferment: Shell set-point temperature (deg F) heat to this temp */
    &thm.htcl[1].dur,	/* 39 Yogurt: Ferment: Time duration at temp (hours.frac_hours) */
    &thm.htcl[1].cool,	/* 40 Yogurt: Ferment: Shell end-point temperature (deg F) cool to this temp */
    \
    &thm.thmidx_shell,	/* 41 YOGURT_1_SHELL_THERM,	Yogurt: Thermistor number for shell temp (0 - 3) */
    &thm.thmidx_pot,	/* 42 YOGURT_1_CTL_THERM_POT,	Yogurt: Thermistor number for center of pot temp (0 - 3) */
    &thm.thmidx_airin,	/* 43 YOGURT_1_CTL_THERM_AIRIN,	Yogurt: Thermistor number for air inlet to fan temp (0 - 3) */
    &thm.thmidx_airout,	/* 44 YOGURT_1_CTL_THERM_AIROUT,Yogurt: Thermistor number for air coming out of holes (0 - 3) */
    \
    &thm.p,		/* 45 YOGURT_1 			Yogurt: Control loop: Proportional coefficient */
    &thm.i,		/* 46 YOGURT_1			Yogurt: Control loop: Integral coefficient */
    &thm.d,		/* 47 YOGURT_1			Yogurt: Control loop: Derivative coefficient */
    &thm.cid_yog_cmd,	/* 48 YOGURT_CMD_YOGURT_1	Yogurt: CANID: cid_yog_cmd: Yogurt maker parameters */
    &thm.cid_yog_msg,	/* 49 YOGURT_MSG_YOGURT_1	Yogurt: CANID: cid_yog_msg: Yogurt maker msgs */
    &thm.cid_yog_hb,	/* 50 YOGURT_HB_YOGURT_1	Yogurt: CANID: cid_yog_hb: Yogurt maker heart-beats */

    &thm.heat_km_p,	/* 50 YOGURT_1_HEATCONSTANT_KM_P	stored heat constant Pasteur phase */
    &thm.heat_km_f,	/* 51 YOGURT_1_HEATCONSTANT_KM_M	stored heat constant Ferment phase */
    &thm.integrate_a,	/* 52 YOGURT_1_INTEGRATEINIT_A		integrator initialization, a of  a + b*x */
    &thm.integrate_b,	/* 53 YOGURT_1_INTEGRATEINIT_B		integrator initialization, b of  a + b*x */
    &thm.stabilize_p,	/* 54 YOGURT_1_STABILIZETIMEDELAY_P	time delay for temperature stabilization, Pasteur */
    &thm.stabilize_f,	/* 55 YOGURT_1_STABILIZETIMEDELAY_F	time delay for temperature stabilization, Ferment */

    NULL		/* -- End of list: (since valid pointers must not be NULL)  */
};

