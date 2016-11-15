/* **************************************************************************************
 * void qd_toeuler(VD3 *e, QUATD *q);
 * @brief	: Convert quaternion to Euler angle vector (Kuipers p 168)
 * @param	; e = pointer to output vector
 * @param	: q = pointer to quaternion
 * ************************************************************************************** */
#include "vectormatrixquat.h"
void qd_toeuler(VD3 *e, QUATD *q)
{
	double q0q0 = q->d0 * q->d0;
	double q0q1 = q->d0 * q->d1;
	double q0q2 = q->d0 * q->d2;
	double q0q3 = q->d0 * q->d3;

	double q1q1 = q->d1 * q->d1;
	double q1q2 = q->d1 * q->d2;
	double q1q3 = q->d1 * q->d3;

	double q2q3 = q->d2 * q->d3;

	double q3q3 = q->d3 * q->d3;

	double m11 = 2 * (q0q0 + q1q1) - 1;
	double m12 = 2 * (q1q2 + q0q3);
	double m13 = 2 * (q1q3 - q0q2);
	double m23 = 2 * (q2q3 + q0q1);
	double m33 = 2 * (q0q0 + q3q3) - 1;

	e->z = atan2(m12, m11);	// psi
	e->y = asin(-m13);	// theta
	e->z = atan2(m23,m33);	// phi
	
	return;
}
