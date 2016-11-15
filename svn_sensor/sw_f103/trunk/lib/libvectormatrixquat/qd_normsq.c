/* **************************************************************************************
 * double qd_normsq(QUATD *q);
 * @brief	: Make Norm squared N(q)^2  (See ref: p 111, Sec 5.6)
 * @param	: pointers to quaternions
 * @return	: Norm of quaternion q
 * ************************************************************************************** */
#include "vectormatrixquat.h"
double qd_normsq(QUATD *q)
{
	return	( (q->d0 * q->d0)
		+ (q->d1 * q->d1)
		+ (q->d2 * q->d2)
		+ (q->d3 * q->d3) );
}
