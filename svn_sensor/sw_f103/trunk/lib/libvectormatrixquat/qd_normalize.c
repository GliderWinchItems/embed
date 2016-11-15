/* **************************************************************************************
 * void qd_normalize (QUATD *vo, QUATD *vi);
 * @brief	: Normalize quaternion vector
 * @param	: vo = pointer to output
 * @param	: vi = pointer to input
 * ************************************************************************************** */
#include "vectormatrixquat.h"
void qd_normalize (QUATD *vo, QUATD *vi)
{
	double tmp = qd_magnitude (vi);
	vo->d0 = vi->d0 / tmp;
	vo->d1 = vi->d1 / tmp;
	vo->d2 = vi->d2 / tmp;
	vo->d3 = vi->d3 / tmp;
	return;
}
