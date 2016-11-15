/* **************************************************************************************
 * void vd3_123from313(VD3 *vo, VD3 *vi);
 * @brief	: Diebel paper: eq 93: Convert seq {3,1,3} to {1,2,3}
 * @param	: vo = pointer to output vector
 * @param	: vi = pointer to input vector
 * ************************************************************************************** */
#include "vectormatrixquat.h"
void vd3_123from313(VD3 *vo, VD3 *vi)
/* Vector: {x, y, z} = {phi, theta, psi} */
{
	double cphi = cos(vi->x);
	double sphi = sin(vi->x);

	double ctht = cos(vi->y);
	double stht = sin(vi->y);

	double cpsi = cos(vi->z);
	double spsi = sin(vi->z);

	vo->x = atan2((cphi * stht), ctht);
	vo->y = -asin(sphi * stht);
	vo->z = atan2((cphi * spsi + sphi * ctht * cpsi),  (sphi * ctht * spsi));
	return;
}

