/* File: float_timegpsaccel.c -- clone and hack of float_time.c for gps data
 * deh 09-18-2012
 * deh 11--5-2012 Hack of float_timegpsaccel.c to add accelerometer provision

Note: the 'RM file is the normal input to this routine.  See the scripts
../svn_pod/sw_PC/trunk/dateselect/mmag_xxxxx
 
 */ 
/*
Compile--
gcc float_timegpsaccel.c -o float_timegpsaccel -lm
Compile & test--
gcc float_timegpsaccel.c -o float_timegpsaccel -lm && mmag_plotx 121104/121104.234957

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

/* Constants of the sphere */
#define EARTHRADIUS   (1000.0*(12756.32/2.0))	// Radius in (meters)
#define RADIANCONVERT (3.14159265358979323/180.0)

/* 09/15/2012 winch lat/long/height) and TwinAstir mass, defaults */
double winchlat = 34.920623077;
double winchlong = 81.950259487;
double winchheight = 233;
double glidermass = 555;


/* For determining glider mass during ground roll */
// Sum tension during the ground roll, and note speed increase during the interval
#define TIMEGNDROLLSTART	9.54	// Start time (sec) of summing tension
#define TIMEGNDROLLSTOP		18	// End time (sec) of summing tension


#define	MYBUFSZ	1000

#define DELAYLINESIZE	64	// Max number of lines of delay for adjusting Tensin v GPS data
int tension[DELAYLINESIZE];	// Tension (grams)
int nDelayIdx;
int nDelaySw;

int nMassCt;
double MassSum;

main(int argc, char *argv[])
{
		char mybuf[MYBUFSZ], *p = mybuf;
		
		/* Measure'ables */
		unsigned long long int time;	// Linux time (in 1/64 ticks) of 1st record
		double dtime,t0;	
		double rtime;			// Time (secs) relative to 1st record
		double latitude,  lat0;		// Latitude (degrees),  and of 1st record. 
		double longitude, lon0;		// Longitude (degrees), and of 1st record
		double height, 	h0;		// Height (meters) of 1st record, and of 1st record
		double velocity_Vert,  vV0;	// Vertical velocity (meters/second), and of 1st record 
		double velocity_North, vN0;	// Horizontal velocity North (meters/second), and of 1st record 
		double velocity_East,  vE0;	// Horizontal velocity East  (meters/second), and of 1st record 
		double accel_X;			// Acceleration X
		double accel_Y;			// Acceleration Y
		double accel_Z;			// Acceleration Z
		double accel_V;			// Acceleration vector magnitude

		/* Compute'ables */
		double horizontal_velocity;	// Computed horizontal velocity vector magnitude
		double vector_velocity;		// square-root of sum of velocities
		double line_length;		// Computed from lat/long/height versus winch
		double dX1,dX2,dX3,hor_length;
		double cable_length;
		double cable_length_old;
		double cable_speed;
		double power_cable;		// Power = line speed * tension
		double vector_velocity_old;	
		double power_total;
		double pwr_potential;		// Power = Potential eneryg/sec + change in kinetic energy/sec
		double pwr_kinetic;
		double cable_accel;
		double cable_speed_old;
		double glider_mass;
		double horizontal_velocity_old;

	int linelength;

	if(argc != 2)
	{
		fprintf(stderr, "usage: $ float_timegpsaccel linelength < <infile> > <outfile>\n");
		exit(EXIT_FAILURE);
	}
	sscanf (argv[1],"%u",&linelength);

	while(1==1)
	{

		/* Logic'ables */
		static int first_time_flag = (1==1);
		
		p = fgets(mybuf, MYBUFSZ, stdin);// Input line (piped in)
		if(p == NULL) break;		// We're done.
		
		if(strlen(p) != linelength) 
		{
//printf("linelength %d %d\n",strlen(p), linelength);
			continue; // Skip short lines (summary, junk, etc.)
		}

		
		/* Convert numbers that will be used in graph */
		sscanf(&mybuf[  0],"%10llu",&time);
		sscanf(&mybuf[ 10],"%d" ,&tension[nDelayIdx]);
		sscanf(&mybuf[ 19],"%lf",&latitude);
		sscanf(&mybuf[ 32],"%lf",&longitude);
		sscanf(&mybuf[ 46],"%lf",&height);
		sscanf(&mybuf[ 55],"%lf",&velocity_North);
		sscanf(&mybuf[ 65],"%lf",&velocity_East);
		sscanf(&mybuf[ 73],"%lf",&velocity_Vert);
		sscanf(&mybuf[ 82],"%lf",&accel_X);
		sscanf(&mybuf[ 91],"%lf",&accel_Y);
		sscanf(&mybuf[ 99],"%lf",&accel_Z);
		sscanf(&mybuf[107],"%lf",&accel_V);

// Debug
//printf("%11.6f %10d %10.6F %10.6F %8.2F %8.2F %8.2F %8.2F %8.3F %8.3F %8.3F %8.3F\n", rtime, tension[nDelayIdx],latitude,longitude,height,velocity_North,velocity_East,velocity_Vert,accel_X,accel_Y,accel_Z,accel_V);

		
		dtime =  time;

		latitude  *= RADIANCONVERT;	// Convert to radians 
		longitude *= RADIANCONVERT;

		/* Tension is delayed with respect to the gps data */
		nDelayIdx += 1;
		if (nDelayIdx >= DELAYLINESIZE)
		{
			nDelayIdx = 0;  // Wrap the delay line index around
			nDelaySw = 1; 	// Skip output until one cycle of the delay line
		}

		/* Computer values derived from gps readings */
		horizontal_velocity = sqrt(velocity_North*velocity_North + velocity_East*velocity_East);
		vector_velocity = sqrt(horizontal_velocity*horizontal_velocity + velocity_Vert*velocity_Vert);

		/* Compute cable length */
		dX1 = latitude - (winchlat*RADIANCONVERT);	// Difference of POD and winch: latitude
		dX2 = cos(latitude) * (longitude - (winchlong*RADIANCONVERT));	// Difference of POD and winch: longitude
		dX3 = height - winchheight;	// Difference of POD and winch: height
		hor_length = EARTHRADIUS  * sqrt(dX1*dX1 + dX2*dX2); // Horizontal distance to winch (meters)
		cable_length = sqrt(hor_length*hor_length + dX3*dX3);	// Cable length

		/* Cable speed and power_cable = tension * cable speed */
		cable_speed = (cable_length_old - cable_length)*64.; // (meters/sec x10)
		cable_length_old = cable_length;	
		power_cable = cable_speed * tension[nDelayIdx] * 1E-3 * 9.8066 * 1E-3 * 10; // (g m/s) -> (kg m/s) -> (watt) -> (Kw) -> (scaled up for plot)
		
		/* pwr_potential = potential energy/sec + kinetic energy change /sec */
		pwr_potential = (glidermass * velocity_Vert) * 9.8066 * 1E-3 * 10; // Potential (Kw scaled for plot)
		if (pwr_potential < 0) pwr_potential = 0.0; // Helps plot after release

		pwr_kinetic = ((glidermass * 0.5) * ((vector_velocity*vector_velocity) - (vector_velocity_old*vector_velocity_old)))*64.;
		pwr_kinetic *= 1E-3 * 10;	// Convert to Kw * 10
		if (pwr_kinetic < -400) pwr_kinetic = -400.0; // Helps plot after release
		vector_velocity_old = vector_velocity;	// Update old for taking difference next time

		power_total = pwr_potential + pwr_kinetic; // Total power = Potential plus kinetic


					
		/* We save the first readings as the datum for time, and height */
		if(first_time_flag) 
		{
			t0 = dtime;
			lat0 = latitude;
			lon0 = longitude; 
			h0 = height;
			vV0 = velocity_Vert;
			vN0 = velocity_North;
			vE0 = velocity_East;
			cable_speed_old = cable_speed;

			first_time_flag = (1==0);
		}

		rtime =	(dtime-t0)/64.0;	// Relative time 


		if (nDelaySw != 0)	// Skip output until tension delay buffer full
		{
			/* Apparent mass of glider from cable acceleration and tension */
			if ((rtime > TIMEGNDROLLSTART) && (rtime < TIMEGNDROLLSTOP))
			{
				if ((horizontal_velocity - horizontal_velocity_old) != 0)
				{
					glider_mass = (.001 * 2 * 9.80665 * (1.0/64.0)) * (tension[nDelayIdx] / (horizontal_velocity - horizontal_velocity_old) );
					horizontal_velocity_old = horizontal_velocity;
				}
					MassSum += glider_mass;
					nMassCt += 1;
			}

// Debug
printf("%11.6f %10d %10.6F %10.6F %8.2F %8.2F %8.2F %8.2F %8.3F %8.3F %8.3F %8.3F\n", rtime, tension[nDelayIdx],latitude,longitude,height,velocity_North,velocity_East,velocity_Vert,accel_X,accel_Y,accel_Z,accel_V);

//		printf("%11.6f %10d %10.2F %10.2F %10.2F %10.2F %10.2F %10.2F %10.2F %10.2F %10.2F %10.2F %10.2F %10.2F\n", rtime, tension[nDelayIdx], (height-h0), 10*(velocity_Vert-vV0),10*horizontal_velocity, 10*vector_velocity, hor_length, cable_length, cable_speed, power_cable, pwr_potential, pwr_kinetic, power_total,glider_mass);
		}

	}
//		printf("%11.6f %10d %10.2F %10.2F %10.2F %10.2F %10.2F %10.2F %10.2F %10.2F %10.2F %10.2F %10.2F %10.2F %10.2F\n", rtime, tension[nDelayIdx], (height-h0), 10*(velocity_Vert-vV0),10*horizontal_velocity, 10*vector_velocity, hor_length, cable_length, cable_speed, power_cable, pwr_potential, pwr_kinetic, power_total,glider_mass, MassSum/nMassCt);

}
