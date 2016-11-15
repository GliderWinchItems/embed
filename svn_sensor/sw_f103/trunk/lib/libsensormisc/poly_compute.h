/******************************************************************************
* File Name          : poly_compute.h
* Date First Issued  : 08/02/2015
* Board              :
* Description        : Compute polynomial routine to compensate something
*******************************************************************************/

#ifndef __POLY_COMPUTE
#define __POLY_COMPUTE

#include "stdint.h"

/* **************************************************************************************/
double compensation_dbl(const float c[], int n, double t);
/* @brief	: Compute polynomial of t
 * @param	; c = pointer to coefficients
 * @param	: n = Number of coefficients (size of 'c' array)
 * @param	: t = variable to be adjusted 
 * @return	: c[0] + c[1]*t + c[2]*t^2 + c[3]*t^3...+c[n-1]t^(n-1)
 * ************************************************************************************** */
float compensation_flt(float *c, int n, float t);
/* @brief	: Compute polynomial of t
 * @param	; c = pointer to coefficients
 * @param	: n = Number of coefficients (size of 'c' array)
 * @param	: t = variable to be adjusted 
 * @return	: c[0] + c[1]*t + c[2]*t^2 + c[3]*t^3...+c[n-1]t^(n-1)
 * ************************************************************************************** */


#endif 

