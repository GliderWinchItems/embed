/* **************************************************************************************
 * void qd_mulpq(QUATD *pq, QUATD *p, QUATD *q);
 * @brief	: Multiply: pq = p mul q (See ref: p 156. Eq 7.1)
 * @param	: pointers to quaternion (vectors)
 * ************************************************************************************** */
#include "vectormatrixquat.h"
void qd_mulpq(QUATD *pq, QUATD *p, QUATD *q)
{
	/* Group additions: d1 & d2 typically will be small, closely followed by d3.  d0 is large */

	pq->d0 =  (p->d0 * q->d0) - (((p->d1 * q->d1) + (p->d2 * q->d2)) + (p->d3 * q->d3));
		
	pq->d1 =  (p->d0 * q->d1) + (p->d1 * q->d0) + ((p->d2 * q->d3) - (p->d3 * q->d2));

	pq->d2 =  (p->d0 * q->d2) - (p->d1 * q->d3) + (p->d2 * q->d0) + (p->d3 * q->d1);

	pq->d3 =  (p->d0 * q->d3) + ((p->d1 * q->d2) - (p->d2 * q->d1)) + (p->d3 * q->d0);

	return;
}
