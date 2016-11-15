/* **************************************************************************************
 * void vd3_normalize (VD3 *vo, VD3 *vi);
 * @brief	: Normalize xyz vector
 * @param	: vo = pointer to output
 * @param	: vi = pointer to input
 * ************************************************************************************** */
#include "vectormatrixquat.h"
void vd3_normalize (VD3 *vo, VD3 *vi)
{
	double tmp = sqrt ( (vi->x * vi->x) + (vi->y * vi->y) + (vi->z * vi->z) );
	vo->x = vi->x / tmp;
	vo->y = vi->y / tmp;
	vo->z = vi->z / tmp;
	return;
}
