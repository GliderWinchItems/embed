/* **************************************************************************************
 * void eulertoquatd(QUATD *q, VD3 *vi);
 * @brief	: Euler angles to unit Quaternion (double precision) (Eq 297 Diebel pdf)
 * @param	; q = pointer to output quaternion
 * @param	: vi = pointer to input euler angle vector
 * ************************************************************************************** */
#include "vectormatrixquat.h"
void eulertoquatd(QUATD *q, VD3 *vi)
{
/* Vector sequence: q313( phi, theta, psi ) */
	double cphi = cos(vi->x/2);
	double sphi = sin(vi->x/2);

	double ctht = cos(vi->y/2);
	double stht = sin(vi->y/2);

	double cpsi = cos(vi->z/2);
	double spsi = sin(vi->z/2);

	q->d0 =  (cphi * ctht * cpsi) + (sphi * stht * spsi);
	q->d1 = -(cphi * stht * spsi) + (sphi * ctht * cpsi);
	q->d2 =  (cphi * stht * cpsi) + (sphi * ctht * spsi);
	q->d3 =  (cphi * ctht * spsi) - (sphi * stht * cpsi);

	return;

}
