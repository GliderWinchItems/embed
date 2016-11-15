/******************************************************************************
* File Name          : vcal_idx_v_struct.h
* Date First Issued  : 09/28/2015
* Board              : POD
* Description        : Initialize parameters for vcal
*******************************************************************************/
/*
The struct is setup to all for two voltage sources with ovens, e.g. one to
run the Vref and another for AIN2.
*/

#ifndef __VCAL_IDX_V_STRUCT
#define __VCAL_IDX_V_STRUCT

#include <stdint.h>
#include "common_can.h"
#include "iir_1.h"

 // Thermistor parameters for converting ADC readings to temperature
struct THERMPARAM2
 {   //                   	   default values    description
	float B;		//	3380.0	// Thermistor constant "B" (see data sheets: http://www.murata.com/products/catalog/pdf/r44e.pdf)
	float RS;		//	10.0	// Series resistor, fixed (K ohms)
	float R0;		//	10.0	// Thermistor room temp resistance (K ohms)
	float TREF;		//	298.0	// Reference temp for thermistor
	double poly[4];		//      {0.0, 1.0, 0.0, 0.0} Polynomial correction coefficients
 };

/* Voltage Source Oven */
struct VSOVEN
{
	struct THERMPARAM2 tp;	// Thermistor for this oven instance
	double 	p;		// Proportional coefficient
	double	i;		// Integral coefficient
	double	d;		// Derivative coefficient
	double  setpt;		// Temperature setpt (deg C)
	double  integral;	// Variable: accumulator for integral
	double  deriv_prev;	// Variable: previous value for derivative
	struct IIR_1 iir;	// iir filter for iir LP filter ahead of derivative
};

/* Adjustments for one AD7799 input pair */
struct ADCINPUT
{
	double offset;		// Zero input offset
	double gain;		// Gain factor (reading -> volts)
	double poly[4];		// Temperature correction polynomical coefficients
};
 
 // Overall unit parameters
 struct VCALTHERMS
 {
	struct THERMPARAM2 tp;	// Parameters for thermistor on AD7799
	struct ADCINPUT ain[3];	// Adjustments for each AD7799 input pair.
	struct VSOVEN oven[2];	// Control of oven package
	double vref_tempco;	// Temperature coefficient for Vrefint
	double vref_nominal;	// Vrefint nominal voltage
	double tsens_v25;	// Temperature sensor: voltage at 25 deg C
	double tsens_slope;	// Temperature sensor: slope
	double v2p5_exact;	// Precision voltage source--2.50000v
	struct IIR_1 tsens_iir;	// Temperature sensor: iir filter
	struct IIR_1 vref_iir;	// Vrefint: iir filter
	struct IIR_1 v2p5_iir;	// Vrefint: iir filter
	struct IIR_1 vext_iir;	// Vrefint: iir filter
 };

/* **************************************************************************************/
void vcal_idx_v_struct_init(void);
/* @brief	: Set parameters in struct
 * ************************************************************************************** */

extern struct VCALTHERMS thm;	// struct with TENSION function parameters
extern const void* idx_v_ptr[];

#endif
