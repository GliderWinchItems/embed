/* **************************************************************************************
 * void md33_rotfromquatd(MD33 *m, QUATD *q);
 * @brief	: Convert quaternion to Rotation matrix (Kuipers p 126)
 * @param	; m = pointer to output matrix
 * @param	: q = pointer to quaternion
 * ************************************************************************************** */
#include "vectormatrixquat.h"
void md33_rotfromquatd(MD33 *m, QUATD *q)
{
	double q0q0 = q->d0 * q->d0;
	double q0q1 = q->d0 * q->d1;
	double q0q2 = q->d0 * q->d2;
	double q0q3 = q->d0 * q->d3;

	double q1q1 = q->d1 * q->d1;
	double q1q2 = q->d1 * q->d2;
	double q1q3 = q->d1 * q->d3;

	double q2q2 = q->d2 * q->d2;
	double q2q3 = q->d2 * q->d3;

	double q3q3 = q->d3 * q->d3;

	m->d11 = 2 * (q0q0 + q1q1) - 1;
	m->d12 = 2 * (q1q2 - q0q3);
	m->d13 = 2 * (q1q3 + q0q2);

	m->d21 = 2 * (q1q2 + q0q3);
	m->d22 = 2 * (q0q0 + q2q2) - 1;
	m->d23 = 2 * (q2q3 - q0q1);

	m->d31 = 2 * (q1q3 - q0q2);
	m->d32 = 2 * (q2q3 + q0q1);
	m->d33 = 2 * (q0q0 + q3q3) - 1;
	return;
}
