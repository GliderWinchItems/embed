/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : 32KHz_cal.c
* Hackeroo           : deh
* Date First Issued  : 11/03/2011
* Board              : Linux PC
* Description        : Build a calibration table for the 32 KHz osc v temperature
*******************************************************************************/
/*
 gcc 32KHz_cal.c -o 32KHz_cal && ./32KHz_cal /home/deh/minicom_20111102_2310.txt
 gcc 32KHz_cal.c -o 32KHz_cal && ./32KHz_cal minicom_20111105_2222.txt
 gcc 32KHz_cal.c -o 32KHz_cal && ./32KHz_cal minicom_20111111_1610.txt


This reads the captured data from the POD for the 'g' or 'c' commands and extracts
the 32 KHz 1 sec ticks and processor ticks from the GPS 1 pps.  These are used to 
compute the 32 KHz freq error.  The temperature is also extracted from the input.

The output is in the form of a file that can be used to compile a calibration table.

The temperature is stored in the high order 16 bits and the ppb error in the lower
16 bits.  The temperature is between 0 and 50 deg C, which stores a 0 to 5000.  Negative
numbers could be used, but temperature data will all be above freezing so the
numbers will be positive.  The ppb values are all positive.  The freq offset and temperature
error is all in the direction for positive numbers.


*/

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

static int compare_int(const void *a, const void *b);


#define DATSIZE 200000	// Number of readings to deal with
int 	nTemp[DATSIZE];	// temperature in upper 16 bits, ppb in lower 16 bits

#define BUFSIZE 256

char	cmd;
unsigned int secs,remdr,subtick,xtalticks,procticks,ppb,ppbwtemp,ppbtemp,adctemp;
int diffhi, difflo,itemp,ippb;
double	temp;
double dppb;

FILE *fpIn;

int main(int argc, char **argv)
{
	int i,j;
	int imax;
	char buf[BUFSIZE];


/* ************************************************************************************************************ */
/* Handle command line arguments, if any.                                                                       */
/*  Set these up on struct rwfile1                                                                              */
/* ************************************************************************************************************ */
	if (argc != 2)	// Requires an input file name on the command line
	{
		printf ("\nNeed input file name %d %s\n",argc,*argv);return 1;
	}

/* ************************************************************************************************************ */
/* Open read file                                                                                               */
/* ************************************************************************************************************ */
	if ( (fpIn = fopen (*(argv+1),"r")) == NULL)
	{
		printf ("\nInput file did not open %s\n",*argv); return 1;
	}
/* ************************************************************************************************************ */
/* Read in lines captured from POD                                                                              */
/* ************************************************************************************************************ */
	i = 0;
	while ( (fgets (&buf[0],BUFSIZE,fpIn)) != NULL)
	{
		j = strlen(buf);
		if ( j == 151)	// Is the length of the captured line what we are looking for?
		{
//			printf ("%6u %s",i,buf);
			sscanf(buf,"%c %u %u %d %d %u %u %u %u %u %u %u %lf", 
	&cmd, &secs, &remdr, &diffhi, &difflo, &subtick, &xtalticks, &procticks, &ppb, &ppbwtemp, &ppbtemp, &adctemp, &temp);

		if ( (procticks < (48000000 + 6000)) && (procticks > (48000000 - 6000)) )
		{
/* NOTE--changes the following to do what you want! */
			// 8 MHz osc ticks
			dppb = procticks;
//			dppb = ((1.0E9 * dppb)/48E6) - 1E9;// ppb
			dppb = dppb - 48E6;

					
			// 32 KHz osc ticks
//			dppb = xtalticks;
//			dppb = ((2.0E9 * dppb)/procticks) - 1E9;

			itemp = temp * 100;	// Convert xx.xx floating input temp to xxxx integer

//			printf ("%6u %6u %6u %8d %8d %4u %7u %7u %7.1lf %6u %6u %6u %10.2f %6d\n",i, secs,
//	diffhi, difflo, subtick, xtalticks, procticks, ppb, dppb,ppbwtemp, ppbtemp, adctemp, temp, itemp);

//			if (( i > 2000) && ( i < 6000))
//			printf ("%6u %6u %8.2lf\n",i,itemp,dppb);

			ippb = (dppb +0.5);	// Parts per billion error; convert to integer

			/* Skip of any bogus values */
			if ((ippb < 64000) && (ippb > 0))
			{
				/* Save the pertinent data for whatever we might want to do */
				nTemp[i] = itemp << 16;	// Temperature (deg C * 100)
				nTemp[i] = nTemp[i] | (ippb & 0xffff);	// Put ppb in lower 16 bits
				i++;
			}

			if (i >= DATSIZE) // Be sure to stay within the array limits
			{
				fclose (fpIn);
				printf ("Input file too big %u\n",DATSIZE); return 1;
			}
		}
		}
		imax = i;	// Save last index + 1
	}
/* ************************************************************************************************************ 
 * Sort by temperature
 * Remember that the temperature is in the high order 16 bits
 * ************************************************************************************************************ */
	qsort (nTemp,imax,sizeof (int),compare_int);	// Sorts in ascending order
	printf ("Sort done\n");				// Just a reminder
/* ************************************************************************************************************ 
 * Build an array with the average ppb for each discrete temperature reading
 * ************************************************************************************************************ */

int iAve;	// Averaging accumulator
int iAvect;	// Average counter
int k = 0;	// Count of discrete temperature readings
	j = nTemp[0]>>16;	// Current temperature reading
	i = 0;			// Input array index
	while (i < imax-1)	// Go until we hit the end
	{
		iAve = 0;	iAvect = 0;	// Initial for average

		while ((j == nTemp[i]>>16) && (i < imax-1))
		{
			iAve += (nTemp[i] & 0xffff); // Accumulate ppb
			iAvect += 1;		// Count number of cases at this temperature
			i ++;			// Input index advance
		}
		k += 1;				// Number of different temperature readings
		printf ("%6u %6u %6d %6u\n",k, j, iAve/iAvect, iAvect);
		j = nTemp[i]>>16;	// Update current temperature reading
	}

	printf ("\n\nEND");

	return 0;
}
/* ************************************************************************************************************ 
 * Comparison function for qsort
 * ************************************************************************************************************ */

      static int compare_int(const void *a, const void *b)
       {
		const int *da = (const int *) a;
		const int *db = (const int *) b;
     
		return (*da > *db) - (*da < *db);
       }




