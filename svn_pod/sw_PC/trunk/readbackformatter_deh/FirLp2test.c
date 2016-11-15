/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : FirLp2test.c
* Creater            : deh
* Date First Issued  : 07/25/2012
* Board              : Linux PC
* Description        : Test FirLp2.c
*******************************************************************************/
/*

// Compiple--
gcc FirLp2.c FirLp2test.c -o FirLp2test -lm

// Compiple & execute--
gcc FirLp2.c FirLp2test.c -o FirLp2test -lm && ./FirLp2test

// Compile & execute with parameters (16 taps, 0.2 cutoff, Hamming type)
gcc FirLp2.c FirLp2test.c -o FirLp2test -lm && ./FirLp2test 16 0.2 0
*/

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "FirLp2.h"


int main(int argc, char **argv)
{
	int i;
	int nCoeffSize;
	double dCutoff;
	int nW;

	struct FIRLPCO2 firlp2co2;

	/* Check the command line arguments */
	if (argc != 4)
	{
		printf("argument count = %u, it should be 1\n",argc-1);
		printf("Args:\n[1] number of coefficients\n[2] cutoff\n[3] type filter:\n   0 = Hamming\n   1 = Hanning\n   2 = Blackman\n   3 = RaisedCosine\n");
		return -1;
	}

	/* Convert coefficient array size to binary */
	sscanf (argv[1],"%u", &nCoeffSize);
	printf("Coefficient size: %u\n",nCoeffSize);

	/* Convert cutoff */
	sscanf (argv[2],"%lf",&dCutoff);
	printf("Cutoff %F\n",dCutoff);

	/* Convert filter type */
	sscanf (argv[3],"%u",&nW);
	printf("Filter type: %u\n",nW);

	/* Setup struct parameters */
	firlp2co2.nSze = nCoeffSize;

	/* Allocate memory and Compute coefficients. Complete setting up firlpco2 */
	FirLpInit(&firlp2co2, nCoeffSize, dCutoff, nW);

	/* List coefficients */
	
	for (i = 0; i < (firlp2co2.nSze/2 + (firlp2co2.nSze & 0x01)); i++)
	{
		printf ("%3u %14.9F\n",i, *(firlp2co2.dpHBase + i) );
	}

	return 0;
}

