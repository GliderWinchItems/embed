/******************************************************************************
* File Name          : sdadc_poly_correct_f.h
* Date First Issued  : 03/23/2016
* Board              : 
* Description        : Polynomial correction of sdadc readings--float
*******************************************************************************/
/*


*/
#include <stdint.h>

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SDADC_POLY_CORRECT_F
#define __SDADC_POLY_CORRECT_F


struct SDADC_POLY_F
{
	float	c[7];	// Coefficients: [0] = x^0 term, [1] = x^1, ...
	float	vref;	// Reference voltage used with these coefficients
	float	offset;	// Offset prior to regression
	int	n;	// Number of coefficients
};

/******************************************************************************/
float sdadc_poly_correct_f(const struct SDADC_POLY_F *p, float r);
/* @brief 	: Apply polynomial correction
 * @param	: p = pointer to struct with coefficients
 * @param	: r = unadjusted reading
 * @return	: corrected voltage 
*******************************************************************************/
float sdadc_poly_correct_test_f(int n, float r);
/* @brief 	: Select a polynomial set of coefficients and apply them
 * @param	: n = select the set number
 * @return	: corrected voltage 
*******************************************************************************/


#endif

