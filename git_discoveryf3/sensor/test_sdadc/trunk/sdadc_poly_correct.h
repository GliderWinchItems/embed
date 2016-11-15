/******************************************************************************
* File Name          : sdadc_poly_correct.h
* Date First Issued  : 03/20/2016
* Board              : 
* Description        : Polynomial correction of sdadc readings--doubles
*******************************************************************************/
/*


*/
#include <stdint.h>

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SDADC_POLY_CORRECT
#define __SDADC_POLY_CORRECT


struct SDADC_POLY
{
	double	c[7];	// Coefficients: [0] = x^0 term, [1] = x^1, ...
	double	vref;	// Reference voltage used with these coefficients
	double	offset;	// Offset prior to regression
	int	n;	// Number of coefficients
};

/******************************************************************************/
double sdadc_poly_correct(const struct SDADC_POLY *p, double r);
/* @brief 	: Apply polynomial correction
 * @param	: p = pointer to struct with coefficients
 * @param	: r = unadjusted reading
 * @return	: corrected voltage 
*******************************************************************************/
double sdadc_poly_correct_test(int n, double r);
/* @brief 	: Select a polynomial set of coefficients and apply them
 * @param	: n = select the set number
 * @return	: corrected voltage 
*******************************************************************************/


#endif

