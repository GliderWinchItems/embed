/******************************************************************************
* File Name          : derivative_iir.c
* Date First Issued  : 08/31/2015
* Board              : f103
* Description        : Compute derivative w iir filter ahead of simple delta
*******************************************************************************/

#include <stdint.h>
#include <math.h>
#include "derivative_iir.h"


/******************************************************************************
 * void derivative_iir_reset(struct DELTAIIR* p);
 * @brief	: Initialize past values to current value, upon next poll
 * @param	: p = pointer to filter params
*******************************************************************************/
void derivative_iir_reset(struct DELTAIIR* p)
{
	p->initsw = 0;
	return;
}
/******************************************************************************
 *double derivative_iir(struct DELTAIIR* p, double d);
 * @brief	: One-sided derivative
 * @param	: p = pointer to filter params
 * @param	: d = loop error
 * @return	: derivative
*******************************************************************************/
double derivative_iir(struct DELTAIIR* p, double d)
{
	double tmp;
	double delta;

	if (p->initsw == 0)
	{ // Here set all values equal (make derivative zero)
		p->initsw = 1; 	// No more entries unitl '_reset called
		p->prev = d;	// Previous reading same as current
		p->exp = exp(-(1.0/p->a));
		p->scale = (1.0 - p->exp);
p->scale = p->scale * 13.3;
		p->z = (d * p->a); // Set iir filter to have same output as current
		return 0;
	}

	tmp = d + p->z * p->exp;
	p->z = tmp;
	tmp = (tmp * p->scale);

	// 'tmp' is current iir filter output
	delta = tmp - p->prev;	// Change from last time
	p->prev = tmp;		// Update for next time
	return delta;		// Return sample-to-sample change

}

