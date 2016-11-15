/* **************************************************************************************
 * void md33mul31(VD3 *pc, MD33 *pb, VD3 *va);
 * @brief	: Multiply 3x3 matrix and column vector (double precision) C = A x B
 * @param	: pointers to output vector, input matrix, input vector
 * ************************************************************************************** */
#include "m3x3.h"
void md33mul31(VD3 *pc, MD33 *pb, VD3 *va)
{
	pc->x = (pb->d11 * va->x) + (pb->d12 * va->y) + (pb->d13 * va->z);
	pc->y = (pb->d21 * va->x) + (pb->d22 * va->y) + (pb->d23 * va->z);
	pc->z = (pb->d31 * va->x) + (pb->d32 * va->y) + (pb->d33 * va->z);
	return;
}
