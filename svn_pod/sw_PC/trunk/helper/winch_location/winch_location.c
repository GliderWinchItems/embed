/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : winch_location.c
* Creater            : deh
* Date First Issued  : 12/14/2012
* Board              : Linux PC
* Description        : 
*******************************************************************************/
/*
Hack of separator_tension.c

// 12/12/2012 example--
gcc winch_location.c -o winch_location && ./winch_location  < ~/winch/download/121208/121208.174500R
STDIN = reformatted & sorted input file

Some alternate examples--
gcc winch_location.c -o winch_location && ./winch_location  < y

*/

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

#include <stdlib.h>
#include <ctype.h>

#define PODTIMEEPOCH	1318478400	// Offset from Linux epoch to save bits

static int compare_int(const void *a, const void *b);

/* Line buffer size */
#define LINESIZE 256

char buf[LINESIZE];
int nlinect;
/* ************************************************************************************************************ */
/*  Yes, this is where it starts.                                                                               */
/* ************************************************************************************************************ */
int main(int argc, char **argv)
{
	unsigned long long ull;
	double dLat,dLon;
	char * p;
	int iX;
	double dX;
	double dLatmax = 0;
	double dLonmax = 0;
	double dLatmin = 1E10;
	double dLonmin = 1E10;
	
	double dLatd,dLond;
	double dLatdmax = 0;
	double dLondmax = 0;
	double dLatdmin = 1E10;
	double dLondmin = 1E10;
/* ************************************************************************************************************ */
/*                            */
/* ************************************************************************************************************ */
	while ( (fgets (buf,LINESIZE,stdin)) != NULL)	// Get a line from stdin
	{

		nlinect += 1;	// Simple file line counter
	
	p = buf;

	/* Lift the code from 'ratechangecic.c' */

	/* Latitude (degrees) (we won't screw around with NS) */
	sscanf((p+11),"%2u",&iX);
	dLat = iX;
	sscanf((p+14),"%lf",&dX);
	dLatd = dX;
	dLat += (dX/60.0);

	/* Longitude (degrees) (we won't screw around with EW) */
	sscanf((p+24),"%3u",&iX);
	dLon = iX;
	sscanf((p+27),"%lf",&dX);
	dLond = dX;
	dLon += (dX/60.0);

//printf("%f %f\n",dLat,dLon);
	if (dLat > dLatmax) dLatmax = dLat;
	if (dLon > dLonmax) dLonmax = dLon;
	if (dLat < dLatmin) dLatmin = dLat;
	if (dLon < dLonmin) dLonmin = dLon;

	if (dLatd > dLatdmax) dLatdmax = dLatd;
	if (dLond > dLondmax) dLondmax = dLond;
	if (dLatd < dLatdmin) dLatdmin = dLatd;
	if (dLond < dLondmin) dLondmin = dLond;

	}
	
	printf ("\nLat(secs)  Long(secs)\n");
	printf ("Max %10.8lf %10.8lf\n", dLatdmax,dLondmax);
	printf ("Min %10.8lf %10.8lf\n", dLatdmin,dLondmin);

	printf ("\nLat(deg)   Long(deg)\n");
	printf ("Max %10.8lf %10.8lf\n", dLatmax,dLonmax);
	printf ("Min %10.8lf %10.8lf\n", dLatmin,dLonmin);

	return 0;
}
