/******************************************************************************
* File Name          : derivative_oneside.h
* Date First Issued  : 08/30/2015
* Board              : f103
* Description        : Compute derivative 
*******************************************************************************/
/*
http://www.holoborodko.com/pavel/wp-content/uploads/OneSidedNoiseRobustDifferentiators.pdf
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __YOG_DERIVATIVE_ONESIDE
#define __YOG_DERIVATIVE_ONESIDE

#include <stdint.h>
/******************************************************************************/
void derivative_oneside_reset(void);
/* @brief	: Initialize past values to current value, upon next poll
*******************************************************************************/
double derivative_oneside(double d);
/* @brief	: One-sided derivative
 * @param	: d = loop error
 * @return	: derivative
*******************************************************************************/

#endif 
