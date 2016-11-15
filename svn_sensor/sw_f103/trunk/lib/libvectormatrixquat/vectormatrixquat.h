/******************************************************************************
* File Name          : vectormatrixquat.h
* Date First Issued  : 01/19/2015
* Board              : ...
* Description        : vector, matrix, quaternion definitions
*******************************************************************************/

#ifndef __VECTORMATRIXQUAT
#define __VECTORMATRIXQUAT

#include <stdint.h>
#include <math.h>

#define RADDEG (180.0/3.14159265358979323)
#define DEGRAD (3.14159265358979323/180.0)
#define PI     (3.14159265358979323)


/* 3 single precsion */
typedef struct
{
	float	x;
	float	y;
	float	z;
}VS3;

typedef struct
{
	float	f11;
	float	f12;
	float	f13;
	
	float	f21;
	float	f22;
	float	f23;

	float	f31;
	float	f32;
	float	f33;
}MS33;

/* 3 double precsion */

typedef struct 
{
	double	f0;
	double	f1;
	double	f2;
	double	f3;
}QUATS;

typedef struct
{
	double	x;	// also, phi
	double	y;	// also, theta
	double	z;	// also, psi
}VD3;


typedef struct
{
	double	d11;
	double	d12;
	
	double	d21;
	double	d22;
}MD22;

typedef struct
{
	double	d11;
	double	d12;
	double	d13;
	
	double	d21;
	double	d22;
	double	d23;

	double	d31;
	double	d32;
	double	d33;
}MD33;


typedef struct 
{
	double	d0;
	double	d1;
	double	d2;
	double	d3;
}QUATD;

struct AXISANGLEANDPHI
{
	double	axa;
	double	phi;
};

/* **************************************************************************************/
void qd_rotation_matrixB(MD33 *r, QUATD *q);
/* @brief	: Compute rotation matrix r from quaternion q (Eq (6) in article)
 * @param	; r = pointer to 3x3 output matix (double precision)
 * @param	: q = pointer to quaternion (doubles)
 * ************************************************************************************** */
void qd_rotation_matrix(MD33 *r, QUATD *q);
/* @brief	: Compute rotation matrix r from quaternion q (Eq (125) attiude.pdf)
 * @param	; r = pointer to 3x3 output matix (double precision)
 * @param	: q = pointer to quaternion (doubles)
 * ************************************************************************************** */
void qd_fromaxisangleandvector(QUATD *q, double ax, VD3 *anorm);
/* @brief	: Compute quaternion from angle-axis and normalized vector
 * @param	: q = pointer to quaternion (double precision)
 * @param	: ax = angle
 * @param	: anorm = pointer to normalized vector (double)
 * ************************************************************************************** */
double vd3_axisanglefromvector(VD3 *vi, double w);
/* @brief	: Compute angle-axis angle (Eq 5 in article)
 * @param	: vi = pointer to initial frame readings
 * @param	: w = vi->z, or axis to be used
 * @return	: angle (radians)
 * ************************************************************************************** */
void vd3_degtorad (VD3 *vo, VD3 *vi);
/* @brief	: Convert degrees to radians for vector
 * @param	: vo = pointer to output
 * @param	: vi = pointer to input
 * ************************************************************************************** */
void vd3_radtodeg (VD3 *vo, VD3 *vi);
/* @brief	: Convert radians to degrees for vector
 * @param	: vo = pointer to output
 * @param	: vi = pointer to input
 * ************************************************************************************** */
void vd3_normalize (VD3 *vo, VD3 *vi);
/* @brief	: Convert radians to degrees for unit vector
 * @param	: vo = pointer to output
 * @param	: vi = pointer to input
 * ************************************************************************************** */ 
void vs3_normalize (VS3 *vo, VS3 *vi);
/* @brief	: Normalize xyz vector
 * @param	: vo = pointer to output
 * @param	: vi = pointer to input
 * ************************************************************************************** */
void qd_normalize (QUATD *vo, QUATD *vi);
/* @brief	: Normalize quaternion vector
 * @param	: vo = pointer to output
 * @param	: vi = pointer to input
 * ************************************************************************************** */
double qd_magnitude (QUATD *vi);
/* @brief	: Compute magnitude for quaternion (double precision)
 * @param	: vi = pointer to input
 * @return	: magnitude of vector
 * ************************************************************************************** */
double qd_magnitude (QUATD *vi);
/* @brief	: Compute magnitude for quaternion (double precision)
 * @param	: vi = pointer to input
 * @return	: magnitude of vector
 * ************************************************************************************** */
double vd3_magnitude (VD3 *vi);
/* @brief	: Compute magnitude (double precision)
 * @param	: vi = pointer to input
 * @return	: magnitude of vector
 * ************************************************************************************** */
float vs3_magnitudeS (VS3 *vi);
/* @brief	: Compute magnitude (single precision)
 * @param	: vi = pointer to input
 * @return	: magnitude of vector
 * ************************************************************************************** */
void eulertoquatd(QUATD *q, VD3 *vi);
/* @brief	: Euler angles to unit Quaternion (double precision) (Eq 84 Deibel pdf)
 * @param	; q = pointer to output quaternion
 * @param	: vi = pointer to input euler angle vector
 * ************************************************************************************** */
void eulertoquatdK(QUATD *q, VD3 *vi);
/* @brief	: Euler angles to unit Quaternion (double precision) (Kuipers pg 167)
 * @param	; q = pointer to output quaternion
 * @param	: vi = pointer to input euler angle vector
 * ************************************************************************************** */
void cosinetoquat(QUATD *q, MD33 *pa);
/* @brief	: Direction cosines to unit Quaternion (double precision) (Kuipers pg 169)
 * @param	; q = pointer to output quaternion
 * @param	: pa = pointer to input direction cosine matrix
 * ************************************************************************************** */
void eulertodirectioncosines(MD33 *c, VD3 *e);
/* @brief	: Euler to Direction Cosine matrix (double precision) (Kuipers pg 169 eq 7.17)
 * @param	; c = pointer to output cosine matrix
 * @param	: e = pointer to input euler angle vector {psi, theta, phi}
 * ************************************************************************************** */
double md33_rotationangle(MD33 *r);
/* @brief	: Computer rotation angle from rotation matrix (Kuipers p 163)
 * @param	: r = rotation matrix
 * @return	: rotation angle
 * ************************************************************************************** */
double vd3_rotationanglefromeuler(VD3 *e);
/* @brief	: Computer rotation angle from euler (Kuipers p 207)
 * @param	: e = euler angle vector
 * @return	: rotation angle
 * ************************************************************************************** */
void vd3_axisofrotation(VD3 *v, MD33 *a);
/* @brief	: get axis of rotation from rotation matrix: Kuipers p 66 eq 3.12
 * @param	: a = pointer to rotation matrix
 * @param	: v = pointer to fixed axis of rotation output vector
 * ************************************************************************************** */
void vd3_123from313(VD3 *vo, VD3 *vi);
/* @brief	: Diebel paper: eq 93: Convert seq {3,1,3} to {1,2,3}
 * @param	: vo = pointer to output vector
 * @param	: vi = pointer to input vector
 * ************************************************************************************** */
void vd3_313from123(VD3 *vo, VD3 *vi);
/* @brief	: Diebel paper: eq 93: Convert seq {1,2,3} to {3,1,3}
 * @param	: vo = pointer to output vector
 * @param	: vi = pointer to input vector
 * ************************************************************************************** */
void qd_mulpq(QUATD *pq, QUATD *p, QUATD *q);
/* @brief	: Multiply: pq = p mul q (See ref: p 156. Eq 7.1)
 * @param	: pointers to quaternion (vectors)
 * ************************************************************************************** */
void qd_mulqp(QUATD *qp, QUATD *p, QUATD *q);
/* @brief	: Multiply: qp = q mul p (See ref: p 157)
 * @param	: pointers to quaternion (vectors)
 * ************************************************************************************** */
void qd_muleff(QUATD *res, QUATD *q1, QUATD *q2);
/* @brief	: Efficient Multiply: q1q2 = q1 mul q2
 * @param	: pointers to quaternions
 * ************************************************************************************** */
void qd_conj(QUATD *qstar, QUATD *q);
/* @brief	: Make complex conjuate q* = q (See ref: p 110, Sec 5.5)
 * @param	; pointers to quaternions
 * ************************************************************************************** */
double qd_normsq(QUATD *q);
/* @brief	: Make Norm squared N(q)^2  (See ref: p 111, Sec 5.6)
 * @param	: pointers to quaternions
 * @return	: Norm of quaternion q
 * ************************************************************************************** */
void qd_inverse(QUATD *qm1, QUATD *q);
/* @brief	: Make inverse q^(-1) = qstart/(N^2) (See ref: p 112, Sec 5.7)
 * @param	; pointers to quaternions
 * ************************************************************************************** */
void md33_rotfromquatd(MD33 *m, QUATD *q);
/* @brief	: Convert quaternion to Rotation matrix (Kuipers p 168)
 * @param	; m = pointer to output matrix
 * @param	: q = pointer to quaternion
 * ************************************************************************************** */
void qd_toeuler(VD3 *e, QUATD *q);
/* @brief	: Convert quaternion to Euler angle vector (Kuipers p 168)
 * @param	; e = pointer to output vector
 * @param	: q = pointer to quaternion
 * ************************************************************************************** */
void vd3_eulertovector(VD3 *v, VD3 *e);
/* @brief	: Convert euler angles to vector (Kuipers p 107)
 * @param	; v = pointer to output vector
 * @param	: e = euler input vector
 * ************************************************************************************** */
struct AXISANGLEANDPHI axisangle_phi_fromtwoangletiltd(double elev, double bank );
/* @brief	: Compute rotation axis angle and angle of rotation, from elevation and bank
 * @param	: elev = elevation (X axis) angle (radians) measured to horizontal plane (double)
 * @param	: bank = bank (Y axis) angle (radians)  measured to horizontal plane (double)
 * @return	: axis_angle and phi (radians) (double)
 * ************************************************************************************** */
void vd3_fromaxisandphid(VD3 *vo, struct AXISANGLEANDPHI aap);
/* @brief	: Compute xyz vector, given the axis_angle and angle of rotation (double precision)
 * @param	: vo = pointer to output vector
 * @param	: aap = axis_angle, phi; (radians) (double)
 * ************************************************************************************** */
void qd_fromvd3andaxisangleandrotation(QUATD *q, VD3 *vi, double rotangle);
/* @brief	: quaternion from vector and axis angle & rotation angle
 * @param	: vi = pointer to input vector
 * @param	: aap = axis_angle, phi; (radians) (double)
 * @param	: q = pointer to output quaternion
 * ************************************************************************************** */
void vd3_euler123fromquatd(VD3 *vo, QUATD *q);
/* @brief	: Convert quaternion to euler angles (double precision) Deibel eq 290
 * @param	: vo = pointer to output vector with euler angles (radians) (double)
 * @param	: q = pointer to quaternion
 * ************************************************************************************** */
void qd_todirectioncosinesd(MD33 *m, QUATD *q);
/* @brief	: Convert quaternion to direction cosine matrix (double) Kuipers p168 
 * @param	; m = pointer to direction cosine matrix output
 * @param	: q = pointer to quaternion input
 * ************************************************************************************** */
void qd_toeuleranglesd(VD3 *vo, QUATD *q);
/* @brief	: Convert quaternion to euler angles (double) Kuipers p168 
 * @param	; vo = pointer to euler angles output vector
 * @param	: q = pointer to quaternion input
 * ************************************************************************************** */
QUATD qd_quatunit(void);
/* @return	: unit quaterion 
 * ************************************************************************************** */
void qd_fromvd3(QUATD *q, VD3 *vi);
/* @brief	: Convert vector to quaternion
 * @param	: q = pointer to quaternion output
 * @param	: vi = pointer to input vector
 * ************************************************************************************** */


#endif

