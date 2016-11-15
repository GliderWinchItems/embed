/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : reformatgps2clean.c
* Creater            : deh
* Date First Issued  : 08/18/2012
* Board              : Linux PC
* Description        : Strip junk lines from output of reformatgps2.c, do any last minute rearranging
*******************************************************************************/
/*

// 08/18/2012 example--
gcc reformatgps2clean.c -o reformatgps2clean -lm && sudo ./ratechangecic < ../dateselect/120815.173751RS | sort | ./reformatgps2 | ./reformatgps2clean


*/

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

#define LINESIZE 1024
char buf[LINESIZE];

/* ************************************************************************************************************ */
/*  Yes, this is where it starts.                                                                               */
/* ************************************************************************************************************ */
int main(int argc, char **argv)
{
	int j;

	while ( (fgets (&buf[0],LINESIZE,stdin)) != NULL)	// Get a line from stdin
	{
		j = strlen(buf);	// j = length of line
		if (j < 117) continue;	// Skip out if wrong size

		printf ("%s",buf);
	
	}
	return 0;
}
