/******************************************************************************
* File Name          : derivative_19.c
* Date First Issued  : 08/25/2015
* Board              : f103
* Description        : Compute derivative 
*******************************************************************************/
/* Based on
http://www.holoborodko.com/pavel/numerical-methods/numerical-derivative/smooth-low-noise-differentiators/#deriv


*/


#include <stdint.h>

#define FIRDERIVATIVESIZE19 19	// Number of coefficients
static const int32_t a[FIRDERIVATIVESIZE19] = { \
-36894,	/*  1 */	
-45552,	/*  2 */		
-28028,	/*  3 */
 -7056,	/*  4 */
  2700,	/*  5 */
  3152,	/*  6 */
  1281,	/*  7 */
   264,	/*  8 */
    23,	/*  9 */
     0,	/*  - */
   -23,	/* -1 */
  -264,	/* -2 */
 -1281,	/* -3 */
 -3152,	/* -4 */
 -2700,	/* -5 */
  7056,	/* -6 */
 45552,	/* -7 */
 28028,	/* -8 */
 36894,	/* -9 */
};

static int32_t r[FIRDERIVATIVESIZE19];
static int32_t* pr = &r[0];
static uint8_t initsw = 0;	// 0 = past values to current value
/******************************************************************************
 * void derivative_19_reset(void);
 * @brief	: Initialize past values to current value, upon next poll
*******************************************************************************/
void derivative_19_reset(void)
{
	initsw = 0;
	return;
}
/******************************************************************************
 * double derivative_19(double d);
 * @brief	: Compute a FIR based derivative 
 * @param	: d = loop error
 * @return	: derivative
*******************************************************************************/
double derivative_19(double d)
{
	int32_t accum = 0;
	int i;
	double ret;
	int tmp  = (d * 1000);

	if (initsw == 0)
	{ // Here set all values equal (make derivative zero)
		initsw = 1; 
		pr = &r[0];
		for (i = 0; i < FIRDERIVATIVESIZE19; i++)
			*pr++ = tmp;
		pr = &r[0];
		return 0;
	}
	*pr++ = tmp;
	if (pr >= &r[FIRDERIVATIVESIZE19]) pr = &r[0];
	for (i = 0; i < FIRDERIVATIVESIZE19; i++)
	{ // Accumulate oldest thru current
		accum += (*pr++ * a[i]);
		if (pr >= &r[FIRDERIVATIVESIZE19]) pr = &r[0];
	}
	ret = accum;	// Convert to double
	return (ret / (1000*393216/2.32)); // Return scaled value
}
