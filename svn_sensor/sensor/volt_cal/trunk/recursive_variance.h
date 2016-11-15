/******************************************************************************
* File Name          : recursive_variance.h
* Date First Issued  : 10/18/2015
* Board              : 
* Description        : Compute the variance recursively
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RECURSIVE_VAR
#define __RECURSIVE_VAR

#include <stdint.h>

struct RECURSIVE
{
	double xnp1;	// x(n+1)	// x(n + 1)
	double xn;	// x(n)		// x(n)
	double xbnp1;	// xbar(n+1)	// xave(n + 1)
	double xbn;	// xbar(n)	// xave(n)
	double snp1;	// sigma(n+1)	// variance(n + 1)
	double sn;	// sigma(n)	// variance(n)
	uint32_t n;	// Count (after initialization)
	uint32_t oto;	// Startup countdown counter
};

/******************************************************************************/
void recursive_variance(struct RECURSIVE* p, double x);
/* @brief	: Compute the variance recursively
 * @param	: p = pointer to struct with stuff for the variable of interest
 * @param	: x = new reading
 * @return	: struct is updated during this computation
*******************************************************************************/
void recursive_variance_reset(struct RECURSIVE* p);
/* @brief	: Reset averaging
 * @param	: p = pointer to struct with stuff for the variable of interest
 * @return	: struct is updated during this computation
*******************************************************************************/


#endif
