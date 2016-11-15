/* **************************************************************************************
 * void ms33mul31(VS3 *pc, MS33 *pb, VS3 *va);
 * @brief	: Multiply 3x3 matrix and column vector (single precision) C = A x B
 * @param	: pointers to output vector, input matrix, input vector
 * ************************************************************************************** */
#include "m3x3.h"
void ms33mul31(VS3 *pc, MS33 *pb, VS3 *va)
{
	pc->x = (pb->f11 * va->x) + (pb->f12 * va->y) + (pb->f13 * va->z);
	pc->y = (pb->f21 * va->x) + (pb->f22 * va->y) + (pb->f23 * va->z);
	pc->z = (pb->f31 * va->x) + (pb->f32 * va->y) + (pb->f33 * va->z);
	return;
}
