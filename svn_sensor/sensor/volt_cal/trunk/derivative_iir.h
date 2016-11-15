/******************************************************************************
* File Name          : derivative_iir.h
* Date First Issued  : 08/31/2015
* Board              : f103
* Description        : Compute derivative w iir filter ahead of simple delta
*******************************************************************************/

#include <stdint.h>


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __YOG_DERIVATIVE_IIR
#define __YOG_DERIVATIVE_IIR

#include <stdint.h>

/* Each filter has the following "control block" */
struct DELTAIIR
{
	double a;	// Coefficient for iir filter
	double z;	// Z^(-1) 
	double prev;	// Previous reading for computing delta
	double exp;	// Precompute iir multiplier
	double scale;	// Precompute iir scale factor
	uint8_t initsw;	// Init switch
};

/******************************************************************************/
void derivative_iir_reset(struct DELTAIIR* p);
/* @brief	: Initialize past values to current value, upon next poll
 * @param	: p = pointer to filter params
*******************************************************************************/
double derivative_iir(struct DELTAIIR* p, double d);
/* @brief	: One-sided derivative
 * @param	: p = pointer to filter params
 * @param	: d = loop error
 * @return	: derivative
*******************************************************************************/

#endif 
