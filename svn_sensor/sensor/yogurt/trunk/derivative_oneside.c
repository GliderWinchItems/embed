/******************************************************************************
* File Name          : derivative_oneside.c
* Date First Issued  : 08/30/2015
* Board              : f103
* Description        : Compute derivative 
*******************************************************************************/
/*
http://www.holoborodko.com/pavel/wp-content/uploads/OneSidedNoiseRobustDifferentiators.pdf
*/

#include <stdint.h>

#define FIRDERIVATIVESIZE 24
static const int32_t c[24] = { \
      -1,
     -22,	
    -230,	
   -1518,	
   -7084,	
  -24794,	
  -67298,	
 -144210,	
 -245157,	
 -326876,	
 -326876,	
 -208012,
  208012,
  326876,	
  326876,	
  245157,
  144210,
   67298,
   24794,
    7084,
    1518,
     230,
      22,
       1,
};
static int64_t r[FIRDERIVATIVESIZE];
static int64_t* pr = &r[0];
static uint8_t initsw = 0;	// 0 = past values to current value
/******************************************************************************
 * void derivative_oneside_reset(void);
 * @brief	: Initialize past values to current value, upon next poll
*******************************************************************************/
void derivative_oneside_reset(void)
{
	initsw = 0;
	return;
}
/******************************************************************************/
double derivative_oneside(double d);
/* @brief	: One-sided derivative
 * @param	: d = loop error
 * @return	: derivative
*******************************************************************************/
double derivative_oneside(double d)
{
	int64_t ll = (d * 16384); // Convert to fixed pt, scaled
	int64_t accum = 0;
	double ret;
	int i;

	if (initsw == 0)
	{ // Here set all values equal (make derivative zero)
		initsw = 1; 
		pr = r;
		for (i = 0; i < FIRDERIVATIVESIZE; i++)
			*pr++ = ll;
		pr = r;
		return 0;
	}

	*pr++ = ll;	// Insert new value into array
	if (pr >= &r[FIRDERIVATIVESIZE]) pr = r;

	for (i = 0; i < FIRDERIVATIVESIZE; i++)
	{
		accum += *pr++ * c[i];
		if (pr >= &r[FIRDERIVATIVESIZE]) pr = r;
	}
	
	ret = accum;	// Convert back to double
	return (ret * (1.0 / (16384 * 524288.0))); // Scale return
}

