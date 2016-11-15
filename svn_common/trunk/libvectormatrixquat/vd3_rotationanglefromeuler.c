/* **************************************************************************************
 * double vd3_rotationanglefromeuler(VD3 *e);
 * @brief	: Computer rotation angle from euler (Kuipers p 207)
 * @param	: e = euler angle vector
 * @return	: rotation angle
 * ************************************************************************************** */
#include "vectormatrixquat.h"
double vd3_rotationanglefromeuler(VD3 *e)
{
	double cA = cos(e->x/2);	// Half angle of psi (see p 206)
	double sA = sin(e->x/2);

	double cB = cos(e->y/2);	// Half angle of theta
	double sB = sin(e->y/2);

	double cG = cos(e->z/2);	// Half angle of phi
	double sG = sin(e->z/2);	
	
	double g = (cA * cB * cG) + (sA * sB * sG);

	return (2 * g);
}
