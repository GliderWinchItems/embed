/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : ratechage.c
* Creater            : gsm
* Date First Issued  : 07/21/2012
* Board              : Linux PC
* Description        : Convert 5 Hz gps lines contained the output of reformat.c to 64 Hz
*******************************************************************************/
/*

// Command line example when the input is via stdin
To compile--
gcc ratechange.c -o ratechange -lm 

To execute (output to console)--
./ratechange < ../dateselect/120721.195850RS

To execute (output to file)
./ratechange < ../dateselect/120721.195850RS > file.txt

To execute (output to console and file)
./ratechange < ../dateselect/120721.195850RS | tee > file.txt

To compile and execute--
gcc ratechange.c -o ratechange -lm && ./ratechange < ../dateselect/120721.195850RS

*/

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>


/* Values extracted from ascii in GPS line */
struct GPSVALUES
{
	unsigned long long ullTime;
	double d[6];
// 0 Latitude (degrees)
// 1 Longitude (degrees)
// 2 Height (m above MSL)
// 3 Velocity East (m/s)
// 4 Velocity North (m/s)
// 5 Velocity Up (m/s)
};
#define PODTIMEEPOCH	1318478400	// Offset from Linux epoch to save bits
unsigned long long ullEpoch = PODTIMEEPOCH;

/* Subroutine prototypes */
void extract_values(struct GPSVALUES *g, char * p);



/* Line buffer size */
#define LINESIZE 2048
char buf[LINESIZE];

char infile[128];	// Input file name buffer

/* ************************************************************************************************************ */
/*  Yes, this is where it starts.                                                                               */
/* ************************************************************************************************************ */
int main(int argc, char **argv)
{
	int i,j;
	char cC;
	struct GPSVALUES strV;


/* ************************************************************************************************************ */
/* Read in lines formatted with 'reformat.c' piped to 'sort' and piped to this routine                          */
/* ************************************************************************************************************ */
	while ( (fgets (&buf[0],LINESIZE,stdin)) != NULL)	// Get a line from stdin
	{
		j = strlen(buf);	// j = length of line
		cC = buf[j-2];		// cC = packet ID
		if (j < 35) continue;
//printf ("3%i# %s",j,buf);	// Debugging

		switch (cC)
		{
		case '1': // Tension data line
//			printf ("%s",buf);	// Pass it to the output
			break;
		case '5': // Accelerometer data line
//			printf ("%s",buf);	// Pass it to the output
			break;
		case '6': // GPS data

//printf ("%s",buf);		//  Debugging
			extract_values(&strV, buf);	// Extract values from ascii

// Debugging: see the numbers in the struct
// ColumnsL Extended time count, Lat, Long, Height, Velocity E, N, U
printf("%10llu %10.5F%10.5F%7.2F%8.2F%8.2F%8.2F",strV.ullTime,strV.d[0],strV.d[1],strV.d[2],strV.d[3],strV.d[4],strV.d[5]);
// Add readabke date/time
unsigned int Debug1 = ((strV.ullTime >> 6) + PODTIMEEPOCH);
unsigned int Debug2 = (strV.ullTime & 31);// Isolate 1/64th tick
struct tm *ptm;
ptm = gmtime( (time_t*) &Debug1 );// ascii'ize it
// Columns: 64th tick, human time
printf(" %3u %s", Debug2, asctime (ptm));

			break;
		default:
			break;
		}
	}
//	printf ("Done\n");
	return 0;
}
/* ************************************************************************************************************ 
 * void extract_values(struct GPSVALUES *g, char * p);                                                   
 * brief: Convert ascii values in gps input line to doubles in a struc along with Linux time * 64
 * parm: int j--Length of line (for error checking)
 * parm: char* p--pointer to input line 
 * parm: struct GPSVALUES *g: pointer to struct to be filled
/* ************************************************************************************************************ */
void extract_values(struct GPSVALUES *g, char * p)
{	
	int iX;		// Temp
	double dX;	// Temp

/* NOTE:
'printf' statements are for debugging.  Comment out.
*/

	/* Convert time  */
	sscanf(p,"%llu",&g->ullTime);
//printf("Time: %llu\n",g->ullTime);

	/* Latitude (degrees) (N is positive, S is negative) */
//printf("Lat: ");
	sscanf((p+11),"%2u",&iX);
	g->d[0] = iX;
	sscanf((p+14),"%lf",&dX);
//printf("  %4u %8.5F ",iX,dX);
	g->d[0] += (dX/60.0);
	if (*(p+22) == 'S') g->d[0] = -g->d[0];
//printf (" decimal: %10.5F\n",g->d[0]);

	/* Longitude (degrees) (E is negative, W is positive) */
//printf("Long: ");
	sscanf((p+24),"%3u",&iX);
	g->d[1] = iX;
	sscanf((p+27),"%lf",&dX);
//printf(" %4u %8.5F",iX,dX);
	g->d[1] += (dX/60.0);
	if (*(p+36) == 'E') g->d[1] = -g->d[1];
//printf ("  decimal: %10.5F\n",g->d[1]);

	/* Height (m above MSL) */
	sscanf((p+42),"%lf",&dX);
	g->d[2] =  dX;
//printf ("Ht:  %12.5F\n",g->d[2]);

	/* Velocity East (signed) */
	sscanf((p+71),"%lf",&dX);
	g->d[3] =  dX;
//printf ("VelE: %8.2F\n",g->d[3]);
	
	/* Velocity North (signed) */
	sscanf((p+79),"%lf",&dX);
	g->d[4] =  dX;
//printf ("VelN: %8.2F\n",g->d[4]);

	/* Velocity Up (signed) */
	sscanf((p+87),"%lf",&dX);
	g->d[5] =  dX;
//printf ("VelU: %8.2F\n",g->d[5]);


//printf("\n");	
	return;
}

