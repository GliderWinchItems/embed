/* **************************************************************************************
 * void vd3_eulertovector(VD3 *v, VD3 *e);
 * @brief	: Convert euler angles to vector (Kuipers p 107)
 * @param	; v = pointer to output vector
 * @param	: e = euler input vector
 * ************************************************************************************** */
#include "vectormatrixquat.h"
void vd3_eulertovector(VD3 *v, VD3 *e)
{
	double cA = cos(e->z/2);	// Half angle of psi (see p 206)
	double sA = sin(e->z/2);

	double cB = cos(e->y/2);	// Half angle of theta
	double sB = sin(e->y/2);

	double cG = cos(e->x/2);	// Half angle of phi
	double sG = sin(e->x/2);

	e->x = (cA * cB * sG) - (sA * sB * cG);
	e->y = (cA * sB * cG) + (sA * cB * sG);
	e->z = (sA * cB * cG) - (cA * sB * sG);

}
