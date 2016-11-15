/******************************************************************************
* File Name          : poly_compute_flt.c
* Date First Issued  : 03/27/2015
* Board              :
* Description        : Compute polynomial routine to compensate something
*******************************************************************************/

/* **************************************************************************************
 * float copensation_flt(float *c, int n, float t);
 * @brief	: Compute polynomial of t
 * @param	; c = pointer to coefficients
 * @param	: n = Number of coefficients (size of 'c' array)
 * @param	: t = variable
 * @return	: Polynomial result
 * ************************************************************************************** */
float copensation_flt(float *c, int n, float t)
{

	float x = *(c+0);
	float tt = t;

	for (i = 1; i < n; i++)
	{
		x += (*(c+i) * tt);
		tt *= t;
	}
	return x;
}

