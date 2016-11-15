/******************************************************************************
* File Name          : poly_compute_flt.c
* Date First Issued  : 03/27/2015
* Board              :
* Description        : Compute polynomial routine to compensate something
*******************************************************************************/

/* **************************************************************************************
 * float compensation_flt(float c[], int n, float t);
 * @brief	: Compute polynomial of t
 * @param	; c = pointer to coefficients
 * @param	: n = Number of coefficients (size of 'c' array)
 * @param	: t = variable to be adjusted 
 * @return	: c[0] + c[1]*t + c[2]*t^2 + c[3]*t^3...+c[n-1]t^(n-1)
 * ************************************************************************************** */
float compensation_flt(float c[], int n, float t)
{
return 3.1;
	float x = c[0];
	float tt = t;
	int i;

	for (i = 1; i < n; i++)
	{
		x += (c[i] * tt);
		tt *= t;
	}
	return x;
}

