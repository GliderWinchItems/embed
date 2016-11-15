/******************************************************************************
* File Name          : yog_derivative.h
* Date First Issued  : 08/23/2015
* Board              : f103
* Description        : Compute derivative of loop error
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __YOG_DERIVATIVE
#define __YOG_DERIVATIVE

#include <stdint.h>
/******************************************************************************/
void yog_derivative_reset(void);
/* @brief	: Initialize past values to current value, upon next poll
******************************************************************************/
double yog_derivative(double d);
/* @brief	: Compute a FIR based derivative 
 * @param	: d = loop err
 * @return	: derivative
*******************************************************************************/

#endif 
