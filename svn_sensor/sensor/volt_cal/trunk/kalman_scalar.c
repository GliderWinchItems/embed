/******************************************************************************
* File Name          : kalman_scalar.c
* Date First Issued  : 10/22/2015
* Board              : 
* Description        : Scalar Kalman filter
*******************************************************************************/

#include "kalman_scalar.h"


/******************************************************************************
 * double kalman_scalar_filter(struct KALMANSC* p, double new);
 * @brief	: Update filter with new reading
 * @param	: p = pointer to struct
 * @param	: New reading
 * @param	: New filtered estimated
*******************************************************************************/
/*
struct KALMANSC
{
	double svsq;	// Sigma squared, sub v
	double p0;	// p(0)  error covariance
	double xhat;	// x^(k) estimate
	double K;	// b(k)  filter gain
	double Q;	// 
	double R;	// 
	uint32_t k;	// k     reading number (0 - n)

};
*/
double kalman_scalar_filter(struct KALMANSC* p, double new)
{
	if (p->k == 0)
		p->xhat = new;
	p->K = (p->P / (p->P + p->R)) ;
	p->xhat =  p->xhat + p->K * (new - p->xhat);
	p->P = p->P * (1.0 - p->K);
	p->k += 1;

	return p->xhat;
}

