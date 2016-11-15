/******************************************************************************
* File Name          : vcal_idx_v_struct.c
* Date First Issued  : 09/28/2015
* Board              : POD
* Description        : Initialize parameters for vcal
*******************************************************************************/
#include <stdint.h>
#include "vcal_idx_v_struct.h"

struct VCALTHERMS thm;	// struct with parameters
/* **************************************************************************************
 * void vcal_idx_v_struct_init(void);
 * @brief	: Set parameters in struct
 * ************************************************************************************** */
void vcal_idx_v_struct_init(void)
{
	/* AD7799 Thermistor (PA3) */
    thm.tp.B			= 3390.0;	/*  3 VCAL_THERM1_CONST_B,	volt_cal: Thermistor1 param: constant B */
    thm.tp.R0			= 10.0;		/*  4 VCAL_THERM1_R_SERIES,	volt_cal: Thermistor1 param: Thermistor room temp resistance (K ohms) */
    thm.tp.RS			= 7.15;		/*  5 VCAL_THERM1_R_ROOMTMP,	volt_cal: Thermistor1 param: Series resistor, fixed (K ohms) */
    thm.tp.TREF			= 290.0;	/*  6 VCAL_THERM1_REF_TEMP,	volt_cal: Thermistor1 param: Reference temp for thermistor */
    thm.tp.poly[0]		= 3.0;		/*  7 VCAL_THERM1_LC_COEF_0,	volt_cal: Thermistor1 param: Load-Cell polynomial coefficient 0 (offset) */
    thm.tp.poly[1]		= 1.00;		/*  8 VCAL_THERM1_LC_COEF_1,	volt_cal: Thermistor1 param: Load-Cell polynomial coefficient 1 (scale) */
    thm.tp.poly[2]		= 0.0;		/*  9 VCAL_THERM1_LC_COEF_2,	volt_cal: Thermistor1 param: Load-Cell polynomial coefficient 2 (x^2) */
    thm.tp.poly[3]		= 0.0;		/* 10 VCAL_THERM1_LC_COEF_3,	volt_cal: Thermistor1 param: Load-Cell polynomial coefficient 3 (x^3) */
    
	/* Oven: first voltage source. */
    thm.oven[0].tp.B		= 3390.0;	/* 11 VCAL_THERM2_CONST_B,	volt_cal:  oven1: Thermistor2 param: constant B */
    thm.oven[0].tp.R0		= 10.0;		/* 12 VCAL_THERM2_R_SERIES,	volt_cal:  oven1: Thermistor2 param: Thermistor room temp resistance (K ohms) */
    thm.oven[0].tp.RS		= 10.0;		/* 13 VCAL_THERM2_R_ROOMTMP,	volt_cal:  oven1: Thermistor2 param: Series resistor, fixed (K ohms) */
    thm.oven[0].tp.TREF		= 290.0;	/* 14 VCAL_THERM2_REF_TEMP,	volt_cal:  oven1: Thermistor2 param: Reference temp for thermistor */
    thm.oven[0].tp.poly[0]	= 5.6;		/* 15 VCAL_THERM2_LC_COEF_0,	volt_cal:  oven1: Thermistor2 param: Load-Cell polynomial coefficient 0 (offset) */
    thm.oven[0].tp.poly[1]	= 1.00;		/* 16 VCAL_THERM2_LC_COEF_1,	volt_cal:  oven1: Thermistor2 param: Load-Cell polynomial coefficient 1 (scale) */
    thm.oven[0].tp.poly[2]	= 0.0;		/* 17 VCAL_THERM2_LC_COEF_2,	volt_cal:  oven1: Thermistor2 param: Load-Cell polynomial coefficient 2 (x^2) */
    thm.oven[0].tp.poly[3]	= 0.0;		/* 18 VCAL_THERM2_LC_COEF_3,	volt_cal:  oven1: Thermistor2 param: Load-Cell polynomial coefficient 3 (x^3) */
    thm.oven[0].p		= 400.0;		/* 19 VCAL_OVEN1_P,		volt_cal:  oven1: Oven control loop, proportional */
    thm.oven[0].i		= 3.0;		/* 20 VCAL_OVEN1_I,		volt_cal:  oven1: Oven control loop, integral */
    thm.oven[0].d		= 1.8E3;		/* 21 VCAL_OVEN1_D,		volt_call: Oven control loop, derivative */
    thm.oven[0].setpt		= ((80.0 - 32.0) * (5.0/9.0));	/* 22 VCAL_OVEN1_SETPT,		volt_cal:  oven1: Oven temperature setpt (degC) */
    thm.oven[0].integral	= 0.0;		/* 23 VCAL_OVEN1_INTEGRAL,	volt_cal:  oven1: variable for holding integral */
    thm.oven[0].deriv_prev	= 50.0;		/* 24 VCAL_OVEN1_INTEGRAL,	volt_cal:  oven1: variable for holding previous sample value for derivative */
    thm.oven[0].iir.a		= 4.0;		/* 24 VCAL_OVEN1_IIR_CO,	volt_cal:  oven1: Coefficient for iir LP filter ahead of derivative */
    
	/* Oven: Second voltage source. */ 
    thm.oven[1].tp.B		= 3390.0;	/* 25 VCAL_THERM3_CONST_B,	volt_cal:  oven2: Thermistor3 param: constant B */
    thm.oven[1].tp.R0		= 10.0;		/* 26 VCAL_THERM3_R_SERIES,	volt_cal:  oven2: Thermistor3 param: Thermistor room temp resistance (K ohms) */
    thm.oven[1].tp.RS		= 10.0;		/* 27 VCAL_THERM3_R_ROOMTMP,	volt_cal:  oven2: Thermistor3 param: Series resistor, fixed (K ohms) */
    thm.oven[1].tp.TREF		= 290.0;	/* 28 VCAL_THERM3_REF_TEMP,	volt_cal:  oven2: Thermistor3 param: Reference temp for thermistor */
    thm.oven[1].tp.poly[0]	= 0.0;		/* 29 VCAL_THERM3_LC_COEF_0,	volt_cal:  oven2: Thermistor3 param: Load-Cell polynomial coefficient 0 (offset) */
    thm.oven[1].tp.poly[1]	= 1.00;		/* 30 VCAL_THERM3_LC_COEF_1,	volt_cal:  oven2: Thermistor3 param: Load-Cell polynomial coefficient 1 (scale) */
    thm.oven[1].tp.poly[2]	= 0.0;		/* 31 VCAL_THERM3_LC_COEF_2,	volt_cal:  oven2: Thermistor3 param: Load-Cell polynomial coefficient 2 (x^2) */
    thm.oven[1].tp.poly[3]	= 0.0;		/* 32 VCAL_THERM3_LC_COEF_3,	volt_cal:  oven2: Thermistor3 param: Load-Cell polynomial coefficient 3 (x^3) */
    thm.oven[1].p		= 11.0;		/* 33 VCAL_OVEN2_P,		volt_cal:  oven2: Oven control loop, proportional */
    thm.oven[1].i		= 0.01;		/* 34 VCAL_OVEN2_I,		volt_cal:  oven2: Oven control loop, integral */
    thm.oven[1].d		= 0.0;		/* 35 VCAL_OVEN2_D,		volt_cal:  oven2: Oven control loop, derivative */
    thm.oven[1].setpt		= ((80.0 - 32.0) * (5.0/9.0));	/* 36 VCAL_OVEN2_SETPT,		volt_cal:  oven2: Oven temperature setpt (degC) */
    thm.oven[1].integral	= 0.0;		/* 37 VCAL_OVEN2_INTEGRAL,	volt_cal:  oven2: variable for holding integral */
    thm.oven[1].deriv_prev	= 0.0;		/* 38 VCAL_OVEN2_INTEGRAL,	volt_cal:  oven2: variable for holding previous sample value for derivative */
    thm.oven[1].iir.a		= 4.0;		/* 24 VCAL_OVEN2_IIR_CO,	volt_cal:  oven2: Coefficient for iir LP filter ahead of derivative */
    
	/* AD7799 input: AIN1 +/-  */
    thm.ain[0].offset		= 0.0;		/* 39 VCAL_INPUT_0_OFFSET,	volt_cal: ad7799 ain1 offset */
    thm.ain[0].gain		= 1.0;		/* 40 VCAL_INPUT_0_GAIN,	volt_cal: ad7799 ain1 gain */
    thm.ain[0].poly[0]		= 0.0;		/* 41 VCAL_INPUT_0_LC_COEF_0,	volt_cal: ad7799 ain1 temperature compensation: coefficient 0 (offset) */
    thm.ain[0].poly[1]		= 1.0;		/* 42 VCAL_INPUT_0_LC_COEF_1,	volt_cal: ad7799 ain1 temperature compensation: coefficient 1 (scale) */
    thm.ain[0].poly[2]		= 0.0;		/* 43 VCAL_INPUT_0_LC_COEF_2,	volt_cal: ad7799 ain1 temperature compensation: coefficient 2 (x^2) */
    thm.ain[0].poly[3]		= 0.0;		/* 44 VCAL_INPUT_0_LC_COEF_3,	volt_cal: ad7799 ain1 temperature compensation: coefficient 3 (x^3) */

	/* AD7799 input: AIN2 +/-  */
    thm.ain[1].offset		= 0.0;		/* 45 VCAL_INPUT_1_OFFSET,	volt_cal: ad7799 ain2 offset */
    thm.ain[1].gain		= 1.0;		/* 46 VCAL_INPUT_1_GAIN,	volt_cal: ad7799 ain2 gain */
    thm.ain[1].poly[0]		= 0.0;		/* 47 VCAL_INPUT_1_LC_COEF_0,	volt_cal: ad7799 ain2 temperature compensation: coefficient 0 (offset) */
    thm.ain[1].poly[1]		= 1.0;		/* 48 VCAL_INPUT_1_LC_COEF_1,	volt_cal: ad7799 ain2 temperature compensation: coefficient 1 (scale) */
    thm.ain[1].poly[2]		= 0.0;		/* 49 VCAL_INPUT_1_LC_COEF_2,	volt_cal: ad7799 ain2 temperature compensation: coefficient 2 (x^2) */
    thm.ain[1].poly[3]		= 0.0;		/* 50 VCAL_INPUT_1_LC_COEF_3,	volt_cal: ad7799 ain2 temperature compensation: coefficient 3 (x^3) */

	/* AD7799 input: AIN3 +/-  */
    thm.ain[2].offset		= 0.0;		/* 51 VCAL_INPUT_2_OFFSET,	volt_cal: ad7799 ain3 offset */
    thm.ain[2].gain		= 1.0;		/* 52 VCAL_INPUT_2_GAIN,	volt_cal: ad7799 ain3 gain */
    thm.ain[2].poly[0]		= 0.0;		/* 53 VCAL_INPUT_2_LC_COEF_0,	volt_cal: ad7799 ain3 temperature compensation: coefficient 0 (offset) */
    thm.ain[2].poly[1]		= 1.0;		/* 54 VCAL_INPUT_2_LC_COEF_1,	volt_cal: ad7799 ain3 temperature compensation: coefficient 1 (scale) */
    thm.ain[2].poly[2]		= 0.0;		/* 55 VCAL_INPUT_2_LC_COEF_2,	volt_cal: ad7799 ain3 temperature compensation: coefficient 2 (x^2) */
    thm.ain[2].poly[3]		= 0.0;		/* 56 VCAL_INPUT_2_LC_COEF_3,	volt_cal: ad7799 ain3 temperature compensation: coefficient 3 (x^3) */

	/* Internal voltage reference and temperature sensor. */
    thm.vref_tempco		= 8E-6;	/* 57 VCAL_INTERNAL_TEMPCO,	volt_cal: Vrefint: Temperature coefficient for Vrefint (ppm/degC) */
    thm.vref_nominal		= 1.20;		/* 57 VCAL_INTERNAL_VREF,	volt_cal: Vrefint: nominal voltage (volts) */
    thm.tsens_v25		= 1.43;		/* 58 VCAL_INTERNAL_T25,	volt_cal: Temperature sensor: voltage at 25 deg C (volts) */
    thm.tsens_slope 		= 4.3;		/* 59 VCAL_INTERNAL_TSLOPE,	volt_cal: Temperature sensor: slope (mv/degC) */
    thm.tsens_iir.a		= 4.0;		/* 60 VCAL_INTERNAL_IIR_CO,	volt_cal: Temperature sensor: iir filter coefficient > 1 */
    thm.vref_iir.a		= 4.0;		/* 61 VCAL_INTERNAL_IIR_CO,	volt_cal: Vrefint: iir filter coefficient > 1 internal volt reference */
    thm.v2p5_iir.a		= 4.0;		/* 62 VCAL_INTERNAL_IIR_CO,	volt_cal: Vrefint: iir filter coefficient > 1 precision volt source*/
    thm.vext_iir.a		= 4.0;		/* 62 VCAL_INTERNAL_EXT_V,	volt_cal: Vrefint: iir filter coefficient > 1 external voltage input */
    thm.v2p5_exact		= 2.500000;	/* 63 VCAL_PREC_VOLT_SRC,	volt_cal: V2p5: Precision 2.5v voltage source, exact voltage. */

	return;
}

