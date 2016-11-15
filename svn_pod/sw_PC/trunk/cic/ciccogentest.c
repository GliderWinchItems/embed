/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : ciccogentest.c
* Creater            : deh
* Date First Issued  : 08/03/2012
* Board              : Linux PC
* Description        : Test subroutine to generate CIC coefficients
*******************************************************************************/
/*

Compile and execute command line--
gcc ciccogen.c ciccogentest.c -o ciccogentest && ./ciccogentest 64 3 test

*/

#include <stdio.h>
#include "ciccogen.h"

/******************************************************************************
 Begin here
 ******************************************************************************/
int main(int argc, char **argv)
{
	int i;
	int nRate;
	int nOrder;
	FILE *fpOut;
/* ************************************************************************************************************ */
/* Handle command line arguments                                                                                */
/* ************************************************************************************************************ */
	if (argc != 4)	// Requires all three arguments
	{ // Here, something wrong, give a reminder message
		printf ("\nArguments we saw: %d %s\nNeed boxcar length CIC order and file name\ne.g. ./ciccotest 64 3 test.txt",argc,*argv);return -1;
	}

	/* Convert arguments ascii -> binary */
	sscanf (argv[1],"%u",&nRate);	// Number of taps in boxcar
	sscanf (argv[2],"%u",&nOrder);	// CIC order
	
	/* Assign array that receives the coefficients from the 'ciccogen.c' */
	unsigned long long ullCo[nRate*nOrder];	
	
	/* Open output file */
	if ( (fpOut = fopen (argv[3],"w")) == NULL)
	{ // Here, output open failed.
		printf ("\nOutput file did not open %s\n",argv[3]); return -1;
	}		

	/* Convolve boxcar multiple times to compute the coefficients and return them in 'nCo' */
	ciccogen(ullCo, nRate, nOrder);

	/* Polyphase filter gain */
	unsigned long long	ullGain = 0;
	for (i = 0; i < (nRate*nOrder); i += nRate)
		ullGain += ullCo[i];

	/* Sum of coefficients */	
	unsigned long long ullCosum = 0;
	for (i = 0; i < (nRate*nOrder)-(nOrder-1); i++)
		ullCosum += ullCo[i];

	/* Output a file that can be used in a C routine */
	// Header lines
	fprintf (fpOut,"/*  FILE NAME: %s */\n",argv[3]);
	fprintf (fpOut,"/*  CIC coefficients for CIC%u */\n",nOrder);
	fprintf (fpOut,"int nCICsize %u;\t// Number taps in convolving boxcar\n",nRate);
	fprintf (fpOut,"int nCICorder %u;\t// CIC order\n",nOrder);
	fprintf (fpOut,"int nCICgain %llu;\t// Polyphase filter gain\n",ullGain);
	fprintf (fpOut,"int nCICcosum %llu;\t// Sum of coefficients\n",ullCosum);
	fprintf (fpOut,"int nCICcoeff[%u];\n{\n",nRate*nOrder);

	// Output coefficients
	for (i = 0; i < (nRate*nOrder)-(nOrder-1); i++)
	{
		printf ("%3u %14llu\n",i+1,ullCo[i]);	// Output list to console
		fprintf (fpOut,"%14llu,\t/*%5u  */\n",ullCo[i],i); // Output for .c file
	}

	/* Finishing */
	fprintf (fpOut,"};\n\n");// End c file initializtion
	
	printf ("Done\n");	// Something for the hapless op.

	return 0;	// Success return
}

