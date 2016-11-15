/* **************************************************************************************
 * void qd_rotation_matrix(MD33 *r, QUATD *q);
 * @brief	: Compute rotation matrix r from quaternion q (Eq (125) attiude.pdf)
 * @param	; r = pointer to 3x3 output matix (double precision)
 * @param	: q = pointer to quaternion (doubles)
 * ************************************************************************************** */
/* See pg 158 Kuipers (Eq 7.7)...differs from article. ??? */
#include "vectormatrixquat.h"
void qd_rotation_matrix(MD33 *r, QUATD *q)
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

	r->d11 = q0sq + q1sq - q2sq - q3sq;
	r->d12 = 2 * (q1q2 - q0q3);
	r->d13 = 2 * (q1q3 - q0q2); // * +

	r->d21 = 2 * (q1q2 - q0q3); // * +
	r->d22 = q0sq - q1sq + q2sq - q3sq;
	r->d23 = 2 * (q2q3 + q0q1); // * -

	r->d31 = 2 * (q1q3 + q0q2); // * -
	r->d32 = 2 * (q2q3 - q0q1); // * +
	r->d33 = q0sq - q1sq - q2sq + q3sq;

// * = sign differs between 'attitude.pdf' eq 125 and CH Robotic equ 5.
// same for: http://www.slideshare.net/krizie07/quaternion-to-matrix-matrix-to-quaternion
// same for: http://wiki.roblox.com/index.php?title=Quaternions_for_rotation
// same for: http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm
// same for: http://www.geometrictools.com/Documentation/RotationIssues.pdf
// same for: http://run.usc.edu/cs520-s15/quaternions/quaternions-cs520.pdf

	return;
}
