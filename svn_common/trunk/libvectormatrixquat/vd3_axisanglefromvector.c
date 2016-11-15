/* **************************************************************************************
 * double vd3_axisanglefromvector(VD3 *vi, double w);
 * @brief	: Compute angle-axis angle (Eq 5 in article)
 * @param	: vi = pointer to initial frame readings
 * @param	: w = vi->z, or axis to be used
 * @return	: angle (radians)
 * ************************************************************************************** */
#include "vectormatrixquat.h"
double vd3_axisanglefromvector(VD3 *vi, double w)
{
	double tmp =sqrt( (vi->x * vi->x) + (vi->y * vi->y) + (vi->z * vi->z) );
	return acos( w / tmp );
}
