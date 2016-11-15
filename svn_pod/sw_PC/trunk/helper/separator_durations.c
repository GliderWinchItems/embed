/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : separator_durations.c
* Creater            : deh
* Date First Issued  : 12/14/2012
* Board              : Linux PC
* Description        : Make a list of durations based on tension thresholds
*******************************************************************************/
/*
Hack of reformatgpsaccelmerge.c

// 12/12/2012 example--
gcc separator_durations.c -o separator_durations && ./separator_durations 50000 1000 50 < ~/winch/download/121208/121208.174500R
Where arg #1 = Tension threshold (grams), increasing threshold, detect launch in progress
Where arg #2 = Tension threshold (grams), decreasing threshold, detect launch ending
Where arg #3 = (optional) length of tension input line
STDIN = reformatted & sorted input file

Some alternate examples--
gcc separator_durations.c -o separator_durations && ./sepgo 50000 1000 121208/x

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
#define NUMLINESPERSEC	73
#define NUMSECS 3
#define NUMLINESDELAY (NUMSECS*NUMLINESPERSEC)
#define PADEND	(NUMLINESPERSEC*3)



char bufdelay[NUMLINESDELAY][LINESIZE];
int bufidx = 0;

unsigned int nlinect = 0;
unsigned int nlinelen_ten;
char sepfile[256];
char vv[256];
char tt[256];
char tt_start[256];
unsigned int sepct = 0;
/* ************************************************************************************************************ */
/*  Yes, this is where it starts.                                                                               */
/* ************************************************************************************************************ */
int main(int argc, char **argv)
{
	unsigned long long ullStart,ullEnd,ull;
	long long llTemp;
	int nTemp,nTemp1;
	int nX = 0;
	int nEventctr = 0;
	int nLaunchctr=1;
	int nTension = 0;
	int nThreshold_up;
	int nThreshold_dn;
	int nCtend;
	int i;

	if (argc > 4)
	{
		fprintf(stderr, "usage: $ separator_durations linelength < <infile> \n");
		exit(EXIT_FAILURE);
	}

	if (argc == 4)
	{
	 	sscanf (argv[3],"%u",&nlinelen_ten);
	}
	else
	{
		nlinelen_ten = 50;
	}

	if ((argc == 3) |(argc == 4) )
	{
		sscanf (argv[1],"%u",&nThreshold_up);
		sscanf (argv[2],"%u",&nThreshold_dn);
		printf ("Tension Threshold: up %d  down  %d\n",nThreshold_up, nThreshold_dn);
	}
	else
	{ 

	}

/* ************************************************************************************************************ */
/*                            */
/* ************************************************************************************************************ */
	while ( (fgets (&bufdelay[bufidx][0],LINESIZE,stdin)) != NULL)	// Get a line from stdin
	{

		nlinect += 1;	// Simple file line counter
		i = strlen (&bufdelay[bufidx][0]);
		if (i == nlinelen_ten)
		{
			sscanf (&bufdelay[bufidx][0],"%10llu %d",&ull,&nTension);
			strcpy (tt,&bufdelay[bufidx][0]);
		}
		switch (nX)
		{
		case 0:
			if (nTension > nThreshold_up)
			{
				strcpy (tt_start,tt);	// Save up-going tension 
				nX = 1;	
			}
			break;

		case 1:
			if (nTension < nThreshold_dn)
			{	
				sscanf(tt_start,"%10llu",&ullStart);
				
//printf("%10llu \n",ull>>5);
				nTemp1= (int)((ullStart >> 5) - (ullEnd >> 5));	// Get difference in seconds
				sscanf(tt      ,"%10llu",&ullEnd);
				nTemp = (int)((ullEnd >> 5) - (ullStart >> 5));
				nEventctr += 1;	// Count detection of launch tension (or greater!)
				nX = 0;
				if (nTemp > 10)	// Skip off-scale bogus events
				{
					printf ("%4d %10d %10.1f %s",nLaunchctr,nTemp, (double)(nTemp1/60.0), &tt[10]);
					nLaunchctr += 1;	// Count launches
				}
			}
			break;
		}
	}		
	return 0;
}

