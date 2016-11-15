/******************************************************************************
* File Name          : qrotate.h
* Date First Issued  : 01/19/2015
* Board              : ...
* Description        : quaternion rotation
*******************************************************************************/

#ifndef __QROTATE
#define __QROTATE

#include <stdint.h>
#include "vectormatrixquat.h"





void compute_normalized_vectorY(VD3 *anorm, VD3 *vi);
/* @brief	: Compute normalized vector (Eq 4 in article) Y is direction of gravity
 * @param	: vi = pointer to initial frame readings
 * ************************************************************************************** */
void compute_normalized_vectorZ(VD3 *anorm, VD3 *vi);
/* @brief	: Compute normalized vector (Eq 4 in article) Z is direction of gravity
 * @param	: vi = pointer to initial frame readings
 * ************************************************************************************** */

double xyzmagnitude (VD3 *vi);
/* @brief	: Compute magnitude (double precision)
 * @param	: vo = pointer to output
 * @param	: vi = pointer to input
 * @return	: magnitude of vector
 * ************************************************************************************** */


void vectrotate_xy (VD3 *vo, VD3 *vi, double abank, double aelev);
/* @brief	: Rotate unit vector around y and x.
 * @param	: vo = pointer to output vector
 * @param	: vi = pointer to input vector
 * @param	: abank = angle of bank (radians) (rotate around x)
 * @param	: aelev = angle of evelation/pitch (radians) (rotate around y)
 * ************************************************************************************** */













#endif

