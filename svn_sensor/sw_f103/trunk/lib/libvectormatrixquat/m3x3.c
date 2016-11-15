/******************************************************************************
* File Name          : m3x3.c
* Date First Issued  : 01/19/2015
* Board              : ...
* Description        : 3x3 matrix routines: single precision float
*******************************************************************************/
#include "m3x3.h"

/* Identity matrices, double and single precision. */
const MD33 md33identity = {1,0,0,  0,1,0,  0,0,1};
const MS33 ms33identity = {1,0,0,  0,1,0,  0,0,1};


/* **************************************************************************************
 * void vs3tovd3(VD3 *vo, VS3 *vi);
 * @brief	: Convert single prec vector to double precision
 * @param	: vi = pointer to input vector
 * @param	: vo = pointer to output vector
 * ************************************************************************************** */
#include "m3x3.h"
void vs3tovd3(VD3 *vo, VS3 *vi)
{
	vo->x = vi->x;
	vo->y = vi->y;
	vo->z = vi->z;
	return;
}
/* **************************************************************************************
 * void vd3tovs3(VS3 *vo, VD3 *vi);
 * @brief	: Convert double prec vector to single precision
 * @param	: vi = pointer to input vector
 * @param	: vo = pointer to output vector
 * ************************************************************************************** */
#include "m3x3.h"
void vd3tovs3(VS3 *vo, VD3 *vi)
{
	vo->x = vi->x;
	vo->y = vi->y;
	vo->z = vi->z;
	return;
}

/* **************************************************************************************
 * void md33mulscalar(MD33 *pb, MD33 *pa, double k);
 * @brief	: Multiply 3x3 matrix by a scalar
 * @param	: pb = pointer to output matrix
 * @param	: pa = pointer to input matrix
 * @param	: scalar
 * ************************************************************************************** */
#include "m3x3.h"
void md33mulscalar(MD33 *pb, MD33 *pa, double k)
{
	pb->d11 = pa->d11 * k;
	pb->d12 = pa->d12 * k;
	pb->d13 = pa->d13 * k;

	pb->d21 = pa->d21 * k;
	pb->d22 = pa->d22 * k;
	pb->d23 = pa->d23 * k;

	pb->d31 = pa->d31 * k;
	pb->d32 = pa->d32 * k;
	pb->d33 = pa->d33 * k;
	return;
}
/* **************************************************************************************
 * double md33trace(MD33 *pa);
 * @brief	: Computer trace of a matrix (sum of diagonal elements)
 * @param	: pa = pointer to input matrix
 * @return	: trace
 * ************************************************************************************** */
#include "m3x3.h"
double md33trace(MD33 *pa)
{
	return (pa->d11 + pa->d22 + pa->d33);
}

/* **************************************************************************************
 * void md33cofactors(MD33 *pb, MD33 *pa);
 * @brief	: Convert 3x3 matrix to cofactor matrix (double precision)
 * @param	: pointers to output matrix, input matrix
 * ************************************************************************************** */
#include "m3x3.h"
static double co2(double a11, double a12, double a22, double a21)
{
	return	( (a11 * a22) - (a12 * a21) );
}
void md33cofactors(MD33 *pb, MD33 *pa)
{
	pb->d11 =  co2(pa->d22, pa->d23, pa->d32, pa->d33); // 1 1
	pb->d12 = -co2(pa->d21, pa->d23, pa->d31, pa->d33); // 1 2
	pb->d13 =  co2(pa->d21, pa->d22, pa->d31, pa->d32); // 1 3

	pb->d21 = -co2(pa->d12, pa->d13, pa->d32, pa->d33); // 2 1
	pb->d22 =  co2(pa->d12, pa->d13, pa->d31, pa->d33); // 2 2
	pb->d23 = -co2(pa->d12, pa->d12, pa->d31, pa->d32); // 2 3

	pb->d31 =  co2(pa->d12, pa->d13, pa->d22, pa->d23); // 3 1
	pb->d32 = -co2(pa->d11, pa->d13, pa->d21, pa->d23); // 3 2
	pb->d33 =  co2(pa->d11, pa->d12, pa->d21, pa->d22); // 3 3

	return;
}


