/* **************************************************************************************
 * void vd3_degtorad (VD3 *vo, VD3 *vi);
 * @brief	: Convert degrees to radians for vector
 * @param	: vo = pointer to output
 * @param	: vi = pointer to input
 * ************************************************************************************** */
#include "vectormatrixquat.h"
void vd3_degtorad (VD3 *vo, VD3 *vi)
{
	vo->x = DEGRAD * vi->x;
	vo->y = DEGRAD * vi->y;
	vo->z = DEGRAD * vi->z;
	return;
}
