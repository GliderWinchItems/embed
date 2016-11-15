/* **************************************************************************************
 * double qd_magnitude (QUATD *vi);
 * @brief	: Compute magnitude for quaternion (double precision)
 * @param	: vi = pointer to input
 * @return	: magnitude of vector
 * ************************************************************************************** */
#include "vectormatrixquat.h"
double qd_magnitude (QUATD *vi)
{
	return sqrt ((vi->d0 * vi->d0) + (vi->d1 * vi->d1) + (vi->d2 * vi->d2) + (vi->d3 * vi->d3) );
}
