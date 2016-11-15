/* **************************************************************************************
 * static double md22det(MD22 *pa);
 * @brief	: Compute determinant of 2x2 matrix
 * @param	: pa = pointer to input matrix
 * @return	: determinant
 * ************************************************************************************** */
#include "m3x3.h"
static double md22det(MD22 *pa)
{
	return ((pa->d11 * pa->d22) - (pa->d12 * pa->d21));

}
/* **************************************************************************************
 * double md33det(MD33 *pa);
 * @brief	: Compute determinant of 3x3 matrix
 * @param	: pa = pointer to input matrix
 * @return	: determinant
 * ************************************************************************************** */
#include "m3x3.h"
double md33det(MD33 *pa)
{
	MD22 x;
	x.d11 = pa->d22;
	x.d12 = pa->d23;
	x.d21 = pa->d32;
	x.d22 = pa->d33;
	double det = pa->d11 * md22det(&x);

	x.d11 = pa->d21;
	x.d12 = pa->d23;
	x.d21 = pa->d31;
	x.d22 = pa->d33;
	det -= pa->d12 * md22det(&x);

	x.d11 = pa->d21;
	x.d12 = pa->d22;
	x.d21 = pa->d31;
	x.d22 = pa->d32;
	det += pa->d13 * md22det(&x);
	
	return det;
}
