/*
pctestq.c // Test quaternion rotation and math routines
01/23/2015
pctest arguments: bank elevation  accel_x accel_y accel_z accelrun_x accelrun_y accelrun_z
for winch calibration tilt paired with accelerometer calibration readings, then tested with 
  accelerometer "run" readings.

To compile and runcut & paste the following to a terminal--
cd ~/svn_sensor/PC/testvectormatrixquat/trunk/vmq
./mm 10 10  0.172658573005743   0.172658573005743    0.969730908208685   -0.172658573005743   -0.172658573005743    0.969730908208685
OR,
./pctestq 10 10  0.172658573005743   0.172658573005743    0.969730908208685   -0.172658573005743   -0.172658573005743    0.969730908208685
*/
#include <stdio.h>
#include <math.h>

#include "vectormatrixquat.h"
#include "qrotate.h"
#include "quaternionmath.h"
#include "m3x3.h"



static void printvectorD(char *p, VD3 *v);
//static void printvectorS(char *p, VS3 *v);
static void printquat(char *p, QUATD *q);
//static void printms33(char *p, MS33 *m);
static void printmd33(char *p, MD33 *m);
static void printdashline(int j);
static void printseparator(char *p);
static void printvectoranglesD(char *p, VD3 *vi);
static void matrixchk(char *p, MD33 *m);
static void printquatsummary(char *p, QUATD *q);


/* Winch frame with respect to world frame. */
#define WFRAMEANGLEBANK	10.0	// Default frame reference Euler angle phi  : bank/roll
#define WFRAMEANGLEELEV	5.0	// Default frame reference Euler angle theta: elevation/pitch

/* Accelerometer "readings" paired with winch frame above. */
#define REFREADANGLEBANK	45.0	// Default readings reference Euler angle phi  : bank/roll
#define REFREADANGLEELEV	60.0	// Default readings reference Euler angle theta: elevation/pitch



int main(int argc, char *argv[])
{
	VD3 vxr;

	VD3 vd001 = {0,0,1};

	/* Input angles for calibration. */
	double abankref;	// Winch bank
	double aelevref;	// Winch elevation

	VD3 accelcal;	// Accelerometer calibration readings vector {x y z}
	VD3 accelrun;	// Accelerometer readings "run" when winch tilts


	double rot_angle;


	/* In actual use, the accelerometer input would be the xyz readings rather than angles. */
	
	printdashline(10);
	printf("\n### pctestq.c (Test for winch tilt routines) 01-26-2015\n");
	printf("    Command-line ARGUMENTS: winch ANGLES (deg): bank elevation accelerometer readings (+/-g) x y z\n");

	if (argc != 9)
	{
		printf("arg count: %u\n",argc); return -1;
//	printf("Just 8 arguments (not %u)\n\tthe frame reference pitch, bank (deg)\n\tx,y,z accelerometer readings for winch calibration\n\t x,y,z accelerometer readings tilt of winch e.g\n\t5.6 10.2  .982 .251 -.350 1500 1500 6000\n", argc); return -1;
	}

	// Here, convert args entered on command linerot_angle
	sscanf (argv[1],"%lf", &abankref);
	sscanf (argv[2],"%lf", &aelevref);
		
	sscanf (argv[3],"%lf", &accelcal.x);
	sscanf (argv[4],"%lf", &accelcal.y);
	sscanf (argv[5],"%lf", &accelcal.z);

	sscanf (argv[6],"%lf", &accelrun.x);
	sscanf (argv[7],"%lf", &accelrun.y);
	sscanf (argv[8],"%lf", &accelrun.z);


	/* Convert winch tilt to radians. */
	abankref *= DEGRAD; aelevref *= DEGRAD; 

	printf("winch ref:  bank: %10.4f  elevatation: %10.4f\n", abankref * RADDEG, aelevref * RADDEG);
	printvectorD("accelerometer calibration",&accelcal);
	printvectorD("accelerometer runtime val",&accelrun);


	/* Handle zero|zero case which if not fixed will give 'nan' when normalizing. */
	if ((abankref == 0) && (aelevref == 0)) {abankref = 1E-6; aelevref = 1E-6;}
	// Probably bogus if 'accelcal' is all zero
	if ((accelcal.x == 0) && (accelcal.y == 0) && (accelcal.z == 0)) {accelcal.z = 1;}

/* 
Kuipers pg 85, 205:  Reference coordiante frame--Aerospace
	x axis +direction points north   
	y axis +direction points east    
	z axis +direction points downward = 
	rotation about z axis angle = psi   = +yaw/heading (skid)
	rotation about y axis angle = theta = +x yields +pitch (nose up)
	rotation about x axis angle = phi   = +y yields +bank (right wing down?)
*/		

	/* ---------- DEH two angle scheme ---------------------------- */
	// The input is measurements of elevation and bank of the winch frame

	printseparator(" DEH two angle scheme\n");
	double alpha = abankref;
	double beta  = aelevref;

	// Computer axis angle and rotation angle from input elevation and bank measurements
	struct AXISANGLEANDPHI aap = axisangle_phi_fromtwoangletiltd(alpha, beta);
	printf("\nKuipers page 62 angle of rotation phi: %9.7f(radians) %12.6f(deg)\n",aap.phi, (aap.phi * RADDEG));
	printf("axis angle: %9.7f(radians) %12.6f(deg)\n",aap.axa, (aap.axa * RADDEG));

	// Convert axis angle and rotation angle to a unit vector
	VD3 vaap;
	vd3_fromaxisandphid(&vaap, aap);
	printvectorD("vaap components x y z",&vaap);

	// Make quaternion from vector and axis angle|rotation angle
	QUATD qdeh;
	qd_fromaxisangleandvector(&qdeh, aap.phi, &vaap);
	double a = aap.phi/2;
	double anormr = 1.0/sqrt((vaap.x * vaap.x) + (vaap.y * vaap.y));
	qdeh.d0 = cos(a);
	qdeh.d1 = sin(a) * vaap.x * anormr;
	qdeh.d2 = sin(a) * vaap.y * anormr; 
	qdeh.d3 = 0;
	printquat("qdeh: ",&qdeh);
	printf("quaternion qdeh magnitude check: %15.12f\n",qd_magnitude(&qdeh));

	// Convert to rotation matrix
	MD33 mddeh4;
	qd_todirectioncosinesd(&mddeh4, &qdeh);
	md33_rotfromquatd(&mddeh4, &qdeh);
	matrixchk("mddeh4-direction cosine matrix", &mddeh4);

	rot_angle = md33_rotationangle(&mddeh4); // Compute rotation angle out of rotation matrix
	printf ("\nrot_angle from rotation matrix mddeh4: %9.7f(radians) %12.6f(deg)\n",rot_angle, (rot_angle * RADDEG));
	
	// Euler angles from original quaternion
	VD3 eulerdeh;
	vd3_euler123fromquatd(&eulerdeh, &qdeh);
	printvectoranglesD("eulerdeh angles:  Diebel", &eulerdeh);

	qd_toeuleranglesd(&eulerdeh, &qdeh);
	printvectoranglesD("eulerdeh angles: Kuipers", &eulerdeh);

	// Make quaternion from euler angles
	QUATD qdeh2;
	eulertoquatd(&qdeh2, &eulerdeh);	
	printquat("qdeh2",&qdeh2);

	// Convert to rotation matrix from quaternion made from Euler angles
	MD33 mddeh3;
	qd_todirectioncosinesd(&mddeh3, &qdeh2);
	matrixchk("mddeh3", &mddeh3);	

//return 0;	
	/* ------------- Do x & y rotations with two quaternions --------------- */
	printseparator(" Do x & y rotations with two quaternions\n");
	// Rotation around Y in xz plane
	QUATD qxz;	// quaternion for rotation around Y
	qxz.d0 = cos(eulerdeh.y/2); // Kuipers p167 {psi, theta, phi} = {0, elev, 0}
	qxz.d1 = -sin(eulerdeh.y/2);
	qxz.d2 = 0;
	qxz.d3 = 0;
	printquat("qxz: ",&qxz);

	// Rotation matrix from quaternion (to see if it looks OK)
	MD33 Axz;
	qd_rotation_matrix(&Axz, &qxz); // Convert quaternion to rotation matrix
	printmd33("Axz",&Axz);
	rot_angle = md33_rotationangle(&Axz); // Compute rotation angle out of rotation matrix
	printf ("\nrot_angle from rotation matrix Axz: %9.7f(radians) %12.6f(deg)\n",rot_angle, (rot_angle * RADDEG));
	vd3_axisofrotation(&vxr, &Axz); printvectorD("Axis of rotation: Axz",&vxr);

	VD3 vxz;
	md33mul31(&vxz, &Axz, &vd001);
	printvectorD("Axz: x y z components\t",&vxz);
	printf("rotation matrix determinant: %15.12f\n", md33det(&Axz));

	// Rotation around X in yz plane
	QUATD qyz;	// quaternion for rotation around X
	qyz.d0 = cos(eulerdeh.x/2);
	qyz.d1 = 0;
	qyz.d2 = -sin(eulerdeh.x/2);
	qyz.d3 = 0;
	printquat("qyz: ",&qyz);

	// Rotation matrix from quaterion (to see if it looks OK)
	MD33 Ayz;
	qd_rotation_matrix(&Ayz, &qyz); // Convert quaternion to rotation matrix
	printmd33("Ayz",&Ayz);
	rot_angle = md33_rotationangle(&Ayz); // Compute rotation angle out of rotation matrix
	printf ("\nrot_angle from rotation matrix Ayz: %9.7f(radians) %12.6f(deg)\n",rot_angle, (rot_angle * RADDEG));
	vd3_axisofrotation(&vxr, &Ayz); printvectorD("Axis of rotation: Ayz",&vxr);
	printf("rotation matrix determinant: %15.12f\n", md33det(&Ayz));

	QUATD qxy;
	qd_mulqp(&qxy, &qxz, &qyz); 	// (Kuipers p 156. Eq 7.1)
	QUATD qxystar;
	qd_conj(&qxystar,&qxy);
	QUATD qxyd;
	qd_mulpq(&qxyd,&qxy,&qxystar);
	printquat("qxy",&qxyd);
	
	// Rotation matrix from xy quaterion
	MD33 Axy;
	qd_rotation_matrix(&Axy, &qxy); // Convert quaternion to rotation matrix
	printmd33("Axy",&Axy);
printf("Trace of Axy: %10.7f\n",Axy.d11+Axy.d22+Axy.d33);
	rot_angle = md33_rotationangle(&Axy); // Compute rotation angle out of rotation matrix
	printf ("\nrot_angle from rotation matrix Axy: %9.7f(radians) %12.6f(deg)\n",rot_angle, (rot_angle * RADDEG));
	vd3_axisofrotation(&vxr, &Axy); printvectorD("Axis of rotation: Axy",&vxr);
	printf("rotation matrix determinant: %15.12f\n", md33det(&Axy));

	/* Compute x y z from rotation matrix. */
	VD3 vxy;
	md33mul31(&vxy, &Axy, &vd001);
	printvectorD("Axy: x y z components\t\t",&vxy);

	// Multiply rotation matrices from foregoing
	md33mulmd33(&Axy, &Axz, &Ayz);
	
	rot_angle = md33_rotationangle(&Axy); // Compute rotation angle out of rotation matrix
	printf("----------- Rotation matrix multiply --------------\n");
	printmd33("Axy",&Axy);
	printf ("rot_angle from rotation matrix Axy: %9.7f(radians) %12.6f(deg)\n",rot_angle, (rot_angle * RADDEG));
	vd3_axisofrotation(&vxr, &Axy); printvectorD("Axis of rotation: Axy",&vxr);

	/* Compute x y z from Axy rotation matrix. */
	md33mul31(&vxy, &Axy, &vd001);
	printvectorD("Axy: x y z components\t\t",&vxy);
//return 0;
	/* --------------------------------------------------------------------------------------------------------- 	*/
	/* Generate quaternions for the winch leveling measurement using Euler angles and test the different		*/
	/*   formulas/subroutines.											*/
	/* ---------------------------------------------------------------------------------------------------------	*/
	printseparator("## Winch leveling quaternion: Euler angle based \n");	// Something to make the printout a little easier to read.

	/* Convert cosine matrix to quaternion */
	MD33 drcos = mddeh4;	// drcos = euler angles from above
	QUATD quatcos;
	cosinetoquat(&quatcos, &drcos);	// Convert Euler to quaternion
	printquat("Quaternion from cosine rotation matrix (Kuipers):",&quatcos);

	/* Euler to quat using formula from Kuipers. */
	QUATD eqK;
	eulertoquatdK(&eqK, &eulerdeh);
	printquat("Quaternion eqK from eulerref (Kuipers):",&eqK);

	/* Euler to quat using formula from other web sources (all which agree). */
	// I believe the difference in forumulas is that Kuipers' is based on a unit quaternion 
	//   the others do not have that restriction (which complicates the computation a bit).
	QUATD eqM;
	eulertoquatd(&eqM, &eulerdeh);
	printquat("Quaternion eqM from eulerref (Others):",&eqM);


	/* Convert quaternion to rotation matrix */
	MD33 rot1; 
	qd_rotation_matrix(&rot1, &eqK);
	printmd33("Rotation matrix rot1 from quaternion (Deibel): ",&rot1);
	double rot_angle1 = md33_rotationangle(&rot1);
	printf ("rot_angle from rotation matrix rot1: %9.7f(radians) %12.6f(deg)\n",rot_angle1, (rot_angle1 * RADDEG));
	
	MD33 rot2;
	qd_rotation_matrixB(&rot2, &eqK);
	printmd33("Rotation matrix rot2 from quaternion (Kuipers?): ",&rot2);
	double rot_angle2 = md33_rotationangle(&rot2);
	printf ("rot_angle from rotation matrix rot2: %9.7f(radians) %12.6f(deg)\n",rot_angle2, (rot_angle2 * RADDEG));
	vd3_axisofrotation(&vxr, &rot2); printvectorD("Axis of rotation: vxr|rot2",&vxr);
	VD3 vchk; VD3 vz = {0,0,1};
	md33mul31(&vchk, &rot1, &vz);	printvectorD(  " rot1 x {0, 0, 1} = x y z:",&vchk);
	md33mul31(&vchk, &rot2, &vz);	printvectorD(  " rot2 x {0, 0, 1} = x y z:",&vchk);
	


	/* --------------------------------------------------------------------------------------------------------- 	*/
	/* Next task is to generate a qauternion for the accelerometer readings that pair with the winch "not-level"	*/
	/*  readings.  Following this the two will be multiplied to generate a rotation matrix.				*/
	/* ---------------------------------------------------------------------------------------------------------	*/
	/* Compute rotation matrix and rotate accelerometer calibration readings to world frame. */
	printseparator(" Accelerometer calibration\n");

	// Normalize the accelerometer vector
	VD3 accelref;
	vd3_normalize (&accelref, &accelcal);
	printvectorD("Accelerometer calibration (raw)        ",&accelcal);
	printvectorD("Accelerometer calibration (normalized) ",&accelref);
	
	// Compute rotation angle
	rot_angle = acos(accelref.z);
	printf("Accel rot_angle: %12.9f (rad) %9.6f (deg)\n",rot_angle,rot_angle*RADDEG);

	// Simplification test: cos(theta/2) = sqrt( 0.5 * (1 + cos(theta) );
	double ctht2 = sqrt(0.5 * (1 + accelref.z));
	printf("cos(theta/2): %12.9f via sqrt: %12.9f\n",cos(rot_angle/2), ctht2);
	// Simplification test: sin(theta/2) = sqrt( 0.5 * (1 - cos(theta) );
	double stht2z = sqrt(0.5 * (1 - accelref.z));
	printf("sin(theta/2): %12.9f via sqrt: %12.9f\n",sin(rot_angle/2), stht2z);

	// Multiply calibration and winch tilt quaternions to get accelerometer-to-winch quaternion.
	QUATD qaw;
	QUATD qdehstar;
	qd_conj(&qdehstar,&qdeh);

	double Ax = accelref.x; 
	double Ay = accelref.y;
	double Anorm = 1.0/sqrt(Ax * Ax + Ay * Ay);
	Ax *= Anorm;
	Ay *= Anorm;
	rot_angle = acos(accelref.z);
	printf ("rot_angle: %9.7f(radians) %12.6f(deg)\n",rot_angle, (rot_angle * RADDEG));
	QUATD qaccel2;
	qaccel2.d0 = cos(rot_angle/2);
	qaccel2.d1 = Ax * stht2z;
	qaccel2.d2 = Ay * stht2z;
	qaccel2.d3 = 0;
	printquatsummary("Quaternion qaccel2",&qaccel2);

	qd_mulpq(&qaw,&qdeh,&qaccel2);
	printquatsummary("Quaternion qaw",&qaw);

	// Rotation matrix for calibration
	MD33 mdrot;
	qd_todirectioncosinesd(&mdrot, &qaw);
	matrixchk("mdrot", &mdrot);		

	/* --------------------------------------------------------------------------------------------------------- 	*/
	/* Compute the tilt for a "run" set of readings using the foregoing calibration.				*/
	/* ---------------------------------------------------------------------------------------------------------	*/
	/* Compute rotation matrix and rotate accelerometer calibration readings to world frame. */
	printseparator(" Winch Tilt from accelerometer readings using foregoing calibration\n");

		// Probably bogus if 'accelrun' is all zero
	if ((accelrun.x == 0) && (accelrun.y == 0) && (accelrun.z == 0)) {accelrun.z = 1;}

	// Normalize the accelerometer vector
	VD3 accelnrm;
	vd3_normalize (&accelnrm, &accelrun);	// Normalize accelerometer readings
	printvectorD("Accelerometer calibration      (raw)  ",&accelrun); // Before normalization
	printvectorD("Accelerometer calibration (normalized)",&accelnrm); // After

	/* Multiply the calibration rotation matrix times the accelerometer readings. */
	VD3 vdtilt;
	md33mul31(&vdtilt, &mdrot, &accelnrm); 		// Rotate accelerometer vector to winch frame
	printvectorD("Calibrated reading                    ",&vdtilt);
	printf("X axis angle: %12.9f (rad) %12.9f (deg)\n", asin(vdtilt.x), asin(vdtilt.x) * RADDEG); // Display tilt
	printf("Y axis angle: %12.9f (rad) %12.9f (deg)\n", asin(vdtilt.y), asin(vdtilt.y) * RADDEG); // Display tilt
	printf("Z axis angle: %12.9f (rad) %12.9f (deg)\n", asin(vdtilt.z), acos(vdtilt.z) * RADDEG); // Display tilt

	/* Convert accelerometer readings to quaternion (cell phone article procedure). */
	QUATD qnrm;
	qnrm.d0 = sqrt(0.5 * (1 + accelnrm.z)); 		// cosine( rotation_angle/2);
	double sinnrm = sqrt(0.5 * (1 - accelnrm.z)); 		// Use for normalizing x & y
	double Anrm = 1.0/sqrt((accelnrm.x * accelnrm.x) + (accelnrm.y * accelnrm.y));
//qnrm.d0 = cos(acos(accelnrm.z)/2);
//sinnrm = sin(acos(accelnrm.z)/2);
	qnrm.d1 = -Anrm * accelnrm.x * sinnrm;
	qnrm.d2 = -Anrm * accelnrm.y * sinnrm;
	qnrm.d3 = 0;	// Gravity points straight down
	printquatsummary("Quaternion qnrm",&qnrm); 

	/* Do the rotation using quarternions. */
	QUATD qra;
	QUATD qrot = qaw;
	printquatsummary("quaternion qrot",&qrot);

	QUATD qrotstar;
	qd_conj(&qrotstar,&qrot);	// Conjugate rotation quaternion
	// Make a pure quaternion from normalized accelerometer readings
	QUATD qv; qv.d0 = 0; qv.d1 = accelnrm.x; qv.d2 = accelnrm.y; qv.d3 = accelnrm.z;
	// Rotation quaternion times accelerometer vector
	qd_mulpq(&qra,&qrot,&qv);	
	QUATD qrb;
	qd_mulpq(&qrb,&qra,&qrotstar);	// Result of above times conjugate rotation
	printquatsummary("Vector rotated via quaternion",&qrb);
	double acosz = 2 * acos(qrb.d0);
	printf("Z axis angle: %20.15f (rad) %20.15f (deg)\n", acosz, acosz * RADDEG); // Display tilt
	VD3 tdir;
	double cosecant = 1.0/sin(acosz/2);
	tdir.x = qrb.d1 * cosecant;
	tdir.y = qrb.d2 * cosecant;
	tdir.z = qrb.d3 * cosecant;
	printvectorD("      axis angle vector for each axis",&tdir);

	VD3 tdeg;
	tdeg.x = asin(tdir.x) * RADDEG;
	tdeg.y = asin(tdir.y) * RADDEG;
	tdeg.z = acos(tdir.z) * RADDEG;
	printvectorD("axis angle vector for each axis (deg)", &tdeg);
	
	printf("\n");
	return 0;
}
/* ***************************************************************************************** 
 * static void printvectorD(char *p, VD3 *v);
 * @brief	: Print vector (Double precision)
 * @param	: p = pointer to text
 * @param	: v = pointer to vector
 *******************************************************************************************/
static void printvectorD(char *p, VD3 *v)
{
	printf("%s\t",p);
	printf("%20.15f %20.15f %20.15f  magnitude: %18.16f\n",v->x, v->y, v->z,vd3_magnitude(v));
	return;
}
/* ***************************************************************************************** 
 * static void printvectorS(char *p, VS3 *v);
 * @brief	: Print vector (Single precision)
 * @param	: p = pointer to text
 * @param	: v = pointer to vector
 *******************************************************************************************/
 void printvectorS(char *p, VS3 *v)
{
	printf("%s\t",p);
	printf("%10.4f %10.4f %10.4f\n",v->x, v->y, v->z);
	return;
}
/* ***************************************************************************************** 
 * static void printquat(char *p, QUATD *q);
 * @brief	: Print vector
 * @param	: p = pointer to text
 * @param	: q = pointer to quaternion
 *******************************************************************************************/
static void printquat(char *p, QUATD *q)
{
	printf("\n%s",p);
	printf("\n\tq0: %20.15f\n\tq1: %20.15f\n\tq2: %20.15f\n\tq3: %20.15f\n", q->d0, q->d1, q->d2, q->d3);
	return;
}
/* ***************************************************************************************** 
 * static void printms33(char *p, MS33 *m);
 * @brief	: Print vector
 * @param	: p = pointer to text
 * @param	: m = pointer to 3x3 float matrix
 *******************************************************************************************/
 void printms33(char *p, MS33 *m)
{
	printf("\n%s",p);
	printf("\n\t         1             2             3\n");
	printf("\t1 %13.8f %13.8f %13.8f\n", m->f11, m->f12, m->f13);
	printf("\t2 %13.8f %13.8f %13.8f\n", m->f21, m->f22, m->f23);
	printf("\t3 %13.8f %13.8f %13.8f\n", m->f31, m->f32, m->f33);
	return;
}
/* ***************************************************************************************** 
 * static void printmd33(char *p, MD33 *m);
 * @brief	: Print vector
 * @param	: p = pointer to text
 * @param	: m = pointer to 3x3 float matrix
 *******************************************************************************************/
static void printmd33(char *p, MD33 *m)
{
	printf("\n%s",p);
	printf("\n\t                1                    2                    3\n");
	printf("\t1 %20.15f %20.15f %20.15f\n", m->d11, m->d12, m->d13);
	printf("\t2 %20.15f %20.15f %20.15f\n", m->d21, m->d22, m->d23);
	printf("\t3 %20.15f %20.15f %20.15f\n", m->d31, m->d32, m->d33);
	return;
}
/* ***************************************************************************************** 
 * static void printdashline(int n);
 * @brief	: Print 'n' lines of dashes
 * @param	: n = number of lines
 *******************************************************************************************/
static void printdashline(int n)
{
	int i,j;
	for (j = 0; j < n; j++)
	{
		for (i = 0; i < 100; i++) printf("-"); 
		printf("\n");
	}
	return;
}
/* ***************************************************************************************** 
 * static void printseparator(char *p);
 * @brief	: Print a nice separator
 * @param	: p = pointer to text
 *******************************************************************************************/
static void printseparator(char *p)
{
	printdashline(1);
	printf("%s",p);
	printdashline(1);
	return;
}
/* ***************************************************************************************** 
 * static void printvectoranglesD(char *p, VD3 *vi);
 * @brief	: Print vector of angles in radians and degrees (double precision)
 * @param	: p = pointer to text
 * @param	: vi = pointer to vector with angles in radians
 *******************************************************************************************/
static void printvectoranglesD(char *p, VD3 *vi)
{
	printf("%s\n",p);
	printf("  x %12.9f (rad) %12.9f (deg)\n",vi->x, vi->x * RADDEG);
	printf("  y %12.9f (rad) %12.9f (deg)\n",vi->y, vi->y * RADDEG);
	printf("  z %12.9f (rad) %12.9f (deg)\n",vi->z, vi->z * RADDEG);
	return;
}
/* ***************************************************************************************** 
 * static void matrixchk(char *p, MD33 *m);
 * @brief	: Check rotation matrix
 * @param	: m = pointer to matrix
 *******************************************************************************************/
static void matrixchk(char *p, MD33 *m)
{
	MD33 ttp;
	printf("Input--");
	printmd33(p,m);
	printf("matrix determinant input: %15.12f\n", md33det(m));
	md33transpose(&ttp, m);
	printf("Transpose-- ");
	printmd33(p,&ttp);
	printf("matrix determinant trans: %15.12f\n", md33det(&ttp));

//	MD33 ta;
//	md33mulmd33(&ta,m,&ttp);
//	printmd33("Identity--",&ta);
//	printf("matrix determinant multp: %15.12f\n", md33det(&ta));
	return;
}
/* ***************************************************************************************** 
 * static void printquatsummary(char *p, QUATD *q);
 * @brief	: Summary of quaternion and checking thereof
 * @param	: p = pointer to display
 * @param	: q = pointer to quaternion
 *******************************************************************************************/
static void printquatsummary(char *p, QUATD *q)
{
	double rot;
	printquat(p,q);
	rot = 2 * acos(q->d0);
	printf ("rot_angle: %9.7f(radians) %12.6f(deg)\n",rot, (rot * RADDEG));
	printf("quaternion magnitude check: %15.12f\n",qd_magnitude(q));
	return;
}
