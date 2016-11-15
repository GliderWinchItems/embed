/* **************************************************************************************
 * void cosinetoquat(QUATD *q, MD33 *pa);
 * @brief	: Direction cosines to unit Quaternion (double precision) (Kuipers pg 169)
 * @param	; q = pointer to output quaternion
 * @param	: pa = pointer to input direction cosine matrix
 * ************************************************************************************** */
#include "vectormatrixquat.h"
void cosinetoquat(QUATD *q, MD33 *pa)
{
	q->d0 = 0.5 * (sqrt(pa->d11 + pa->d22 + pa->d33 + 1));
	double tmp = (4 * q->d0);
	q->d1 = (pa->d23 - pa->d32) / tmp;	
	q->d2 = (pa->d31 - pa->d13) / tmp;	
	q->d3 = (pa->d12 - pa->d21) / tmp;
	
	return;
}
