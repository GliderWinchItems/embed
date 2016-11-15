/* **************************************************************************************
 * void vd3_axisofrotation(VD3 *v, MD33 *a);
 * @brief	: get axis of rotation from rotation matrix: Kuipers p 66 eq 3.12
 * @param	: a = pointer to rotation matrix
 * @param	: v = pointer to fixed axis of rotation output vector
 * ************************************************************************************** */
#include "vectormatrixquat.h"
void vd3_axisofrotation(VD3 *v, MD33 *a)
{
	v->x = (a->d23 - a->d32);
	v->y = (a->d31 - a->d13);
	v->z = (a->d12 - a->d21);
	return;
}
