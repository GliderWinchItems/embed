/* **************************************************************************************
 * void vs3_normalize(VS3 *vo, VS3 *vi);
 * @brief	: Normalize xyz vector
 * @param	: vo = pointer to output
 * @param	: vi = pointer to input
 * ************************************************************************************** */
#include "vectormatrixquat.h"
void vs3_normalize(VS3 *vo, VS3 *vi)
{
	double tmp = sqrt ( (vi->x * vi->x) + (vi->y * vi->y) + (vi->z * vi->z) );
	vo->x = vi->x / tmp;
	vo->y = vi->y / tmp;
	vo->z = vi->z / tmp;
	return;
}
