/* **************************************************************************************
 * float vs3_magnitude(VS3 *vi);
 * @brief	: Compute magnitude (single precision)
 * @param	: vi = pointer to input
 * @return	: magnitude of vector
 * ************************************************************************************** */
#include "vectormatrixquat.h"
float vs3_magnitude(VS3 *vi)
{
	return sqrt ((vi->x * vi->x) + (vi->y * vi->y) + (vi->z * vi->z));
}
