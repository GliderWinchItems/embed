/******************************************************************************
* File Name          : derivative_ave.h
* Date First Issued  : 08/26/2015
* Board              : f103
* Description        : Compute derivative of loop error
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __YOG_DERIVATIVE_AVE
#define __YOG_DERIVATIVE_AVE

#include <stdint.h>
/******************************************************************************/
void derivative_ave_reset(void);
/* @brief	: Initialize past values to current value, upon next poll
*******************************************************************************/
double derivative_ave(double d);
/* @brief	: Average of (sample(n) - sample(n-1))
 * @param	: d = loop error
 * @return	: derivative
*******************************************************************************/

#endif 
