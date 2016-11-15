/* **************************************************************************************
 * void eulertodirectioncosines(MD33 *m, VD3 *e);
 * @brief	: Euler to Direction Cosine matrix (double) (Kuipers p167 eq 7.17, p86 eq 4.4, p207)
 * @param	; c = pointer to output cosine matrix
 * @param	: e = pointer to input euler angle vector {psi, theta, phi} -> {zyx} 
 * ************************************************************************************** */
/* Also Diebel pdf, eq 287 R(1,2,3) sequence */
/* Kuipers pg 86 eq 4.4 shows: {phi, theta, psi} -> {RxRyRz} */
/* Kuipers pg 167 eq 7.17 shows {psi, theta, phi} -> {zyx} */
#include "vectormatrixquat.h"
void eulertodirectioncosines(MD33 *m, VD3 *e)
{
	double cpsi = cos(e->z); // xy plane, angle: Rotate around z
	double spsi = sin(e->z);

	double ctht = cos(e->y); // xz plane, angle: Rotate around y
	double stht = sin(e->y);

	double cphi = cos(e->x); // xy plane, angle: Rotate around x
	double sphi = sin(e->x);

	m->d11 = (cpsi * ctht);
	m->d12 = (spsi * ctht);
	m->d13 = -stht;

	m->d21 = (cpsi * stht * sphi) - (spsi * cphi);
	m->d22 = (spsi * stht * sphi) + (cpsi * cphi);
	m->d23 = (ctht * sphi);

	m->d31 = (cpsi * stht * cphi) + (spsi * sphi);
	m->d32 = (spsi * stht * cphi) - (cpsi * sphi);
	m->d33 = (ctht * cphi);

	return;
}
