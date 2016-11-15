/******************************************************************************
* File Name          : iir_1.c
* Date First Issued  : 09/29/2015
* Board              : --
* Description        : Single pole iir filter
*******************************************************************************/

#include "iir_1.h"
#include <math.h>

/******************************************************************************
 * double iir_1(double r, struct IIR_1* p);
 * @brief 	: Single pole IIR Filter 
 * @param	: r = reading to be filtered
 * @param	: p = pointer to iir saved stuff
 * return	: filtered value
*******************************************************************************/
double iir_1(double r, struct IIR_1* p)
{
	double tmp;
	/* Initialize if not previously done. */
	if (p->sw == 0)
	{ 
		p->sw    = 1; 
		p->exp   = exp( -(1.0/(p->a) ));
		p->scale = (1.0 - p->exp);
		p->z = r;
	}
	/* filter */
	tmp = r + p->z * p->exp;
	p->z = tmp;
	tmp = (tmp * p->scale);
	return tmp;
}

