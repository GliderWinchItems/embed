/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : convert_vector.c
* Creater            : deh
* Date First Issued  : 10/05/2012
* Board              : Linux PC
* Description        : Convert ST definitions for F3 to libopenstm32 format
*******************************************************************************/
/*
// 03-22-2012 example--
gcc convert_vector.c -o convert_vector && ./convert_vector < ../../../stm32vector.txt
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
char infile[100][256];	// Input file name buffer
char xfile[100][256];	// Modified file

/* ************************************************************************************************************ */
/*  Yes, this is where it starts.                                                                               */
/* ************************************************************************************************************ */
int main(int argc, char **argv)
{
	int linect = 1;
	char *p;
	int i,j;
	int savect = 0;
/* ************************************************************************************************************ */
/* Read line with ST def's                                                                              */
/* ************************************************************************************************************ */
	while ( (fgets (&infile[savect][0],LINESIZE,stdin)) != NULL)
	{
		if ( (linect != 12) && !( (linect == 1) || (linect == 2) || (linect == 3) ) )
		{
				p = &infile[savect][0]; 
				while ( *p != 0) {*p = tolower(*p); p++;}
				savect += 1;
		}
		linect += 1;
	}
	savect -= 2;
	for (i = 0; i < savect; i++)
	{
		p = &infile[i][0]; j = 0;
		while ((*p != 'r') && (j++ < 50)) p++;
		*p = 's';
		while ((*p != 'q') && (j++ < 50)) p++;
		*p = 'r';
		while ((*p != 'n') && (j++ < 70)) p++;
		*p++ = ',';
		while ((*p != '=') && (j++ < 120)) p++;
		*(p-1) = '/'; *p = '*';
		while ((*p != '/') && (j++ < 120)) p++;
		*p++ = ' '; *p = ' ';

		printf ("%s",&infile[i][0]);
	}
	for (i = 0; i < savect; i++)
	{
		p = &infile[i][0]; j = 0;
		while (*p != ',') p++;
		*p++ = '(';*p++ = 'v';*p++ = 'o';*p++ = 'i';*p++ = 'd';*p++ = ')';*p++ = ';';
		printf ("void WEAK %s",&infile[i][0]);
		
	}
	for (i = 0; i < savect; i++)
	{
		p = &infile[i][0]; j = 0;
		while (*p != '(') p++;
		*p = '\0';
		printf ("#pragma weak%s = weak null_handler\n",&infile[i][0]);
	}

	return 0;
}


