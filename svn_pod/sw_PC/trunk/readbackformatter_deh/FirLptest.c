/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : FirLp2test.c
* Creater            : deh
* Date First Issued  : 07/25/2012
* Board              : Linux PC
* Description        : Test FirLp2.c
*******************************************************************************/
/*

// Compiple--
gcc FirLp2.c FirLp2test.c -o FirLptest -lm


*/

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

int main(int argc, char **argv)
{
	int nCoeffSize;

	if (argc != 1)
	{
		printf("argument count = %u, it should be 1\n",argc-1);
		printf("Args: coefficient_size\n");
		return -1;
	}

	sscanf (*argv[1],"%u", &nCoeffSize);
	printf("Coefficient size: %u/n",nCoeffSize);


	return 0;
}

