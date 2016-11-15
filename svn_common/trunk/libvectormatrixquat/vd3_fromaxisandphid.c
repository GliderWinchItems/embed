/* **************************************************************************************
 * void vd3_fromaxisandphid(VD3 *vo, struct AXISANGLEANDPHI aap);
 * @brief	: Compute xyz vector, given the axis_angle and angle of rotation (double precision)
 * @param	: vo = pointer to output vector
 * @param	: aap = axis_angle, phi; (radians) (double)
 * ************************************************************************************** */
#include "vectormatrixquat.h"
void vd3_fromaxisandphid(VD3 *vo, struct AXISANGLEANDPHI aap)
{
	vo->x = -sin(aap.phi)*sin(aap.axa); 
	vo->y = -sin(aap.phi)*cos(aap.axa); 
	vo->z  = cos(aap.phi);
	return;
}
