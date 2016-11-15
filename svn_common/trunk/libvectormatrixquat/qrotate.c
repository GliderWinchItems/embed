/******************************************************************************
* File Name          : qrotate.c
* Date First Issued  : 01/19/2015
* Board              : ...
* Description        : quaternion rotation
*******************************************************************************/
/*
These may not work correctly!!!
*/
#include <stdio.h>
#include <math.h>
#include "vectormatrixquat.h"
#include "qrotate.h"
#include "m3x3.h"





/* **************************************************************************************
 * void compute_normalized_vectorY(VD3 *anorm, VD3 *vi);
 * @brief	: Compute normalized vector (Eq 4 in article) Y is direction of gravity
 * @param	: vi = pointer to initial frame readings
 * ************************************************************************************** */
void compute_normalized_vectorY(VD3 *anorm, VD3 *vi)
{
	double tmp = sqrt ( (vi->x * vi->x) + (vi->z * vi->z) );
	anorm->x = (vi->z / tmp);
	anorm->y = 0;
	anorm->z = -(vi->x / tmp);
	return;
}
/* **************************************************************************************
 * void compute_normalized_vectorZ(VD3 *anorm, VD3 *vi);
 * @brief	: Compute normalized vector (Eq 4 in article) Z is direction of gravity
 * @param	: vi = pointer to initial frame readings
 * ************************************************************************************** */
void compute_normalized_vectorZ(VD3 *anorm, VD3 *vi)
{
	double tmp = sqrt ( (vi->y * vi->y) + (vi->x * vi->x) );
	anorm->x = (vi->x / tmp);
	anorm->y = (vi->y / tmp);
	anorm->z = 0;
	return;
}


/* **************************************************************************************
 * void eulertoxyz (VD3 *vo, VD3 *vi);
 * @brief	: Convert euler angles to unit vector
 * @param	: vo = pointer to output
 * @param	: vi = pointer to input
 * ************************************************************************************** */
#include "vectormatrixquat.h"
void eulertoxyz (VD3 *vo, VD3 *vi)
{
	vo->x = cos(vi->y) * cos(vi->z);
	vo->y = cos(vi->y) * sin(vi->z);
	vo->z = sin(vi->y);
	return;
}

/* **************************************************************************************
 * void vectrotate_xy (VD3 *vo, VD3 *vi, double abank, double aelev);
 * @brief	: Rotate unit vector around y and x.
 * @param	: vo = pointer to output vector
 * @param	: vi = pointer to input vector
 * @param	: abank = angle of bank (radians) (rotate around x)
 * @param	: aelev = angle of evelation/pitch (radians) (rotate around y)
 * ************************************************************************************** */
#include "vectormatrixquat.h"
void vectrotate_xy(VD3 *vo, VD3 *vi, double abank, double aelev)
{
	// Rotation is around Y axis
	vo->x = (vi->x * cos(aelev)) - (vi->z * sin(aelev));
	vo->z = (vi->z * cos(aelev));
	// Rotation is around X axis
	vo->y = (vi->y * cos(abank)) - (vo->z * sin(abank));
	vo->z = (vo->z * cos(abank));
	return;
}












