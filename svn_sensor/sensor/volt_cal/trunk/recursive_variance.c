/******************************************************************************
* File Name          : recursive_variance.c
* Date First Issued  : 10/18/2015
* Board              : 
* Description        : Compute the variance recursively
*******************************************************************************/

#include "recursive_variance.h"

/******************************************************************************
 * void recursive_variance_reset(struct RECURSIVE* p);
 * @brief	: Reset averaging
 * @param	: p = pointer to struct with stuff for the variable of interest
 * @return	: struct is updated during this computation
*******************************************************************************/
void recursive_variance_reset(struct RECURSIVE* p)
{
	p->n = 0;
//	p->oto = 2;
	return;
}
/******************************************************************************
 * void recursive_variance(struct RECURSIVE* p, double x);
 * @brief	: Compute the variance recursively
 * @param	: p = pointer to struct with stuff for the variable of interest
 * @param	: x = new reading
 * @return	: struct is updated during this computation
*******************************************************************************/
/*
http://math.stackexchange.com/questions/374881/recursive-formula-for-variance
*/
void recursive_variance(struct RECURSIVE* p, double x)
{
	double r;
	double xbnsq;
	double xbnp1sq;
	double xnp1sq;
	p->xnp1 = x;	

	if (p->oto > 0)
	{ // Here, discard readings and init
		p->oto -= 1;	// Count down start up
		p-> xbn = 0;	// Previous Average
		p-> sn = 0;	// Previous Variance(n)
		p->n = 0;	// Count
		p->xn = x;	// Initial previous reading
	}
/*
	uint64_t xnp1;	// x(n+1)	// x(n + 1)
	uint64_t xn;	// x(n)		// x(n)
	double xbnp1;	// xbar(n+1)	// xave(n + 1)
	double xbn	// xbar(n)	// xave(n)
	double snp1;	// sigma(n+1)	// variance(n + 1)
	double sn;	// sigma(n)	// variance(n)
	uint32_t n;	// Count (after initialization)
	uint32_t oto;	// Startup countdown counter
*/
	/* New average (xbar) */
	p->n += 1;
	r = (1.0 / p->n);
	p->xbnp1 = p->xbn + (p->xnp1 - p->xbn) * r;
	
	/* New variance */
	xbnsq = p->xbn * p->xbn;
	xbnp1sq = p->xbnp1 * p->xbnp1;
	xnp1sq = p->xnp1 * p->xnp1;
	p->snp1 = p->sn + xbnsq - xbnp1sq + ((xnp1sq - p->sn - xbnsq) * r);
	
	/* Update */
	p->xn = p->xnp1;
	p->xbn = p->xbnp1;
	p->sn = p->snp1;
	
	return;
	
}
