/* **************************************************************************************
 * void void qd_rotation_matrixB(MD33 *r, QUATD *q);
 * @brief	: Compute rotation matrix r from quaternion q (Eq (6) in article)
 * @param	; r = pointer to 3x3 output matix (double precision)
 * @param	: q = pointer to quaternion (doubles)
 * ************************************************************************************** */
/* See pg 158 Kuipers (Eq 7.7)...differs from article. */
// The following all agree with the code below:
/* http://www.flipcode.com/documents/matrfaq.html#Q54 */
/* http://www.fd.cvut.cz/personal/voracsar/GeometriePG/PGR020/matrix2quaternions.pdf Sec 3 */
/* http://graphics.cs.williams.edu/courses/cs371/s07/reading/quaternions.pdf */
/* https://code.google.com/p/gerardus/source/browse/trunk/matlab/PointsToolbox/quaternion2matrix.m?r=248 */
#include "vectormatrixquat.h"
void qd_rotation_matrixB(MD33 *r, QUATD *q)
{
	double g0g1 = q->d0 * q->d1;
	double g0g2 = q->d0 * q->d2;
	double g0g3 = q->d0 * q->d3;
	double g1g1 = q->d1 * q->d1;
	double g1g2 = q->d1 * q->d2;
	double g1g3 = q->d1 * q->d3;
	double g2g2 = q->d2 * q->d2;
	double g3g3 = q->d3 * q->d3;
	double g2g3 = q->d2 * q->d3;


	r->d11 = 1 - 2 * (g2g2 + g3g3);
	r->d12 =     2 * (g1g2 - g0g3); 
	r->d13 =     2 * (g0g2 + g1g3);

	r->d21 =     2 * (g1g2 + g0g3);
	r->d22 = 1 - 2 * (g1g1 + g3g3);
	r->d23 =     2 * (g2g3 - g0g1);

	r->d31 =     2 * (g1g3 - g0g2);
	r->d32 =     2 * (g0g1 + g2g3);
	r->d33 = 1 - 2 * (g1g1 + g2g2);

	return;
}
