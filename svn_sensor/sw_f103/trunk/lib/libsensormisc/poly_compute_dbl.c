/******************************************************************************
* File Name          : poly_compute_flt.c
* Date First Issued  : 08/02/2015
* Board              :
* Description        : Compute polynomial routine to compensate something
*******************************************************************************/

/* **************************************************************************************
 * double compensation_dbl(const float c[], int n, double t);
 * @brief	: Compute polynomial of t
 * @param	; c = pointer to coefficients
 * @param	: n = Number of coefficients (size of 'c' array)
 * @param	: t = variable to be adjusted 
 * @return	: c[0] + c[1]*t + c[2]*t^2 + c[3]*t^3...+c[n-1]t^(n-1)
 * ************************************************************************************** */
double compensation_dbl(const float c[], int n, double t)
{
	double x = c[0];
	double tt = t;
	int i;

	for (i = 1; i < n; i++)
	{
		x += (c[i] * tt);
		tt *= t;
	}
	return x;
}

