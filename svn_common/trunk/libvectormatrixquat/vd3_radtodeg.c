/* **************************************************************************************
 * void vd3_radtodeg (VD3 *vo, VD3 *vi);
 * @brief	: Convert radians to degrees for vector
 * @param	: vo = pointer to output
 * @param	: vi = pointer to input
 * ************************************************************************************** */
#include "vectormatrixquat.h"
void vd3_radtodeg (VD3 *vo, VD3 *vi)
{
	vo->x = RADDEG * vi->x;
	vo->y = RADDEG * vi->y;
	vo->z = RADDEG * vi->z;
	return;
}
