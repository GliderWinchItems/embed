/* File: float_time.c -- Unix absolute 1/64th sec. to relative sec. as float
 */ 
/*
Compile--
gcc float_time.c -o float_time
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define	MYBUFSZ	1000

main(int argc, char *argv[])
{
	int linelength;

	if(argc != 2)
	{
		fprintf(stderr, "usage: $ float_time linelength < <infile> > <outfile>\n");
		exit(EXIT_FAILURE);
	}
	sscanf (argv[1],"%u",&linelength);

	while(1==1)
	{
		char mybuf[MYBUFSZ], *p = mybuf;
		long long int time, t0;
		static int first_time_flag = (1==1);

		p = fgets(mybuf, MYBUFSZ, stdin);
		if(p == NULL) break;
		
		if(strlen(p) != linelength) continue;
		
		time = atoll(mybuf);
		if(first_time_flag)
		{
			t0 = time;
			first_time_flag = (1==0);
		}
		
		while(isblank(*p)) p++;
		while(isdigit(*p)) p++;
		while(isblank(*p)) p++;
		
		printf("%11.6f  %s", (time-t0)/64.0, p);
	}	
}
