/* **************************************************************************************
 * void qd_fromvd3(QUATD *q, VD3 *vi);
 * @brief	: Convert vector to quaternion
 * @param	: q = pointer to quaternion output
 * @param	: vi = pointer to input vector
 * ************************************************************************************** */
#include "vectormatrixquat.h"
void qd_fromvd3(QUATD *q, VD3 *vi)
{
	q->d0 = 0;
	q->d1 = vi->x;
	q->d2 = vi->y;
	q->d3 = vi->z;
	return;
}
