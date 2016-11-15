/* **************************************************************************************
 * void vd3_313from123(VD3 *vo, VD3 *vi);
 * @brief	: Diebel paper: eq 93: Convert seq {1,2,3} to {3,1,3}
 * @param	: vo = pointer to output vector
 * @param	: vi = pointer to input vector
 * ************************************************************************************** */
#include "vectormatrixquat.h"
void vd3_313from123(VD3 *vo, VD3 *vi)
/* Vector: {x, y, z} = {phi, theta, psi} */
{
	double cphi = cos(vi->x);
	double sphi = sin(vi->x);

	double ctht = cos(vi->y);
	double stht = sin(vi->y);

	double cpsi = cos(vi->z);
	double spsi = sin(vi->z);

	vo->x = atan2(-stht, (sphi * ctht));
	vo->y = acos(cphi * ctht);
	vo->z = atan2(((cphi * stht * cpsi) + (sphi * spsi)), ((-cphi * stht * spsi) + (sphi * cpsi)));
	return;
}

