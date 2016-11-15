/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : ratechangecic.c
* Creater            : deh
* Date First Issued  : 07/31/2012
* Board              : Linux PC
* Description        : Convert 5 Hz gps lines contained the output of reformat.c to 64 Hz (CIC filter)
*******************************************************************************/
/*
08/04/2012 - Change from table supplied cic coefficients to subroutine computed coefficients 
             which makes changing cic filter order a bit easier. (rev 615 prior to this change).

// Command line example when the input is via stdin
To compile--
gcc ../cic/ciccogen.c ratechangecic.c -o ratechangecic -lm  -Wall

To execute (output to console)--
./ratechangecic < ../dateselect/120722.182240RS

To execute (output to file)
./ratechangecic < ../dateselect/120722.182240RS > file.txt

To execute (output to console and file)
./ratechangecic < ../dateselect/120722.182240RS | tee > file.txt

To compile and execute--
gcc ../cic/ciccogen.c ratechangecic.c -o ratechangecic -lm  -Wall && ./ratechangecic < ../dateselect/120722.182240RS

To compile, execute, and sort--
gcc ../cic/ciccogen.c ratechangecic.c -o ratechangecic -lm  -Wall && ./ratechangecic < ../dateselect/120722.182240RS | tee | sort

To output results to a file, e.g. "test", append  > test

Courtney Campbell--
./ratechangecic < ../dateselect/120815.181711RS | tee | sort

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
#define CICORDER	4	// CIC filter order 

/* Resampling */
#define INPUTRATE	5	// Input data rate 5 Hz
#define OUTPUTRATE	64	// Output data rate 64 Hz

/* Polyphase filter coefficients */
#define LPCOEFFICIENTSIZE  ((OUTPUTRATE * CICORDER)-(CICORDER - 1))	// Number of LP filter taps

/* Group Delay */
#define CICMIDDLE (LPCOEFFICIENTSIZE/2)	// Middle array index (exact if CICORDER is even)
#define GROUPDELAYTICKS (CICMIDDLE/OUTPUTRATE)	// Whole number of time ticks
#define GROUPDELAYSUBTICK (CICMIDDLE-((CICMIDDLE/OUTPUTRATE)*OUTPUTRATE))

/* CIC filter coefficients (used to build polyphase filters) */
unsigned long long ullCo[OUTPUTRATE * CICORDER];

/* Commutating (polyphase) filters for rate change */
#define GPSBUFSIZE	CICORDER	// Input buffer size is the same as the CIC filter order
double dLp[OUTPUTRATE][GPSBUFSIZE];


int iR;					// Index for commutating polyphase filters
unsigned long long ullGpstime64; 	// Time stamp saved when gps 1/10th seconds = zero.
unsigned long long ullGpstime320;	// ullGpstime64 times INPUTRATE (5) (= 320/sec) with group delay offset subtracted

/* Values extracted from ascii in GPS line */
#define GPSVALSIZE	6	// Number of GPS values on one line
// 0 Latitude (degrees)
// 1 Longitude (degrees)
// 2 Height (m above MSL)
// 3 Velocity East (m/s)
// 4 Velocity North (m/s)
// 5 Velocity Up (m/s)

/* GPS input data buffer */
struct GPSVALUES
{
	unsigned long long ulltime;	// Linux time in 1/64th sec ticks
	int	nSecs;			// Seconds
	int	nSecs10th;		// Tenth seconds
	double d[GPSVALSIZE];		// One set of readings
}strV[GPSBUFSIZE];	// GPS input data buffer

int nInIdx;	// Index into buffer
int nBufFillCt;	// One time switch	

int nSwX;	// Switch for by-passing time stamp error check during buffer filling & time sync'ing
int nGPSsw;	// GPS data buffering initialization state

unsigned int nCoeffgain;	// Polyphase filter gain

/* Line buffer size */
#define LINESIZE 2048	// Big and plenty
char buf[LINESIZE];

char infile[128];	// Input file name buffer

int nTestCt;	// Debugging & testing rate change
double dS;	// Debugging & testing sine wave

/* Subroutine prototypes */
void ratechangeall(void);
void extract_values(struct GPSVALUES *g, char * p, int j);
void missinglinecheck(void);

/* ************************************************************************************************************ */
/*  Yes, this is where it starts.                                                                               */
/* ************************************************************************************************************ */
int main(int argc, char **argv)
{
	int i,j,k;
	char cC;
	double dX;
	unsigned long long ullX;

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
		for ( j = 0; j < GPSBUFSIZE; j++)
		{
			if (k < LPCOEFFICIENTSIZE ) 
				dLp[OUTPUTRATE-1-i][j] = ullCo[k] * (1.0/nCoeffgain) ;
			else
				dLp[OUTPUTRATE-1-i][j] = 0;	// Take care of zeroes not in table

// Debug
//printf ("%3u%3u%4u %4u%4u%9llu\n",j,i,(OUTPUTRATE-1-i),l,k,ullCo[k]);	// List the ordering 
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
		for ( j = 0; j < GPSBUFSIZE; j++)
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
			printf ("%s",buf);	// Pass it to the output
			break;

		case '6': // GPS data


//			printf ("%s",buf);	// Pass input to the output for debugging, etc.
			extract_values(&strV[nInIdx], buf, j);	// Convert ascii input to binary
			
			/* Save extended (1/64th ticks) Linux time for even seconds (i.e. 1/10th sec = 0) */
			if (strV[nInIdx].nSecs10th == 0)  	// Is this the even seconds?
			{ // Here, yes.
				ullGpstime64   = strV[nInIdx].ulltime; // Save 1/64th sec tick count
				ullX  = (ullGpstime64 * INPUTRATE); // Convert to upsampling rate
				ullX -= CICMIDDLE;	// Subtract offset for group delay
				if ((ullX != ullGpstime320) && (nSwX != 0))	// New time stamp equal carried forward time?
				{ // Here, no.
					printf("TIME STAMP DIFF: %llu %llu %lld\n",ullX,ullGpstime320,ullX-ullGpstime320);
				}
				ullGpstime320 = ullX;
			}
		

			/* The following 'switch' takes care of startup and sync'ing to the seconds epoch */
			switch(nGPSsw)
			{

			case 0:	// Fill gps buffer with readings
				/* Startup: fill buffer with values before doing rate change */
				if (nBufFillCt <= CICORDER) 	// More readings to be buffered?
				{ // Here, yes.
					nBufFillCt += 1;	// Count inputs to buffer
					break;
				}
				/* Here buffer has been filled */
				nGPSsw = 1;	// Drop thru to see if luck got us seconds x.0

			case 1: // Sync rate upsampling to even seconds (x.0)
				if (strV[nInIdx].nSecs10th != 0) // Even seconds?
					break;	// No, keep buffering readings until even seconds comes up
			
				/* Here, this is an even seconds sample */				
				ullGpstime64= strV[nInIdx].ulltime;// Save even secs time
				iR = 0;		// Polyphase filter index (zero jic)	
				nSwX = 1;	// Allow time stamp check
				nGPSsw = 2;	// Next case

			case 2: // Here, endless entry
				missinglinecheck();	// Fill in missing lines
				/* For new input (5 Hz), convert rate and output (64 Hz) */
				ratechangeall();

			} // End of switch(nGPSsw)

			/* Advance readings input (cricular) buffer index */
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
#define GPSLINEBUFSIZE	128

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
//printf("Lat: ");
	sscanf((p+11),"%2u",&iX);
	g->d[0] = iX;
	sscanf((p+14),"%lf",&dX);
//printf("  %4u %8.5F ",iX,dX);
	g->d[0] += (dX/60.0);
//printf (" decimal: %12.8F\n",g->d[0]);

	/* Longitude (degrees) (we won't screw around with EW) */
//printf("Long: ");
	sscanf((p+24),"%3u",&iX);
	g->d[1] = iX;
	sscanf((p+27),"%lf",&dX);
//printf(" %4u %8.5F",iX,dX);
	g->d[1] += (dX/60.0);
//printf ("  decimal: %12.8F\n",g->d[1]);

	/* Height (m above MSL) */
	sscanf((p+42),"%lf",&dX);
	g->d[2] =  dX;
//printf ("Ht:  %12.5F\n",g->d[2]);

// Get seconds and 1/10th seconds for checking for missing lines.
	sscanf((p+61),"%2u",&iX);
	g->nSecs =  iX;		// Whole seconds
	g->nSecs10th =  *(p+64) - '0'; // Tenth seconds

if ((g->nSecs10th < 0) || (g->nSecs10th > 8))
{
  printf ("HANG: %d %s\n",g->nSecs10th,buf);
  while (1==1);
}
//printf (" Seconds: %2u.%1u\n",g->nSecs,g->nSecs10th);
	
	/* Velocities (m/s) */	
	/* Velocity East (signed) */
	sscanf((p+73),"%lf",&dX);
	g->d[3] =  dX;
//printf ("VelE: %8.2F\n",g->d[3]);
	
	/* Velocity North (signed) */
	sscanf((p+81),"%lf",&dX);
	g->d[4] =  dX;
//printf ("VelN: %8.2F\n",g->d[4]);

	/* Velocity Up (signed) */
	sscanf((p+90),"%lf",&dX);
	g->d[5] =  dX;
//printf ("VelU: %8.2F\n",g->d[5]);
//printf("\n");	

	return;	// Uncommenting this returns before overriding the input data with the following

/* ##################### override input for test purposes ########################## */
// Tinkering with response to a sine wave
//		dS += 2*((2.0 * 3.14159265358079323)/5.0); // 2 Hz
//		dX = sin(dS);
//		for (iX = 0; iX < 6; iX++) // Fill the set of readings with low value
//			g->d[iX] = dX;
//return;

	if ((nTestCt++ % 11) <  5) // Alternate step
//	if (nTestCt++ != 15)	// Impulse
	{
		for (iX = 0; iX < 6; iX++) // Fill the set of readings with low value
			g->d[iX] = -1;
	}
	else
	{
		for (iX = 0; iX < 6; iX++) // Fill the set of readings with high value
			g->d[iX] = 1;
	}
	/* We don't want ton's of output */
	if (nTestCt > 60) exit(0);	// For a long input file, cut the output short

	return;
}
/* *************************************************************************************************************
 * void ratechangeall(void);
 * @brief	: Given a set of data for the new input (5 Hz) compute and output the new outputs (64 Hz)
 * @param	: Index of new values into GPSVALUES buffer
************************************************************************************************************* */
void ratechangeall(void)
{
	int j,k,l;
	double dSum[GPSVALSIZE];	// FIR sum for each value in a set of readings
	unsigned long long ull64;	// Working time

	int m = 0;	// Debug aid

	/* The number of outputs for 5 inputs is 13, 13, 13, 13, 12 = 64 = 1 second */
	do
	{
		for (j = 0; j < GPSVALSIZE; j++) // Rate change each of the values
		{
/* These 'if (j == 0)' are for looking at the indexing and "shift register" computations */
//if (j == 0) printf ("*");

			dSum[j] = 0.0; 	// FIR accumulator initialize
			l = nInIdx;	// Set working index into the input data buffer
			for (k = 0; k < GPSBUFSIZE; k++) // For each reading in the buffer
			{ // Buffered input value * LP coefficient
				l += 1; if (l >= GPSBUFSIZE) l = 0; // Work forward from oldest reading in circular input buffer
				dSum[j] += (strV[l].d[j] * dLp[iR][k]);	// Data point * polyphase filter coefficient 	
//if (j == 0) printf("%4u%4u%8u%4.1F",iR,k,(int)(nCoeffgain * dLp[iR][k]),strV[l].d[j]);
			}
			dSum[j] *= OUTPUTRATE; // Scale for upsampling (more efficiently done when setting up the table of coefficients)
//if (j == 0) printf("\n");
		}

	// Write output here
		/* Output rate changed line */
		ull64 = (ullGpstime320 + 2)/5;	// Somewhat rounded up conversion to 64 ticks per sec
		printf ("%10llu %13.9F %13.9F %8.2F %8.2F %8.2F %8.2F 6\n",ull64,dSum[0],dSum[1],dSum[2],dSum[3],dSum[4],dSum[5]);

// Debug: printf with extras
//printf ("%10llu %13.9F %13.9F %8.2F %8.2F %8.2F %8.2F %4u %3u %3u\n",ull64,dSum[0],dSum[1],dSum[2],dSum[3],dSum[4],dSum[5],(int)(ull64 & 63), iR, m);

		m  += 1;	// Just a counter for debugging
		ullGpstime320 += INPUTRATE;	// Advance group delay adjusted time
		iR += 5;	// Increment index to next polyphase filter

	} while (iR < 64);
	iR -= 64;


	return;
}
/* *************************************************************************************************************
 * void missinglinecheck(void);
 * @brief	: Using secs|tenth secs, see if a gps line is missing, and fill in last value if so.
 * @param	: Index of new values into GPSVALUES buffer
************************************************************************************************************* */
int nMissingGPSct;	// Count of missing lines during this run

void missinglinecheck(void)
{
	int nSecs;		// Previous reading secs
	int nSecs10th;		// Previous reading 1/10th secs
	int nPrev;		// nPrev is previous reading index
	int nSw = 1;		// Switch to stop filling in missing lines
	long long llX;// Working number
	unsigned long long ullTime, ullX;
	struct GPSVALUES strX;	// Local copy to latest input set of readings

	while ( nSw != 0)
	{
		/* Compute index for previous reading */
		nPrev = nInIdx - 1;
		if (nPrev < 0) nPrev = GPSBUFSIZE-1;
		
		/* Get seconds & time stamp from previous reading */
		nSecs     = strV[nPrev].nSecs;
		nSecs10th = strV[nPrev].nSecs10th;
		ullTime = strV[nPrev].ulltime;

		/* Increment previous seconds */
		nSecs10th += 2;		// 5 Hz is 0.2 sec steps
		if (nSecs10th >= 10)
		{
			nSecs10th = 0;
			nSecs += 1;
			if (nSecs >= 60) nSecs = 0;
		}
		
		/* See if it matches latest reading seconds and that any difference is "reasonable" */
		llX = strV[nInIdx].ulltime - ullTime; 	// Find difference (in 1/64th sec ticks)
		if (llX < 0) ullX = - ullX;	// Make absolute

if ((strV[nInIdx].nSecs10th < 0) || (strV[nInIdx].nSecs10th > 8))
{
  printf ("HANG1: %d %s\n",strV[nInIdx].nSecs10th,buf);
  while (1==1);
}

		if ( ((nSecs10th != strV[nInIdx].nSecs10th) || ( nSecs != strV[nInIdx].nSecs)) && (llX < 64*2) )
		{ // Here, they differ and the difference is "reasonable"
printf ("$$$%4u.%1u | %4u:%1d\n",nSecs,nSecs10th, strV[nInIdx].nSecs, strV[nInIdx].nSecs10th);
			strX = strV[nInIdx];		// Save the latest set of readings
			strV[nInIdx] = strV[nPrev];	// Replace the latest with previous readings
			strV[nInIdx].nSecs = nSecs;	// Update seconds
			strV[nInIdx].nSecs10th = nSecs10th; // and 1/10th secs
			ratechangeall();		// Do a rate change
			nInIdx += 1;			// Advance current position in circular buffer
			if (nInIdx >= GPSBUFSIZE) nInIdx = 0;
			strV[nInIdx] = strX;		// Place the saved, latest reading into new position in buffer
			/* Go see if this brought us into match */
		}
		else
		{ // Here they are the same
			nSw = 0;	// Set flag to quit
		}
	}

	return;
}
