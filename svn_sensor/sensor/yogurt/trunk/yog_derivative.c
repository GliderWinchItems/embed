/******************************************************************************
* File Name          : yog_derivative.c
* Date First Issued  : 08/23/2015
* Board              : f103
* Description        : Compute derivative of loop error
*******************************************************************************/

#include <stdint.h>

#include "printf.h"
#include "libusartstm32/usartallproto.h"
#include "libmiscstm32/DTW_counter.h"

#define FIRDERIVATIVESIZE 29	// Number of coefficients
static const int32_t a[FIRDERIVATIVESIZE] = { \
-4,
-7,
-10,
-11,
-10,
-9,
-8,
-7,
-6,
-5,
-4,
-3,
-2,
-1,
0,
1,
2,
3,
4,
5,
6,
7,
8,
9,
10,
11,
10,
7,
4,
};

static int32_t r[FIRDERIVATIVESIZE]; // 2x buffer with duplicate values
static int32_t* pr = &r[0];
static uint8_t initsw = 0;	// 0 = past values to current value
/******************************************************************************
 * void yog_derivative_reset(void);
 * @brief	: Initialize past values to current value, upon next poll
*******************************************************************************/
void yog_derivative_reset(void)
{
	initsw = 0;
	return;
}
/******************************************************************************
 * double yog_derivative(double d);
 * @brief	: Compute a FIR based derivative 
 * @param	: d = loop error
 * @return	: derivative
*******************************************************************************/
double yog_derivative(double d)
{
	int32_t accum = 0;
	int i;
	double ret;
	int tmp  = (d * 1000);

	if (initsw == 0)
	{ // Here set all values equal (make derivative zero)
		initsw = 1; 
		pr = &r[0];
		for (i = 0; i < FIRDERIVATIVESIZE; i++)
			*pr++ = tmp;
		pr = &r[0];
		return 0;
	}
	*pr++ = tmp;
	if (pr >= &r[FIRDERIVATIVESIZE]) pr = &r[0];
	for (i = 0; i < FIRDERIVATIVESIZE; i++)
	{ // Accumulate oldest thru current
		accum += (*pr++ * a[i]);
		if (pr >= &r[FIRDERIVATIVESIZE]) pr = &r[0];
	}
	ret = accum;	// Convert to double
	return (ret / 96000); // Return scaled value
}
