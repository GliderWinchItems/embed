/* **************************************************************************************
 * struct AXISANGLEANDPHI axisangle_phi_fromtwoangletiltd(double elev, double bank );
 * @brief	: Compute rotation axis angle and angle of rotation, from elevation and bank
 * @param	: elev = elevation (X axis) angle (radians) measured to horizontal plane (double)
 * @param	: bank = bank (Y axis) angle (radians)  measured to horizontal plane (double)
 * @return	: axis_angle and phi (radians) (double)
 * ************************************************************************************** */
#include "vectormatrixquat.h"
struct AXISANGLEANDPHI axisangle_phi_fromtwoangletiltd(double elev, double bank )
{
	struct AXISANGLEANDPHI aap;
	aap.phi = acos( ( cos(elev) * cos(bank) + cos(elev) + cos(bank) - 1 ) / 2);
	aap.axa = atan2(cos(bank),cos(elev));
	return aap;
}
