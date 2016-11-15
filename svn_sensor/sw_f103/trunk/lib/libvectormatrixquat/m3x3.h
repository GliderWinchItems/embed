/******************************************************************************
* File Name          : ms3x3.h
* Date First Issued  : 01/19/2015
* Board              : ...
* Description        : 3x3 matrix routines: single and double float
*******************************************************************************/

#ifndef __MATRIX_SD3X3
#define __MATRIX_SD3X3

#include "vectormatrixquat.h"

/* **************************************************************************************/
void md33mul31(VD3 *pc, MD33 *pb, VD3 *va);
/* @brief	: Multiply 3x3 matrix and column vector (double precision) C = A x B
 * @param	; pointers to output vector, input matrix, input vector
 * ************************************************************************************** */
void ms33mul31(VS3 *pc, MS33 *pb, VS3 *va);
/* @brief	: Multiply 3x3 matrix and column vector (single precision) C = A x B
 * @param	; pointers to output vector, input matrix, input vector
 * ************************************************************************************** */
void vs3tovd3(VD3 *vo, VS3 *vi);
/* @brief	: Convert single prec vector to double precision
 * @param	: vi = pointer to input vector
 * @param	: vo = pointer to output vector
 * ************************************************************************************** */
void vd3tovs3(VS3 *vo, VD3 *vi);
/* @brief	: Convert double prec vector to single precision
 * @param	: vi = pointer to input vector
 * @param	: vo = pointer to output vector
 * ************************************************************************************** */
void md33mulmd33(MD33 *pc, MD33 *pb, MD33 *pa);
/* @brief	: Multiply 3x3 matrix by 3x3 matrix (double precision) C = A x B
 * @param	: pointers to output matrix, input matrix, input matrix
 * ************************************************************************************** */
void md33transpose(MD33 *pb, MD33 *pa);
/* @brief	: Transpose of 3x3 matrix (double precision) B = A^t
 * @param	: pointers to output matrix, input matrix
 * ************************************************************************************** */
void md33mulscalar(MD33 *pb, MD33 *pa, double k);
/* @brief	: Multiply 3x3 matrix by a scalar
 * @param	: pb = pointer to output matrix
 * @param	: pa = pointer to input matrix
 * @param	: scalar
 * ************************************************************************************** */
double md33trace(MD33 *pa);
/* @brief	: Computer trace of a matrix (sum of diagonal elements)
 * @param	: pa = pointer to input matrix
 * @return	: trace
 * ************************************************************************************** */
double md33det(MD33 *pa);
/* @brief	: Compute determinant of 3x3 matrix
 * @param	: pa = pointer to input matrix
 * @return	: determinant
 * ************************************************************************************** */
void md33cofactors(MD33 *pb, MD33 *pa);
/* @brief	: Convert 3x3 matrix to cofactor matrix (double precision)
 * @param	: pointers to output matrix, input matrix
 * ************************************************************************************** */
void md33toms33(MS33 *pb, MD33 *pa);
/* @brief	: Convert double prec matrix to single precision
 * @param	: pa = pointer to input matrix
 * @param	: pb = pointer to output matrix
 * ************************************************************************************** */




extern const MS33 ms33identity;
extern const MD33 md33identity;

#endif 

