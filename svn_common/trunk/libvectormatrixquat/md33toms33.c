/* **************************************************************************************
 * void md33toms33(MS33 *pb, MD33 *pa);
 * @brief	: Convert double prec matrix to single precision
 * @param	: pa = pointer to input matrix
 * @param	: pb = pointer to output matrix
 * ************************************************************************************** */
#include "m3x3.h"
void md33toms33(MS33 *pb, MD33 *pa)
{
	pb->f11 = pa->d11;
	pb->f12 = pa->d12;
	pb->f13 = pa->d13;

	pb->f21 = pa->d21;
	pb->f22 = pa->d22;
	pb->f23 = pa->d23;

	pb->f31 = pa->d31;
	pb->f32 = pa->d32;
	pb->f33 = pa->d33;

	return;
}

