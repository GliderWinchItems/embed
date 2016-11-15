/******************************************************************************
* File Name          : dottexttoparmsize.c
* Date First Issued  : 01/04/2015
* Board              : PC
* Description        : Make "sizeof" and sizes for parameter fields from .txt file
*******************************************************************************/
/*
cd ~/svn_discoveryf4/PC/sensor/cangate/trunk
gcc dottexttoparmsize.c -o dottexttoparmsize -Wall -I ../../../../sw_discoveryf4/trunk/lib -I ../../../../common_all/trunk parse.o winch_merge2.o && ./dottexttoparmsize ~/svn_discoveryf4/PC/sensor/cangate/test/testmsg2B.txt
*/
#include <string.h>
#include <stdio.h>

#include "common_highflash.h"
#include "parse.h"
#include "parse_print.h"
#include "winch_merge2.h"	// struct CANCAL, plus some subroutines of use

#define NOPRINTF	1	// parse.c: 0 = print; not zero = exclude

/* Subroutine declarations */

/* From cangate */
FILE *fpList;	// canldr.c File with paths and program names
FILE *fpOut;
int fdp;		// File descriptor for input file

char *outfile = "sizeof.c";

/* List of program specs */
#define NUMPROGS	64	// Number of load specs
static struct LDPROGSPEC  ldp[NUMPROGS];
//static int loadpathfileIdx = 0; // Index in program number
static int unitMax = 0; // Index+1 of last program number

/* List of CAN IDs extracted from input file. */
#define SIZECANID	512	// Max number array will hold
static struct CANIDETAL canidetal[SIZECANID];
static int idsize;	// Number stored in the foregoing array.

void both(char* p)
{
	printf("%s",p);
	fprintf(fpOut,"%s",p);
	return;
}

/******************************************************************************
 * main
*******************************************************************************/
int main(int argc, char **argv)
{

	int ret;
	int i,j;
	int ct;
	int slot_ct;
	char s[256];

	/* Get .txt file with the specs. */
	if (argc != 2)
	{ // Number of command line arguments not correct.
		printf("Wrong number of arguments to dottexttoparmsize.c: %d\n",argc); return -1;
	}

	if ( (fpList = fopen (argv[1],"r")) == NULL)
	{ // File with .txt data did not open
		printf("Test msg file OR program loading path/file given on command line did not open: %s\n",argv[1]); return -1;
	}
	else
	{ // Success!
//		printf("Test msg file opened: %s\n",argv[1]);
	}

	if ( (fpOut = fopen (outfile,"w")) == NULL)
	{
		printf("Output file %s did not open.\n",outfile); return -1;
	}


	/* Build a list of CAN IDs */
	idsize = parse_buildcanidlist(fpList, canidetal, SIZECANID);
	if (idsize < 0){printf("EXIT: FILE CAN ID EXTRACTION ERROR when building CAN ID list from file: %d\n", idsize);return -1; }
printf("/* ");
	/* Get the list of path/file for the CAN bus unit programs */
	ret = parse(fpList, &ldp[0], NUMPROGS, canidetal,idsize,NOPRINTF);	// Read (or re-read) input file and build list	
	if ( ret < 0 ) {printf("EXIT: FILE PARSE ERROR when passing file the second time\n");return -1; }
printf("\n*/ \n");

	/* List sizes of each parameter, calibration, CAN IDs area. */
	unitMax = ret;	// Save number of units in parsed file.
	sprintf(s,"numunits = %u;\n",unitMax); both(s);
	ct = 0;
	for (i = 0; i < unitMax; i++)
	{
		for (j = 0; j < ldp[i].subsysnum; j++)
		{
			if (ldp[i].subsysnum > 0)
			{
				sprintf(s,"sz[%3i]   = sizeof(struct %s);\t// %s\t@ %s\n",
				ct, 
				&ldp[i].subsys[j].structname[0], 
				&ldp[i].c_ldr.name[0], 
				&ldp[i].subsys[j].description[0]);
				both(s);

				/* Get number of slots designated. */
				if (ldp[i].subsysct == 0)
				{
					slot_ct = ldp[i].slotidx;
				}
				else
				{
					slot_ct = (ldp[i].subsys[j].relidx_slot - ldp[i].subsys[j-1].relidx_slot);
				}
				sprintf(s,"slotct[%u] = %u;\n",ct,slot_ct); both(s);
				ct += 1;
				sprintf(s,"\n"); both(s);
			}
		}
	}
	sprintf(s,"numsubsystems = %u;\n",ct); both(s);
	

	return 0;

}
