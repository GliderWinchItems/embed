/* **************************************************************************************
 * void vd3_euler123fromquatd(VD3 *vo, QUATD *q);
 * @brief	: Convert quaternion to euler angles (double precision) Deibel eq 290
 * @param	: vo = pointer to output vector with euler angles (radians) (double)
 * @param	: q = pointer to quaternion
 * ************************************************************************************** */
#include "vectormatrixquat.h"
void vd3_euler123fromquatd(VD3 *vo, QUATD *q)
{
	double q0q0 = q->d0 * q->d0;
	double q1q1 = q->d1 * q->d1;
	double q2q2 = q->d2 * q->d2;
	double q3q3 = q->d3 * q->d3;


	double q0q1 = q->d0 * q->d1;
	double q0q2 = q->d0 * q->d2;
	double q0q3 = q->d0 * q->d3;
	double q1q2 = q->d1 * q->d2;
	double q1q3 = q->d1 * q->d3;
	double q2q3 = q->d2 * q->d3;


	vo->x =  atan2( 2 * (q2q3 + q0q1),  (q3q3 - q2q2 - q1q1 + q0q0) );
	vo->y = -asin ( 2 * (q1q3 - q0q2) );
	vo->z  = atan2( 2 * (q1q2 + q0q3),  (q1q1 + q0q0 - q3q3 - q2q2) );
	return;
}
