/* **************************************************************************************
 * void qd_mulqp(QUATD *qp, QUATD *p, QUATD *q);
 * @brief	: Multiply: qp = q mul p (See ref: p 157)
 * @param	: pointers to quaternion (vectors)
 * ************************************************************************************** */
#include "vectormatrixquat.h"
void qd_mulqp(QUATD *pq, QUATD *p, QUATD *q)
{
	pq->d0 =  (p->d0 * q->d0) - (p->d1 * q->d1) - (p->d2 * q->d2) - (p->d3 * q->d3);
		
	pq->d1 =  (p->d0 * q->d1) + (p->d1 * q->d0) - (p->d2 * q->d3) + (p->d3 * q->d2);

	pq->d2 =  (p->d0 * q->d2) + (p->d1 * q->d3) + (p->d2 * q->d0) - (p->d3 * q->d1);

	pq->d3 =  (p->d0 * q->d3) - (p->d1 * q->d2) + (p->d2 * q->d1) + (p->d3 * q->d0);

	return;
}
