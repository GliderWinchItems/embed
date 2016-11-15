/******************************************************************************
* File Name          : derivative_19.h
* Date First Issued  : 08/25/2015
* Board              : f103
* Description        : Compute derivative 
*******************************************************************************/
/* Based on
http://www.holoborodko.com/pavel/numerical-methods/numerical-derivative/smooth-low-noise-differentiators/#deriv
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DERIVATIVE_19
#define __DERIVATIVE_19

#include <stdint.h>

/******************************************************************************/
void derivative_19_reset(void);
/* @brief	: Initialize past values to current value, upon next poll
*******************************************************************************/
double derivative_19(double d);
/* @brief	: Compute a FIR based derivative 
 * @param	: d = loop error
 * @return	: derivative
*******************************************************************************/

#endif 
