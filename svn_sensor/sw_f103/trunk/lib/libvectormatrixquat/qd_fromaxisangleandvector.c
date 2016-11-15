/* **************************************************************************************
 * void qd_fromaxisangleandvector(QUATD *q, double ax, VD3 *anorm);
 * @brief	: Compute quaternion from angle-axis and normalized vector
 * @param	: q = pointer to quaternion vector (double precision)
 * @param	: ax = axis-angle
 * @param	: anorm = pointer to normalized vector (double)
 * ************************************************************************************** */
#include "vectormatrixquat.h"
void qd_fromaxisangleandvector(QUATD *q, double ax, VD3 *anorm)
{
	ax = ax/2;
	double s = sin(ax);

	q->d0 = cos(ax);
	q->d1 = anorm->x * s;
	q->d2 = anorm->y * s;
	q->d3 = anorm->z * s;
	return;
}
