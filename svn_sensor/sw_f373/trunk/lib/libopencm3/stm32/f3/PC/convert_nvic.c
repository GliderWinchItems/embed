/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : convert_nvic.c
* Creater            : deh
* Date First Issued  : 10/05/2012
* Board              : Linux PC
* Description        : Convert ST definitions for F3 to libopenstm32 format
*******************************************************************************/
/*
// 03-22-2012 example--
gcc convert_nvic.c -o convert_nvic && ./convert_nvic < ../../../stm32vector.txt
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
char infile[256];	// Input file name buffer

/* ************************************************************************************************************ */
/*  Yes, this is where it starts.                                                                               */
/* ************************************************************************************************************ */
int main(int argc, char **argv)
{
	int linect = 1;
	char *p;
	int i;
/* ************************************************************************************************************ */
/* Read line with ST def's                                                                              */
/* ************************************************************************************************************ */
	while ( (fgets (&buf[0],LINESIZE,stdin)) != NULL)
	{
		switch (linect)
		{
			case 1:
			case 2:
			case 3:
			case 12:
				break;
			default:
				p = buf; i = 0;
				while ((*p != 'Q') && (i++ < 50)) p++;
				while ((*p != 'n') && (i++ < 50)) p++;
				*p = ' ';
				while ((*p != '=') && (i++ < 100)) p++;
				*p = ' ';
				while ((*p != ',') && (i++ < 100)) p++;
				*p = ' ';
				while ((*p != '!') && (i++ < 100)) p++;
				*p++ = ' '; *p = ' ';
				printf ("#define  NVIC_%s",&buf[2]);
		}
		linect += 1;
	}
	return 0;
}


