/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : reformat2.c
* Creater            : deh
* Date First Issued  : 01/29/2012
* Board              : Linux PC
* Description        : Merge accelerometer & tension data from *sorted* output of reformat.c
*******************************************************************************/
/*
05-20-2012 Uli test problem

// 01-29-2012 example--
gcc reformat2.c -o reformat2 -lm && sudo ./reformat2 ../dateselect/120128.210900RS
Or, when the input is via stdin
gcc reformat2.c -o reformat2 -lm && sudo ./reformat2 < ../dateselect/120128.210900RS

*/

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

/* Subroutine prototypes */
void pkt_tension(void);
void pkt_accelerometer(void);


static int compare_int(const void *a, const void *b);

/* Line buffer size */
#define LINESIZE 2048
char buf[LINESIZE];



unsigned long long llAticktime64;		// Ticktime from packet in 1/64ths sec
unsigned long long llTticktime64;		// Ticktime from packet in 1/64ths sec


char ca[128] = {"    1.000      0.000      0.000      1.000  "};
char infile[128];	// Input file name buffer

/* ************************************************************************************************************ */
/*  Yes, this is where it starts.                                                                               */
/* ************************************************************************************************************ */
int main(int argc, char **argv)
{
	int i,j,k,l,m;
	int nTemp;
	char *p;
	struct tm *ptm;
	char cC;


/* ************************************************************************************************************ */
/* Read in lines formated with 'reformat.c' piped to 'sort' and piped to this routine                           */
/* ************************************************************************************************************ */
	while ( (fgets (&buf[0],LINESIZE,stdin)) != NULL)	// Get a line from stdin
	{
		j = strlen(buf);
		cC = buf[j-2];
		
//printf ("3%i# %s",j,buf);
		switch (cC)
		{
		case '1': // Tension reformatted line size
			pkt_tension();
			break;
		case '5': // Accelerometer reformatted line size
			pkt_accelerometer();
			break;
		case '6': // GPS data
			break;
		default:
printf ("Size: %d\n",j);
			break;
		}
	}
//	printf ("Done\n");
	return 0;
}
/* ************************************************************************************************************* */
/* void pkt_tension(void);												 */
/* Convert tension packet											 */
/* ************************************************************************************************************* */
void pkt_tension(void)
{
#define AINSERT	19
	char c[512];
	char * p;

	/* Get time (1/64th secs) */
	sscanf(buf,"%9llx",&llTticktime64);	// Time ticks
	
	/* Save time string */
	strcpy (c, &buf[AINSERT]); // Save time string

	/* Edit time string to make it import into Excel */
	p = &c[0];
	while (*p != 0)
	{
		if ((*p == ':') || (*p == '|') )
		{
			*p = ' ';
		}
		p++;
	}

	/* Insert acceleration data */
	strcpy (&buf[AINSERT],ca); // Copy accelerometer readings

	/* Replace time string at end */
	p = &buf[AINSERT+strlen(ca)];	// Point to position to append time
	strcpy(p,c);	// Append time string to end of readings

	printf ("%s",buf);	// For the hapless op
	
	return;
}
/* ************************************************************************************************************* */
/* void pkt_accelerometer(void);												 */
/* Convert tension packet											 */
/* ************************************************************************************************************* */
void pkt_accelerometer(void)
{
	/* Get time (1/64th secs) */
	sscanf(buf,"%9llx",&llAticktime64);	// Time ticks

	/* Extract readings */
	buf[55] = 0;	// Place a string terminator after readings
	strcpy (ca,&buf[12]); // Copy just the readings
//printf (" %s\n",ca);
	return;
}


