/* **************************************************************************************
 * void qd_fromvd3andaxisangleandrotation(QUATD *q, VD3 *vi, double rotangle);
 * @brief	: quaternion from vector and axis angle & rotation angle
 * @param	: vi = pointer to input vector
 * @param	: rotangle = rotation angle (radians) (double)
 * @param	: q = pointer to output quaternion
 * ************************************************************************************** */
#include "vectormatrixquat.h"
void qd_fromvd3andaxisangleandrotation(QUATD *q, VD3 *vi, double rotangle)
{
	double a = rotangle/2;
	double sa = sin(a);
	q->d0 = cos(a);
	q->d1 = vi->x * sa;
	q->d2 = vi->y * sa;
	q->d3 = vi->z * sa;
	return;
}
