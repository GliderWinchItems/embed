/******************************************************************************
* File Name          : derivative_ave.c
* Date First Issued  : 08/26/2015
* Board              : f103
* Description        : Compute derivative of loop error
*******************************************************************************/

#include <stdint.h>

#include "printf.h"
#include "libusartstm32/usartallproto.h"
#include "libmiscstm32/DTW_counter.h"

#define AVESIZE	32	// Size of running average history
static double a[AVESIZE];

static double sum;
static double* pr = a;
static double prev;

static uint8_t initsw = 0;	// 0 = past values to current value
/******************************************************************************
 * void derivative_ave_reset(void);
 * @brief	: Initialize past values to current value, upon next poll
*******************************************************************************/
void derivative_ave_reset(void)
{
	initsw = 0;
	return;
}
/******************************************************************************
 * double derivative_ave(double d);
 * @brief	: Average of (sample(n) - sample(n-1))
 * @param	: d = loop error
 * @return	: derivative
*******************************************************************************/
double derivative_ave(double d)
{
	int32_t i;

	/* Get sample-to-sample difference. */
	double delta = d - prev;
	prev = d;

	if (initsw == 0)
	{ // Here set all values equal (make derivative zero)
		initsw = 1; 
		pr = a;
		for (i = 0; i < AVESIZE; i++)
			*pr++ = delta;
		sum = delta * AVESIZE;
		pr = a;
		return 0;
	}
	sum -= *pr;	// Subtract old value
	sum += delta;	// Add new value
	*pr++ = delta;	// Insert new value
	if (pr >= &a[AVESIZE]) pr = a;	// Wrap check

	return ( sum * (16.0 / AVESIZE) ); // Return scaled value
}
