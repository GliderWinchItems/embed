/******************************************************************************
* File Name          : dottextcompare.c
* Date First Issued  : 01/07/2015
* Board              : PC
* Description        : Do struct size comparison
*******************************************************************************/
/*
 gcc dottextcompare.c -o dottextcompare && ./dottextcompare
*/
#include <stdio.h>
#include "tmpstruct.h"



/******************************************************************************
*******************************************************************************/
int main(int argc, char *argv[])
{
	int sw = 0;
	int numsubsystems;
	int numunits;

#define MAXSTRUCTS	32	
	int sz[MAXSTRUCTS];
	int slotct[MAXSTRUCTS];

#include "sizeof.c"

	int i;
	for (i = 0; i < numsubsystems; i++)
	{
		if (sz[i] != (slotct[i] * 4))
		{
			printf("##############################################################################################################\n"
			"ERROR: index: %u struct size: (%u slots) (%u bytes) |  slot ct: (%u slots) (%u bytes)\n"
				"##############################################################################################################\n"
				,i, sz[i]/4, sz[i], slotct[i], (slotct[i] * 4));
			sw = 1;
		}

	}
	if (sw != 0)
	{
		return -1;
	}
	return 0;
}

