/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : reformat3.c
* Creater            : deh
* Date First Issued  : 03/22/2012
* Board              : Linux PC
* Description        : Convert output of reformat2.c into different date/time format
*******************************************************************************/
/*
// 03-22-2012 example--
gcc reformat3.c -o reformat3 -lm && sudo ./reformat3 ../dateselect/120223.174200RM

*/

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>


/* Line buffer size */
#define LINESIZE 256
char buf[LINESIZE];

char vv[128];


char * extension = {"out\0"};
char infile[128];	// Input file name buffer
FILE *fpIn;

int imax;
/* ************************************************************************************************************ 
* static int month_asciitoint(char* a)
* @brief	: Convert Month ASCII to Month int                                                                            
* @param	: a = pointer to three char month stinge in ascii, (don't forget the terminating \0)
* @return	: 1 - 12; 0 = not found
/* ************************************************************************************************************ */
static char *masc[12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
static int month_asciitoint(char* a)
{
	int i;	

	for (i = 0; i < 12; i++)
	{
		if (strncmp(a, masc[i], 3) == 0)
			return i;
	}
	return 0;
}




/* ************************************************************************************************************ */
/*  Yes, this is where it starts.                                                                               */
/* ************************************************************************************************************ */
int main(int argc, char **argv)
{
	int i,j,k,l;
	int nTemp;
	int imonth;
	char *p;
	struct tm *ptm;

/* ************************************************************************************************************ */
/* Handle command line arguments, if any.                                                                       */
/*  Set these up on struct rwfile1                                                                              */
/* ************************************************************************************************************ */
	if (argc != 2)	// Requires an input file name on the command line
	{
		printf ("\nreformat3.c: Need input file name %d %s\n",argc,*argv);return -1;
	}

/* ************************************************************************************************************ */
/* Open input file                                                                                              */
/* ************************************************************************************************************ */
	strcpy (infile,*(argv+1));
	strcat (infile,"RM");
	i = strlen(infile);
	if ( (fpIn = fopen (infile,"r")) == NULL)
	{
		printf ("\nreformat3.c: Input file did not open %s\n",*(argv+1)); return -1;
	}
	else
	{
		printf("\nInput file: %s\n",infile);
	}

/* ************************************************************************************************************ */
/* Read in lines captured from POD                                                                              */
/* ************************************************************************************************************ */
	imax = 0;
	while ( (fgets (&buf[0],LINESIZE,fpIn)) != NULL)
	{
		buf[69] = 0;
		if ( (imonth = month_asciitoint(&buf[66])) == 0)
		{
			printf ("Month error: %s\n", &buf[66]);
		}
		sprintf (vv,"%2i ",imonth);
		strncpy (&buf[61],&buf[82],4);
		buf[65] = ' ';
		strncpy (&buf[66],vv,3);
		strncpy (&buf[69],&buf[70],12);
		strncpy (&buf[81],&buf[87],2);
		buf[83] = '\n';
		buf[84] = 0;
		printf ("%s",buf);
	}
	return 0;
}


