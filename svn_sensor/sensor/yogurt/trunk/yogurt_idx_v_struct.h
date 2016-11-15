/******************************************************************************
* File Name          : yogurt_idx_v_struct.h
* Date First Issued  : 08/08/2015
* Board              : Olimex
* Description        : Translate parameter index into pointer into struct
*******************************************************************************/

#include <stdint.h>
#include "common_can.h"

#ifndef __TENSION_IDX_V_STRUCT
#define __TENSION_IDX_V_STRUCT


 // Thermistor parameters for converting ADC readings to temperature
struct THERMPARAM2
 {   //                   	   default values    description
	float B;		//	3380.0	// Thermistor constant "B" (see data sheets: http://www.murata.com/products/catalog/pdf/r44e.pdf)
	float RS;		//	10.0	// Series resistor, fixed (K ohms)
	float R0;		//	10.0	// Thermistor room temp resistance (K ohms)
	float TREF;		//	298.0	// Reference temp for thermistor
	float poly[4];		//      {0.0, 1.0, 0.0, 0.0} Polynomial correction coefficients
 };

struct HEATCOOL
{
	float heat;	// Set-point heating temperature (deg F)
	float dur;	// Hours at temperature
	float cool;	// End-point for cooling (deg F)

};
 
 // Unit has four thermistors
 struct YOGURTTHERMS
 {
	uint32_t size;			// Number of items in struct
 	uint32_t crc;			// crc-32 placed by loader
	uint32_t version;		// struct version number
	struct THERMPARAM2 tp[4];	// Parameters for each thermistor
	struct HEATCOOL htcl[2];	// Heat/Cool profile
	uint32_t thmidx_shell;		// Shell temp thermistor index (0-3)
	uint32_t thmidx_pot;		// Center of pot temp thermistor index (0-3)
	uint32_t thmidx_airin;		// Inlet air temp thermistor index (0-3)
	uint32_t thmidx_airout;		// Outlet air temp thermistor index (0-3)
	float 	p;			// Proportional coefficient
	float	i;			// Integral coefficient
	float	d;			// Derivative coefficient
	uint32_t cid_yog_cmd;		// CANID-command .
	uint32_t cid_yog_msg;		// CANID-poll msgs
	uint32_t cid_yog_hb;		// CANID-heart-beats (plural)
	uint32_t delay;			// Delay start of Pasteur counter (secs)
	float	heat_km_p;		// stored heat constant Pasteur phase
	float	heat_km_f;		// stored heat constant Ferment phase
	float	integrate_a;		// integrator initialization, a of  a + b*x
	float 	integrate_b;		// integrator initialization, b of  a + b*x
	uint32_t stabilize_p;		// time delay for temperature stabilization, Pasteur
	uint32_t stabilize_f;		// time delay for temperature stabilization, Ferment

 };

extern struct YOGURTTHERMS thm;	// struct with TENSION function parameters
extern const void* idx_v_ptr[];

#endif
