/******************************************************************************
* File Name          : kalman_scalar.h
* Date First Issued  : 10/22/2015
* Board              : 
* Description        : Scalar Kalman filter
*******************************************************************************/
/*
_Digital and Kalman Filtering_, S.M. Bosic, 2nd ed.,1994, p 128, section 9.2
http://bilgin.esme.org/BitsBytes/KalmanFilterforDummies.aspx
http://www.cs.cornell.edu/courses/cs4758/2012sp/materials/mi63slides.pdf
http://homepages.inf.ed.ac.uk/rbf/CVonline/LOCAL_COPIES/WELCH/kalman.3.html
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __KALMAN_SCALAR
#define __KALMAN_SCALAR

#include <stdint.h>

struct KALMANSC
{
	double svsq;	// Sigma squared, sub v
	double P;	// p(0)  error covariance
	double xhat;	// x^(k) estimate
	double K;	// b(k)  filter gain
	double R;	// Environment noise
	uint32_t k;	// k     reading number (0 - n)

};

/******************************************************************************/
double kalman_scalar_filter(struct KALMANSC* p, double new);
/* @brief	: Update filter with new reading
 * @param	: p = pointer to struct
 * @param	: New reading
 * @param	: New filtered estimated
*******************************************************************************/

#endif

