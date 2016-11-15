/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : separator_tension.c
* Creater            : deh
* Date First Issued  : 12/12/2012
* Board              : Linux PC
* Description        : Separate a big file into small files based on tension
*******************************************************************************/
/*
Hack of reformatgpsaccelmerge.c

// 12/12/2012 example--
gcc separator_tension.c -o separator_tension && ./separator_tension 50000 1000 50 < ~/winch/download/121208/121208.174500R
gcc separator_tension.c -o separator_tension && ./separator_tension 50000 1000 50 < ~/winch/download/130106/130106.165800R
gcc separator_tension.c -o separator_tension && ./separator_tension 50000 1000 50 < ~/winch/download/130421/130421.145820R
Where arg #1 = Tension threshold (grams), increasing threshold, detect launch in progress
Where arg #2 = Tension threshold (grams), decreasing threshold, detect launch ending
Where arg #3 = (optional) length of tension input line
STDIN = reformatted & sorted input file

Some alternate examples--
gcc separator_tension.c -o separator_tension && ./sepgo 50000 1000 121208/x

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
FILE *fpOut;
char sepfile[256];
char vv[256];
char tt[256];
unsigned int sepct = 0;
int nX = 0;
/* *************************************************************************************************************
 * void outputboth(char *p)
 * Brief: Outputs on the console and also the output file
 * Arg: p = pointer to a string
 ************************************************************************************************************* */
void outputboth(char *p)
{
//	printf ("%s",p);	// For the hapless Op or piping
	fputs(p, fpOut);	// For the serious programmer
	return;
}

/* ************************************************************************************************************ */
/*  Yes, this is where it starts.                                                                               */
/* ************************************************************************************************************ */
int main(int argc, char **argv)
{
	unsigned long long ull;
	int nTension = 0;
	int nThreshold_up;
	int nThreshold_dn;
	int nCtend;
	int i;

	if (argc > 4)
	{
		fprintf(stderr, "usage: $ separator_tension linelength < <infile> \n");
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

	strcpy(sepfile,"set.000R");
	sepct += 1;
	sprintf (vv,"%03uR",sepct);
	strcpy ((sepfile+4),vv);

	if ( (fpOut = fopen (sepfile,"w")) == NULL)
	{
		printf("File open error %s\n",sepfile);
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
//printf("%10llu %9d %s",ull, nTension,&bufdelay[bufidx][0]);
			strcpy (tt,&bufdelay[bufidx][0]);
		}
//printf("%s",&bufdelay[bufidx][0]);
		switch (nX)
		{
		case 0: // Fill circular buffer
			bufidx += 1;						
			if (bufidx >= NUMLINESDELAY) 
			{
				bufidx = 0; nX = 1;
			}
			break;

		case 1:
			if (nTension > nThreshold_up)
			{
				for (i = 0; i < NUMLINESDELAY; i++)
				{
					bufidx += 1;	if (bufidx >= NUMLINESDELAY) bufidx = 0;
					outputboth(&bufdelay[bufidx][0]);
				}
				nX = 2;	
printf("%10u UP  %s\n",nlinect,sepfile);
			}
			else
			{
					bufidx += 1;	if (bufidx >= NUMLINESDELAY) bufidx = 0;
			}
			break;

			case 2:
			outputboth(&bufdelay[bufidx][0]);
			bufidx += 1;	if (bufidx >= NUMLINESDELAY) bufidx = 0;
			if (nTension < nThreshold_dn)
			{
				nX = 3;	nCtend = 0;
printf("%10u DN  %s\n",nlinect, sepfile);
			}
			break;

			case 3:
			outputboth(&bufdelay[bufidx][0]);
			bufidx += 1;	if (bufidx >= NUMLINESDELAY) bufidx = 0;
			nCtend += 1;
			if (nCtend >= PADEND)
			{
printf("%10u END %s %s",nlinect, sepfile, &tt[10]);
				fclose(fpOut);
				sepct += 1;
				sprintf (vv,"%03uR",sepct);
				strcpy ((sepfile+4),vv);
printf("%s\n",sepfile);
				if ( (fpOut = fopen (sepfile,"w")) == NULL)
				{
					printf("File open error %s\n",sepfile);
				}					
				nX = 1;
			}
		}
	}		
	fclose(fpOut);
	return 0;
}

