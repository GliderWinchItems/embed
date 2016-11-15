/* **************************************************************************************
 * double vd3_magnitude (VD3 *vi);
 * @brief	: Compute magnitude (double precision)
 * @param	: vi = pointer to input
 * @return	: magnitude of vector
 * ************************************************************************************** */
#include "vectormatrixquat.h"
double vd3_magnitude (VD3 *vi)
{
	return sqrt ((vi->x * vi->x) + (vi->y * vi->y) + (vi->z * vi->z));
}
