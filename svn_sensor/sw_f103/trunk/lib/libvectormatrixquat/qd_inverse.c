/* **************************************************************************************
 * void qd_inverse(QUATD *qm1, QUATD *q);
 * @brief	: Make inverse q^(-1) = qstart/(N^2) (See ref: p 112, Sec 5.7)
 * @param	; pointers to quaternions
 * ************************************************************************************** */
#include "vectormatrixquat.h"
void qd_inverse(QUATD *qm1, QUATD *q)
{
	QUATD qstar;
	qd_conj(&qstar, q);
	double x = (1.0/sqrt(qd_normsq(q)));

	qm1->d0 = x * qstar.d0;
	qm1->d1 = x * qstar.d1;
	qm1->d2 = x * qstar.d2;
	qm1->d3 = x * qstar.d3;
	return;
}
