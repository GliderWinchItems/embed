/* **************************************************************************************
 * void qd_muleff(QUATD *res, QUATD *q1, QUATD *q2);
 * @brief	: Efficient Multiply: q1q2 = q1 mul q2
 * @param	: pointers to quaternions
 * ************************************************************************************** */
/* http://mathinfo.univ-reims.fr/IMG/pdf/Rotating_Objects_Using_Quaternions.pdf page 10. */
#include "vectormatrixquat.h"
void qd_muleff(QUATD *res, QUATD *q1, QUATD *q2)
{
	double A, B, C, D, E, F, G, H;
	A = (q1->d0 + q1->d1)*(q2->d0 + q2->d1);
	B = (q1->d3 - q1->d2)*(q2->d2 - q2->d3);
	C = (q1->d0 - q1->d1)*(q2->d2 + q2->d3);
	D = (q1->d2 + q1->d3)*(q2->d0 - q2->d1);
	E = (q1->d1 + q1->d3)*(q2->d1 + q2->d2);
	F = (q1->d1 - q1->d3)*(q2->d1 - q2->d2);
	G = (q1->d0 + q1->d2)*(q2->d0 - q2->d3);
	H = (q1->d0 - q1->d2)*(q2->d0 + q2->d3);
	res->d0 = B + (-E - F + G + H) /2;
	res->d1 = A - (E + F + G + H)/2;
	res->d2 = C + (E - F + G - H)/2;
	res->d3 = D + (E - F - G + H)/2;
	return;
} 
