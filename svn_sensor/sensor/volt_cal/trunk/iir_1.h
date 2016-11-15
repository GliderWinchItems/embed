/******************************************************************************
* File Name          : iir_1.h
* Date First Issued  : 09/29/2015
* Board              : --
* Description        : Single pole iir filter
*******************************************************************************/
/*


*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ADC_INTERNAL
#define __ADC_INTERNAL

#include <stdint.h>

struct IIR_1
{
	double a;		// Coefficient
	double z;		// Save value
	double exp;		// exp(coefficient)
	double scale;		// Scale factor
	uint8_t sw;		// init switch
};

#include "vcal_idx_v_struct.h"


/******************************************************************************/
double iir_1(double r, struct IIR_1* p);
/* @brief 	: Single pole IIR Filter 
 * @param	: r = reading to be filtered
 * @param	: p = pointer to iir saved stuff
 * return	: filtered value
*******************************************************************************/

#endif

