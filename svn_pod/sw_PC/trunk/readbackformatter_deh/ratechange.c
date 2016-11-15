/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : ratechage.c
* Creater            : deh
* Date First Issued  : 07/21/2012
* Board              : Linux PC
* Description        : Convert 5 Hz gps lines contained the output of reformat.c to 64 Hz
*******************************************************************************/
/*

// Command line example when the input is via stdin
To compile--
gcc FirLp2.c ratechange.c -o ratechange -lm 

To execute (output to console)--
./ratechange < ../dateselect/120721.195850RS

To execute (output to file)
./ratechange < ../dateselect/120721.195850RS > file.txt

To execute (output to console and file)
./ratechange < ../dateselect/120721.195850RS | tee > file.txt

To compile and execute--
gcc FirLp2.c ratechange.c -o ratechange -lm && ./ratechange < ../dateselect/120721.195850RS

*/

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include "FirLp2.h"

#define INPUTRATE	5		// Input data rate 5 Hz
#define OUTPUTRATE	64		// Output data rate 64 Hz
#define UPRATE	(INPUTRATE * OUTPUTRATE)	// Upsampling rate
#define LPCOEFFICIENTSIZE	320	// Number of LP filter taps
#define LPCUTOFF	(0.015625*.5)	// LP filter cutoff
#define GPSBUFSIZE	(LPCOEFFICIENTSIZE/OUTPUTRATE)	// Size of input buffer


/* Commutating filters for rate change */
double dLp[OUTPUTRATE][GPSBUFSIZE];
int coeffarray_idx(int nSize, int nIdx);


/* Values extracted from ascii in GPS line */
#define GPSVALSIZE	6	// Number of GPS values
// 0 Latitude (degrees)
// 1 Longitude (degrees)
// 2 Height (m above MSL)
// 3 Velocity East (m/s)
// 4 Velocity North (m/s)
// 5 Velocity Up (m/s)

/* GPS input data buffer */
struct GPSVALUES
{
	unsigned long long ulltime;
	double d[GPSVALSIZE];
}strV[GPSBUFSIZE];	// GPS input data buffer

int nInIdx;	// Index into buffer
int nSw1;	// One time switch	

/* LP filter for rate changing gps data */
struct FIRLPCO2 firlp2co2;	// Hold parameters and pointer for coefficient table

/* Line buffer size */
#define LINESIZE 2048
char buf[LINESIZE];

char infile[128];	// Input file name buffer

/* Tinkering with removing DC biases */
double dAveX[GPSVALSIZE];

/* Subroutine prototypes */
void ratechangeall(int nInIdx);
void extract_values(struct GPSVALUES *g, char * p, int j);

/* ************************************************************************************************************ */
/*  Yes, this is where it starts.                                                                               */
/* ************************************************************************************************************ */
int main(int argc, char **argv)
{
	int i,j,k,l;
	int nX;
	char cC;

/* ************************************************************************************************************ */
/* LP filter setup:												*/
/* ************************************************************************************************************ */
	// Allocate memory and Compute coefficients. Complete setting up firlpco2 
	FirLpInit(&firlp2co2, LPCOEFFICIENTSIZE, LPCUTOFF, RAISEDCOSINE);

	// Build commutating filter coefficients, e.g. 64 filters of length equal to input samples filtered */
	nX = 0;
	for (i = 0; i <  OUTPUTRATE; i++)
	{
		k = nX;
		for (j = 0; j < GPSBUFSIZE; j++)
		{
//double dLp[OUTPUTRATE][GPSBUFSIZE];
			dLp[i][j] = firlp2co2.dpHBase[ coeffarray_idx(LPCOEFFICIENTSIZE, k) ];
			k += OUTPUTRATE;	
		}
		// Advance LP filter coefficients to next filter
		nX = nX + (OUTPUTRATE - INPUTRATE);
		if (nX >= OUTPUTRATE) nX -= OUTPUTRATE;
	}

	nInIdx = 0;  	// Do we need this?
	nSw1 = 0;	// One-time switch



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

			printf ("%s",buf);	// Pass it to the output
			extract_values(&strV[nInIdx], buf, j);	// Extract values from ascii

			// Remove "dc" offset
			for (k = 0; k < GPSVALSIZE; k++)
				strV[nInIdx].d[k] -= dAveX[k];


			/* The first time fill the entire input buffer with the first set of values */
			if (nSw1 == 0) // Is switch set?
			{ // Here, no.
				nSw1 = 1;	// Once only
				for ( k = 1; k < GPSBUFSIZE; k++)
				{ // Copy struct of first reading to entire buffer of structs
					for ( l = 0; l < GPSVALSIZE; l++)
					{
						strV[k] = strV[0];	// Fill buffer with 1st values
						dAveX[l] = strV[0].d[l];	// Use 1st value to remove large "dc" offset
					}
				}
			}

			/* For new input (5 Hz), convert rate and output (64 Hz) */
			ratechangeall(nInIdx);

			/* Advance input buffer index to input readings */
			nInIdx += 1;  if (nInIdx >= GPSBUFSIZE) nInIdx = 0;

			break;

		default:
			break;
		}
	}

printf ("Done\n");

	return 0;
}
/* *************************************************************************************************************
 * void extract_values(struct GPSVALUES *g, char * p, int j)
 * @brief	: Extract and convert the ascii fields to binary into struct
 * @param	: struct GPSVALUES *g = Points to struct that holds time and values
 * @param	: char *p = points into ascii line
 * @param	: int j = length of ascii line
************************************************************************************************************* */
#define READINGSBUFSIZE	5
#define GPSLINEBUFSIZE	128
static char gpsin[READINGSBUFSIZE*2][GPSLINEBUFSIZE];
static int iInput;
static int iCoef;

void extract_values(struct GPSVALUES *g, char * p, int j)
{	
	int iX;		// Temp
	double dX;	// Temp
	unsigned long long ull64;

	if (j >= GPSLINEBUFSIZE)
	{
		printf ("####ERROR: gps line size = %u: %s\n",j,buf);
	}

	/* Tick time */
	sscanf(buf,"%10llu",&ull64);
	g->ulltime = ull64;

	/* Latitude (degrees) (we won't screw around with NS) */
printf("Lat: ");
	sscanf((p+11),"%2u",&iX);
	g->d[0] = iX;
	sscanf((p+14),"%lf",&dX);
printf("  %4u %8.5F ",iX,dX);
	g->d[0] += (dX/60.0);
printf (" decimal: %12.6F\n",g->d[0]);

	/* Longitude (degrees) (we won't screw around with EW) */
printf("Long: ");
	sscanf((p+24),"%3u",&iX);
	g->d[1] = iX;
	sscanf((p+27),"%lf",&dX);
printf(" %4u %8.5F",iX,dX);
	g->d[1] += (dX/60.0);
printf ("  decimal: %12.6F\n",g->d[1]);

	/* Height (m above MSL) */
	sscanf((p+42),"%lf",&dX);
	g->d[2] =  dX;
printf ("Ht:  %12.5F\n",g->d[2]);

	/* Velocities (m/s) */
	
	/* Velocity East (signed) */
	sscanf((p+71),"%lf",&dX);
	g->d[3] =  dX;
printf ("VelE: %8.2F\n",g->d[3]);
	
	/* Velocity North (signed) */
	sscanf((p+79),"%lf",&dX);
	g->d[4] =  dX;
printf ("VelN: %8.2F\n",g->d[4]);

	/* Velocity Up (signed) */
	sscanf((p+87),"%lf",&dX);
	g->d[5] =  dX;
printf ("VelU: %8.2F\n",g->d[5]);


//printf("\n");	
	return;
}
/* *************************************************************************************************************
 * int coeffarray_idx(int nSize, int nIdx);
 * @brief	: Convert index into ficticous array of coefficients to index into folded, symmetrical array of coefficients
 * @param	: nSize--number of coefficients
 * @param	: nIdx--coefficient to convert
 * @return	: index into the folded array of coefficients
************************************************************************************************************* */
int coeffarray_idx(int nSize, int nIdx)
{
	if ((nSize & 0x01) == 0)
	{ // Here even number of coefficiencts
		if (nIdx >= (nSize >> 1))
		{
			return (nSize -1 - nIdx);
		}
		else
		{
			return nIdx;
		}
	}
	// Here odd number of coefficients
	if (nIdx <= ( (nSize >> 1) +1))
	{
		return nIdx;
	}		
	return (nSize -1 - nIdx);
}
/* *************************************************************************************************************
 * void ratechangeall(int nInIdx);
 * @brief	: Given a set of data for the new input (5 Hz) compute and output the new outputs (64 Hz)
 * @param	: Index of new values into GPSVALUES buffer
************************************************************************************************************* */
void ratechangeall(int nInIdx)
{
	int i,j,k,l;
	int nIdx = nInIdx;		// Working index into buffer with new set of values
	double dSum[GPSVALSIZE];	// FIR sum for each value in a set of readings
	unsigned long long ull64 = strV[nIdx].ulltime; // Time of current reading

	for (i = 0; i < OUTPUTRATE; i++) // Output (64) readings for each input
	{
		for (j = 0; j < GPSVALSIZE; j++) // For each of the values
		{
			dSum[j] = 0.0; l = nInIdx;
			for (k = 0; k < GPSBUFSIZE; k++) // For each reading in the buffer
			{ // Buffered input value * LP coefficient
				dSum[j] += (strV[l].d[j] * dLp[i][k]);	
				l -= 1; if (l < 0) l = GPSBUFSIZE -1; // Work backwards thru circular buffer
			}
			dSum[j] *= 64.0;
		}
// Remove average
for (k = 0; k < GPSVALSIZE; k++)
	dSum[k] += dAveX[k];
	// Write output here
		printf ("%10llu %12.8F %12.8F %8.2F %8.2F %8.2F %8.2F 6\n",ull64,dSum[0],dSum[1],dSum[2],dSum[3],dSum[4],dSum[5]);
		ull64 -= 1;	// Decrement time by 1/64th sec
	}

	return;
}
