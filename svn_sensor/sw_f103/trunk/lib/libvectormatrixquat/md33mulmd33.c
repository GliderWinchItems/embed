/* **************************************************************************************
 * void md33mulmd33(MD33 *pc, MD33 *pb, MD33 *pa);
 * @brief	: Multiply 3x3 matrix by 3x3 matrix (double precision) C = A x B
 * @param	: pointers to output matrix, input matrix, input matrix
 * ************************************************************************************** */
#include "m3x3.h"
void md33mulmd33(MD33 *pc, MD33 *pb, MD33 *pa)
{
	pc->d11 = (pa->d11 * pb->d11) + (pa->d12 * pb->d21) + (pa->d13 * pb->d31) ;
	pc->d12 = (pa->d11 * pb->d12) + (pa->d12 * pb->d22) + (pa->d13 * pb->d32) ;
	pc->d13 = (pa->d11 * pb->d13) + (pa->d12 * pb->d23) + (pa->d13 * pb->d33) ;

	pc->d21 = (pa->d21 * pb->d11) + (pa->d22 * pb->d21) + (pa->d23 * pb->d31) ;
	pc->d22 = (pa->d21 * pb->d12) + (pa->d22 * pb->d22) + (pa->d23 * pb->d32) ;
	pc->d23 = (pa->d21 * pb->d13) + (pa->d22 * pb->d23) + (pa->d23 * pb->d33) ;

	pc->d31 = (pa->d31 * pb->d11) + (pa->d32 * pb->d21) + (pa->d33 * pb->d31) ;
	pc->d32 = (pa->d31 * pb->d12) + (pa->d32 * pb->d22) + (pa->d33 * pb->d32) ;
	pc->d33 = (pa->d31 * pb->d13) + (pa->d32 * pb->d23) + (pa->d33 * pb->d33) ;

	return;
}

