/* **************************************************************************************
 * void qd_todirectioncosinesd(MD33 *m, QUATD *q);
 * @brief	: Convert quaternion to direction cosine matrix (double) Kuipers p168 
 * @param	; m = pointer to direction cosine matrix output
 * @param	: q = pointer to quaternion input
 * ************************************************************************************** */
#include "vectormatrixquat.h"
void qd_todirectioncosinesd(MD33 *m, QUATD *q)
{
	double q0sq = q->d0 * q->d0;
	double q1sq = q->d1 * q->d1;
	double q2sq = q->d2 * q->d2;
	double q3sq = q->d3 * q->d3;


	double q0q1 = q->d0 * q->d1;
	double q0q2 = q->d0 * q->d2;
	double q0q3 = q->d0 * q->d3;
	double q1q2 = q->d1 * q->d2;
	double q1q3 = q->d1 * q->d3;
	double q2q3 = q->d2 * q->d3;

	m->d11 = 2 * (q0sq + q1sq) - 1;
	m->d12 = 2 * (q1q2 + q0q3);
	m->d13 = 2 * (q1q3 - q0q2); 

	m->d21 = 2 * (q1q2 - q0q3);
	m->d22 = 2 * (q0sq + q2sq) - 1;
	m->d23 = 2 * (q2q3 + q0q1);

	m->d31 = 2 * (q1q3 + q0q2);
	m->d32 = 2 * (q2q3 - q0q1);
	m->d33 = 2 * (q0sq + q3sq) - 1;
	return;
}
