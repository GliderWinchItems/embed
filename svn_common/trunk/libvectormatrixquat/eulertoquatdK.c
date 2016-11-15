/* **************************************************************************************
 * void eulertoquatdK(QUATD *q, VD3 *vi);
 * @brief	: Euler angles to unit Quaternion (double precision) (Kuipers pg 167, sec 7.6)
 * @param	; q = pointer to output quaternion
 * @param	: vi = pointer to input euler angle vector
 * ************************************************************************************** */
#include "vectormatrixquat.h"
void eulertoquatdK(QUATD *q, VD3 *vi)
{
/* Vector: { psi = heading angle; theta = elevation angle; phi = bank angle } */
	double cphi = cos(vi->x/2);
	double sphi = sin(vi->x/2);

	double ctht = cos(vi->y/2);
	double stht = sin(vi->y/2);

	double cpsi = cos(vi->z/2);
	double spsi = sin(vi->z/2);

	q->d0 =  (cpsi * ctht * cphi) + (spsi * stht * sphi);
	q->d1 =  (cpsi * ctht * sphi) - (spsi * stht * cphi);
	q->d2 =  (cpsi * stht * cphi) + (spsi * ctht * sphi);
	q->d3 =  (spsi * ctht * cphi) - (cpsi * stht * sphi);

	return;

}
