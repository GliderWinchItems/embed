/******************************************************************************
* File Name          : m2x2.h
* Date First Issued  : 01/15/2015
* Board              : ...
* Description        : 3x3 matrix routines
*******************************************************************************/
#include "m2x2.h"

/* **************************************************************************************
 * void m22xmul22(M22 *pc, M22 *pb, M22 *pa)
 * @brief	: Multiply 2x2 matrices: A*B = C
 * @param	; pointers to matrices
 * ************************************************************************************** */
void m22xmul22(M22 *pc, M22 *pb, M22 *pa)
{
	pc->f11 = pa->f11 * pb->f11 + pa->f12 * pb->f21;
	pc->f12 = pa->f11 * pb->f12 + pa->f12 * pb->f22;
	pc->f21 = pa->f21 * pb->f11 + pa->f22 * pb->f21;
	pc->f22 = pa->f21 * pb->f12 + pa->f22 * pb->f22;
	return;
}
/* **************************************************************************************
 * void m22xadd22(M22 *pc, M22 *pb, M22 *pa)
 * @brief	: Add 2x2 matrices: A + B = C
 * @param	; pointers to matrices
 * ************************************************************************************** */
void m22xadd22(M22 *pc, M22 *pb, M22 *pa)
{
	pc->f11 = pa->f11 + pb->f11;
	pc->f12 = pa->f12 + pb->f12;
	pc->f21 = pa->f21 + pb->f21;
	pc->f22 = pa->f22 + pb->f22;
	return;
}
/* **************************************************************************************
 * void m22xsub22(M22 *pc, M22 *pb, M22 *pa)
 * @brief	: Subtract 2x2 matrices: A - B = C
 * @param	; pointers to matrices
 * ************************************************************************************** */
void m22xsub22(M22 *pc, M22 *pb, M22 *pa)
{
	pc->f11 = pa->f11 - pb->f11;
	pc->f12 = pa->f12 - pb->f12;
	pc->f21 = pa->f21 - pb->f21;
	pc->f22 = pa->f22 - pb->f22;
	return;
}
/* **************************************************************************************
 * void m22xtrans(M22 *pb, M22 *pa)
 * @brief	: Transpose 2x2 matrices: B = A transposed
 * @param	; pointers to matrices
 * ************************************************************************************** */
void m22xtrans(M22 *pc, M22 *pb, M22 *pa)
{
	pb->f11 = pa->f11;
	pb->f21 = pa->f12;
	pb->f12 = pa->f21;
	pb->f22 = pa->f22;
	return;
}
/* **************************************************************************************
 * void m22xscale(M22 *pb, M22 *pa, float scale)
 * @brief	: Scale 2x2 matrice: B = A * scale
 * @param	; pointers to matrices
 * ************************************************************************************** */
void m22xtrans(M22 *pc, M22 *pb, M22 *pa)
{
	pb->f11 = pa->f11 * scale;
	pb->f21 = pa->f12 * scale;
	pb->f12 = pa->f21 * scale;
	pb->f22 = pa->f22 * scale;
	return;
}

