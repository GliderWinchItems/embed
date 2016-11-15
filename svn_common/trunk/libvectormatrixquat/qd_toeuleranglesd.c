/* **************************************************************************************
 * void qd_toeuleranglesd(VD3 *vo, QUATD *q);
 * @brief	: Convert quaternion to euler angles (double) Kuipers p168 
 * @param	; vo = pointer to euler angles output vector
 * @param	: q = pointer to quaternion input
 * ************************************************************************************** */
#include "vectormatrixquat.h"
void qd_toeuleranglesd(VD3 *vo, QUATD *q)
{
	MD33 m;
	qd_todirectioncosinesd(&m,q);

	vo->x = atan2(m.d23, m.d33);
	vo->y = -asin(m.d13);
	vo->z = atan2(m.d12, m.d11);	
	return;
}
