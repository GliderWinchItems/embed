/* **************************************************************************************
 * void qd_conj(QUATD *qstar, QUATD *q);
 * @brief	: Make complex conjuate q* = q (See ref: p 110, Sec 5.5)
 * @param	; pointers to quaternions
 * ************************************************************************************** */
#include "vectormatrixquat.h"
void qd_conj(QUATD *qstar, QUATD *q)
{
	qstar->d0 = +q->d0;
	qstar->d1 = -q->d1;
	qstar->d2 = -q->d2;
	qstar->d3 = -q->d3;
	return;
}
