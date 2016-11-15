/* **************************************************************************************
 * double md33_rotationangle(MD33 *r);
 * @brief	: Computer rotation angle from rotation matrix (Kuipers p 163)
 * @param	: r = rotation matrix
 * @return	: rotation angle
 * ************************************************************************************** */
#include "vectormatrixquat.h"
#include "m3x3.h"
double md33_rotationangle(MD33 *r)
{
	double tr = md33trace(r);
	return (acos((tr - 1)/2));
}
