/******************************************************************************
* File Name          : sdadc_poly_correct_f.c
* Date First Issued  : 03/23/2016
* Board              : 
* Description        : Polynomial correction of sdadc readings--float
*******************************************************************************/

#include "sdadc_poly_correct_f.h"

/* ######## All data combined, include low input values: order 5 #######################
x = reading; y = volts
Result:  y = -3.780922841·10-26 x5 + 6.320277596·10-21 x4 - 3.596932167·10-16 x3 + 6.869610485·10-12 x2 + 2.739435818·10-5 x - 1.408264496·10-4
Residual Sum of Squares: rss = 2.694333605·10-5
Coefficient of Determination: R2 = 9.999991184·10-1
*/

const struct SDADC_POLY_F sdadc3f = { \
{  
  -1.408264496E-4,	/* [0] */
   2.739435818E-5,	/* [1] */
   6.869610485E-12,	/* [2] */
  -3.596932167E-16,	/* [3] */
   6.320277596E-21,	/* [4] */
  -3.780922841E-26,	/* [5] */
   0.0,			/* [6] */
},
1.847,	/* Vref */
.0025,	/* Offset prior to regression */
6	/* Number of coefficients */
};
/* ######## All data combined, include low input values: order 5 #######################
x = reading; y = volts
Result:  y = -3.780922841·10-26 x5 + 6.320277596·10-21 x4 - 3.596932167·10-16 x3 + 6.869610485·10-12 x2 + 2.739435818·10-5 x - 1.408264496·10-4
Residual Sum of Squares: rss = 2.694333605·10-5
Coefficient of Determination: R2 = 9.999991184·10-1
*/

const struct SDADC_POLY_F sdadc2f = { \
{  
  -1.408264496E-4,	/* [0] */
   2.739435818E-5,	/* [1] */
   6.869610485E-12,	/* [2] */
  -3.596932167E-16,	/* [3] */
   6.320277596E-21,	/* [4] */
  -3.780922841E-26,	/* [5] */
   0.0,			/* [6] */
},
1.847,	/* Vref */
.0025,	/* Offset prior to regression */
6	/* Number of coefficients */
};
/* ######## All data combined, include low input values: order 5 #######################
x = reading; y = volts
Result:  y = -3.780922841·10-26 x5 + 6.320277596·10-21 x4 - 3.596932167·10-16 x3 + 6.869610485·10-12 x2 + 2.739435818·10-5 x - 1.408264496·10-4
Residual Sum of Squares: rss = 2.694333605·10-5
Coefficient of Determination: R2 = 9.999991184·10-1
*/

const struct SDADC_POLY_F sdadc1f = { \
{  
  -1.408264496E-4,	/* [0] */
   2.739435818E-5,	/* [1] */
   6.869610485E-12,	/* [2] */
  -3.596932167E-16,	/* [3] */
   6.320277596E-21,	/* [4] */
  -3.780922841E-26,	/* [5] */
   0.0,			/* [6] */
},
1.847,	/* Vref */
.003,	/* Offset prior to regression */
6	/* Number of coefficients */
};
/******************************************************************************
 * float sdadc_poly_correct_f(const struct SDADC_POLY_F *p, float r);
 * @brief 	: Apply polynomial correction
 * @param	: p = pointer to struct with coefficients
 * @param	: r = unadjusted reading
 * @return	: corrected voltage 
*******************************************************************************/
float sdadc_poly_correct_f(const struct SDADC_POLY_F *p, float r)
{
	float fout = p->c[0];	// Start with constant term
	float rr = r;
	int i;
	for (i = 1; i < p->n; i++)
	{
		fout += rr * p->c[i];
		rr *= r;
	}
	return (fout+p->offset);
}
/******************************************************************************
 * float sdadc_poly_correct_test_f(int n, float r);
 * @brief 	: Select a polynomial set of coefficients and apply them
 * @param	: n = select the set number
 * @return	: corrected voltage; -1 = Wrong 'n'
*******************************************************************************/
float sdadc_poly_correct_test_f(int n, float r)
{
	if (n==3) return sdadc_poly_correct_f(&sdadc3f, r);
	if (n==1) return sdadc_poly_correct_f(&sdadc1f, r);
	if (n==2) return sdadc_poly_correct_f(&sdadc2f, r);
	return -1;
}



   
