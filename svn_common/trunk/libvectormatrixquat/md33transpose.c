/* **************************************************************************************
 * void md33transpose(MD33 *pb, MD33 *pa);
 * @brief	: Transpose of 3x3 matrix (double precision) B = A^t
 * @param	: pointers to output matrix, input matrix
 * ************************************************************************************** */
#include "m3x3.h"
void md33transpose(MD33 *pb, MD33 *pa)
{
	pb->d11 = pa->d11;
	pb->d12 = pa->d21;
	pb->d13 = pa->d31;

	pb->d21 = pa->d12;
	pb->d22 = pa->d22;
	pb->d23 = pa->d32;

	pb->d31 = pa->d13;
	pb->d32 = pa->d23;
	pb->d33 = pa->d33;

	return;
}
