/* **************************************************************************************
 * float ms33det(MS33 *pa);
 * @brief	: Determinant of 3x3 matrix (single precision)
 * @param	: pointer to input matrix
 * @param	: determinant of matrix A
 * ************************************************************************************** */
#include "m3x3.h"
float ms33det(MS33 *pa)
{
	return ( (pa->f11 * pa->f22 * pa->f33) +
		+ (pa->f12 * pa->f23 * pa->f31)
		+ (pa->f13 * pa->f21 * pa->f32)
		- (pa->f13 * pa->f22 * pa->f31)
		- (pa->f11 * pa->f23 * pa->f32)
		- (pa->f12 * pa->f21 * pa->f33));
}
