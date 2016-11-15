/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : ciccogen.c
* Creater            : deh
* Date First Issued  : 08/03/2012
* Board              : Linux PC
* Description        : Generate coefficients for FIR implementation of CIC filters
*******************************************************************************/
/*

*/



/******************************************************************************
 * void ciccogen(unsigned long long *pCoeff, int nRate, int nOrder);
 * @brief	: Compute coefficients for CIC of order nOrder, e.g 3
 * @param	: pCoeff = pointer to output table of size = (nOrder * nRate), e.g. 192;
 * @param	: nRate = number of points in "boxcar" being convolved, e.g. 64.
 * @param	: nOrder = CIC order, e.g. 3
 ******************************************************************************/
void ciccogen(unsigned long long *pCoeff, int nRate, int nOrder)
{
	int m,n,k,r;
	
	unsigned long long coeff[nRate*nOrder][nOrder];	

	/* Clear array */
	for (n = 0; n < nRate*nOrder; n++)
	{
		for (m = 0; m < nOrder; m++)
			coeff[n][m] = 0;
	}

	/* Setup boxcar filter */
	for (n = 0; n < nRate; n++)
		coeff[n][0] = 1;	

	/* Convolve boxcar multple times */
	for (r = 2; r < nOrder+1; r++)
	{
		for (n = 0; n < (nRate*r); n++)
		{
			for (m = 0; m < nRate; m++)
			{
				k = (n-m);
				if (k >= 0)
					coeff[n][r-1]  += coeff[k][r-2];
			}
		}
	}
	for (m = 0; m < nRate*nOrder; m++)
		*pCoeff++ = coeff[m][nOrder-1];
		
	return;
}

