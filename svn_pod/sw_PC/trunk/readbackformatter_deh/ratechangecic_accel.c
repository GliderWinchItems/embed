/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : ratechangecic_accel.c
* Creater            : deh
* Date First Issued  : 07/31/2012
* Board              : Linux PC
* Description        : Convert 8 Hz accelerometer to 64 Hz (using CIC filter)
*******************************************************************************/
/*
11/04/2012 A hack of 'ratechangecic.c' to do rate changing for accelerometer data.
Greatly simplified since it is a 4:1 rate change wherease GPS was a 64/5 change.


// Command line example when the input is via stdin
To compile--
gcc ../cic/ciccogen.c ratechangecic_accel.c -o ratechangecic_accel -lm  -Wall

// Compile & execute test
gcc ../cic/ciccogen.c ratechangecic_accel.c -o ratechangecic_accel -lm  -Wall && ./ratechangecic_accel < ~/winch/download/121104/121104.004043RC

To execute (output to console)--
./ratechangecic < ~/winch/download/121104/121104.004043RC

*/

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include "../cic/ciccogen.h"

/* CIC filtering */
#define CICORDER	8	// CIC filter order 

/* Resampling */
#define INPUTRATE	1	// Input data rate 8 Hz
#define OUTPUTRATE	4	// Output data rate 64 Hz

/* Polyphase filter coefficients */
#define LPCOEFFICIENTSIZE  ((OUTPUTRATE * CICORDER)-(CICORDER - 1))	// Number of LP filter taps

/* Group Delay */
#define CICMIDDLE (LPCOEFFICIENTSIZE/2)	// Middle array index (exact if CICORDER is even)
#define GROUPDELAYTICKS (CICMIDDLE/OUTPUTRATE)	// Whole number of time ticks
#define GROUPDELAYSUBTICK (CICMIDDLE-((CICMIDDLE/OUTPUTRATE)*OUTPUTRATE))

/* CIC filter coefficients (used to build polyphase filters) */
unsigned long long ullCo[OUTPUTRATE * CICORDER];

/* Commutating (polyphase) filters for rate change */
#define ACCELBUFSIZE	CICORDER	// Input buffer size is the same as the CIC filter order
double dLp[OUTPUTRATE][ACCELBUFSIZE];


int iR;					// Index for commutating polyphase filters
unsigned long long ullGpstime64; 	// Time stamp saved when gps 1/10th seconds = zero.
unsigned long long ullGpstime320;	// ullGpstime64 times INPUTRATE (5) (= 320/sec) with group delay offset subtracted

/* Values extracted from ascii in Accelerometer line */
#define ACCELVALSIZE	4	// Number of Accelerometer values on one line
// X
// Y
// Z
// Vector magnitude

/* Accelerometer input data buffer */
struct ACCELVALUES
{
	unsigned long long ulltime;	// Linux time in 1/64th sec ticks
	double d[ACCELVALSIZE];		// One set of readings
}strV[ACCELBUFSIZE];	// GPS input data buffer

int nInIdx;	// Index into buffer
int nBufFillCt = 0;	// One time switch	
int nAccelsw;

unsigned int nCoeffgain;	// Polyphase filter gain

/* Line buffer size */
#define LINESIZE 2048	// Big and plenty
char buf[LINESIZE];

char infile[128];	// Input file name buffer

int nTestCt;	// Debugging & testing rate change
double dS;	// Debugging & testing sine wave

/* Subroutine prototypes */
void ratechangeall(void);
void extract_values(struct ACCELVALUES *g, char * p, int j);
void missinglinecheck(void);

/* ************************************************************************************************************ */
/*  Yes, this is where it starts.                                                                               */
/* ************************************************************************************************************ */
int main(int argc, char **argv)
{
	int i,j,k;
	char cC;
	double dX;
//	unsigned long long ullX;

/* ************************************************************************************************************ */
/* CIC filter setup: reorder linear array into multiple polyphase filters					*/
/* ************************************************************************************************************ */
	/* Convolve boxcar multiple times to compute the coefficients and return them in 'nCo' */
	ciccogen(ullCo, OUTPUTRATE, CICORDER);	// Compute array of coefficients for CIC filter

	/* Compute gain of one polyphase filter to be used later as a check */
	unsigned int nCoeffsum = 0;
	for (i = 0; i < (OUTPUTRATE * CICORDER); i += OUTPUTRATE)
		nCoeffsum += ullCo[i];

	/* Compute total sum, or gain for the filter to be used for scaling */
	nCoeffgain = 0;
	for (i = 0; i < LPCOEFFICIENTSIZE; i ++)
		nCoeffgain += ullCo[i];


	k = 0; // Index into input coefficient array

	for ( i = 0; i < OUTPUTRATE; i++)
	{
		k = i;
		for ( j = 0; j < ACCELBUFSIZE; j++)
		{
			if (k < LPCOEFFICIENTSIZE ) 
				dLp[OUTPUTRATE-1-i][j] = ullCo[k] * (1.0/nCoeffgain) ;
			else
				dLp[OUTPUTRATE-1-i][j] = 0;	// Take care of zeroes not in table

// Debug
//printf ("%3u%3u%4u %4u%9llu\n",j,i,(OUTPUTRATE-1-i),k,ullCo[k]);	// List the ordering 
if ( k >= OUTPUTRATE * CICORDER)	// Check index range to see if we did something really stupid
 printf("ERROR: %u %u %u\n",i,j,k);

			k += OUTPUTRATE;
		}
	}
	nInIdx = 0;  	// Do we really need this? No.

	/* Check that the coefficient table and ordering is correct */
	for ( i = 0; i < OUTPUTRATE; i++) // Check that sums of each polyphase filter
	{
		dX = 0.0;
		for ( j = 0; j < ACCELBUFSIZE; j++)
			dX += dLp[i][j];
		if ( (nCoeffgain * dX) != nCoeffsum )	// Is sum correct?
		{ // Here no.  Show the index, value and bombout
			printf ("FAIL: %3u %12.9F %9u\n",i,dX*nCoeffgain,nCoeffsum);
			return -1;
		}
	}
//return 0; // Return to be able to see the debugging listing
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
			printf ("%s",buf);	// Pass it to the output
			break;

		case '2': // GPS time stamp v Linux
			printf ("%s",buf);	// Pass it to the output
			break;		

		case '3': // Battery/temp
			printf ("%s",buf);	// Pass it to the output
			break;

		case '4': // Pushbutton
			printf ("%s",buf);	// Pass it to the output
			break;

		case '5': // Accelerometer data line
//			printf ("%s",buf);	// Pass input to the output for debugging, etc.
			extract_values(&strV[nInIdx], buf, j);	// Convert ascii input to binary
			
			/* The following 'switch' takes care of startup and sync'ing to the seconds epoch */
			switch(nAccelsw)
			{

			case 0:	// Fill accel buffer with readings
				/* Startup: fill buffer with values before doing rate change */
				if (nBufFillCt <= CICORDER) 	// More readings to be buffered?
				{ // Here, yes.
					nBufFillCt += 1;	// Count inputs to buffer
					break;
				}
				/* Here buffer has been filled */

				iR = 0;		// Polyphase filter index (zero jic)	
				nAccelsw = 1;	// Next case

			case 1: // Here, endless entry
				/* For new input (8 Hz), convert rate and output (64 Hz) */
				ullGpstime64= strV[nInIdx].ulltime;// Save even secs time
				ratechangeall();

			} // End of switch(nAccelsw)

			/* Advance readings input (cricular) buffer index */
			nInIdx += 1;  if (nInIdx >= ACCELBUFSIZE) nInIdx = 0;

			break;

		case '6': // GPS data
			printf ("%s",buf);	// Pass it to the output
			break;



		default:
			break;
		}
	}

printf ("Done\n");

	return 0;
}
/* *************************************************************************************************************
 * void extract_values(struct ACCELVALUES *g, char * p, int j)
 * @brief	: Extract and convert the ascii fields to binary into struct
 * @param	: struct ACCELVALUES *g = Points to struct that holds time and values
 * @param	: char *p = points into ascii line
 * @param	: int j = length of ascii line
************************************************************************************************************* */
#define GPSLINEBUFSIZE	128

void extract_values(struct ACCELVALUES *g, char * p, int j)
{	
	int iX;		// Temp
	double dX;	// Temp
	unsigned long long ull64;

	if (j >= GPSLINEBUFSIZE)
	{
		printf ("####ERROR: gps line size = %u: %s\n",j,buf);
	}

	/* LInux tick time */
	sscanf(buf,"%10llu",&ull64);
	g->ulltime = ull64;

	/* X-axis */
	sscanf((p+11),"%lf",&dX);
	g->d[0] = dX;
//printf("X: %7.4F ",dX);

	/* Y-axis */
	sscanf((p+21),"%lf",&dX);
	g->d[1] = dX;
//printf("Y: %7.4F ",dX);

	/* Y-axis */
	sscanf((p+32),"%lf",&dX);
	g->d[2] = dX;
//printf("Z: %7.4F ",dX);

	/* vector magnitude */
	sscanf((p+43),"%lf",&dX);
	g->d[3] = dX;
//printf("V: %7.4F ",dX);

//printf("\n");	

	return;	// Uncommenting this returns before overriding the input data with the following

/* ##################### override input for test purposes ########################## */
// Tinkering with response to a sine wave
//		dS += 0.5*((2.0 * 3.14159265358079323)/8.0); // 1/2 Hz
//		dX = sin(dS);
//		for (iX = 0; iX < ACCELVALSIZE; iX++) // Fill the set of readings with low value
//			g->d[iX] = dX;
//return;

	if ((nTestCt++ % 11) <  5) // Alternate step
//	if (nTestCt++ != 15)	// Impulse
	{
		for (iX = 0; iX < ACCELVALSIZE; iX++) // Fill the set of readings with low value
			g->d[iX] = -1;
	}
	else
	{
		for (iX = 0; iX < ACCELVALSIZE; iX++) // Fill the set of readings with high value
			g->d[iX] = 1;
	}
	/* We don't want ton's of output */
	if (nTestCt > 60) exit(0);	// For a long input file, cut the output short

	return;
}
/* *************************************************************************************************************
 * void ratechangeall(void);
 * @brief	: Given a set of data for the new input (8 Hz) compute and output the new outputs (64 Hz)
 * @param	: Index of new values into ACCELVALUES buffer
************************************************************************************************************* */
void ratechangeall(void)
{
	int j,k,l;
	double dSum[ACCELVALSIZE];	// FIR sum for each value in a set of readings
	unsigned long long ull64 = ullGpstime64;	// Working time

	int m = 0;	// Debug aid

	/* The number of outputs for 5 inputs is 13, 13, 13, 13, 12 = 64 = 1 second */
	for (iR = 0; iR < 4; iR++)
	{
		for (j = 0; j < ACCELVALSIZE; j++) // Rate change each of the values
		{
/* These 'if (j == 0)' are for looking at the indexing and "shift register" computations */
//if (j == 0) printf ("*");

			dSum[j] = 0.0; 	// FIR accumulator initialize
			l = nInIdx;	// Set working index into the input data buffer
			for (k = 0; k < ACCELBUFSIZE; k++) // For each reading in the buffer
			{ // Buffered input value * LP coefficient
				l += 1; if (l >= ACCELBUFSIZE) l = 0; // Work forward from oldest reading in circular input buffer
				dSum[j] += (strV[l].d[j] * dLp[iR][k]);	// Data point * polyphase filter coefficient 	
//if (j == 0) printf("%4u%4u%8u%4.1F",iR,k,(int)(nCoeffgain * dLp[iR][k]),strV[l].d[j]);
			}
			dSum[j] *= OUTPUTRATE; // Scale for upsampling (more efficiently done when setting up the table of coefficients)
//if (j == 0) printf("\n");
		}

	// Write output here
		/* Output rate changed line */
		printf ("%10llu %8.3F%8.3F%8.3F%8.3F 5\n",(ull64-CICMIDDLE),dSum[0],dSum[1],dSum[2],dSum[3]);

// Debug: printf with extras
//printf ("%10llu %13.9F %13.9F %8.2F %8.2F %8.2F %8.2F %4u %3u %3u\n",ull64,dSum[0],dSum[1],dSum[2],dSum[3],dSum[4],dSum[5],(int)(ull64 & 63), iR, m);

		m     += 1;	// Just a counter for debugging
		ull64 += 1;	// Fill in times
	}


	return;
}

