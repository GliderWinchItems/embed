#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static FILE *in, *out;

static long do_file(char *, char *, long noRecords);

main(int argc, char *argv[])
{
	int noRecords;
	
	if(argc != 3) 
		exit(EXIT_FAILURE);
		
	noRecords = do_file(argv[1], argv[2], 0L);
	if(noRecords < 1)
		exit(EXIT_FAILURE);	
	do_file(argv[1], argv[2], noRecords);

	exit(EXIT_SUCCESS);
}

static void out_c(char *);
static void out_4(long);
static void out_2(short);

static long do_file(char *in_file, char *out_file, long noRecords)
{
	long unix_time;
	long tension;
	float x_acc, y_acc, z_acc, total_acc;
	char human_time[30];
	long tick;

	long noLines = 0;
	
	in = fopen(in_file, "r");
	out = fopen(out_file, "wb");
	if(!in || !out)
		exit(EXIT_FAILURE);

	while(1==1)
	{
		int n, tmp;
		int ch = fgetc(in);				/* Peek at 1st char of line */
		
		if(ch == EOF) break;

		if(ch != '1')					/* Skip non-data lines */
		{
			while(ch != '\n')
			{
				ch = fgetc(in);
				if(ch == EOF) break;	/* Handle incomplete line */
			}
			continue;					/* Process next record */
		}

		ungetc(ch, in);					/* Put the 1st char back */

		n = fscanf(in, "%ld %ld %f %f %f %f\n",
				&unix_time, &tension, &x_acc, &y_acc, &z_acc, &total_acc);
				
		if(n != 6)						/* Number of fields is bad */
			continue;

		noLines += 1;					/* Good line.  Count it. */

		if(noRecords != 0)				/* Process it */
		{
			if(noLines == 1)
			{
				out_c("RIFF");
				out_4(44 + (4 * noRecords));
				out_c("WAVE");
				
				out_c("fmt ");
				out_4(16);
				out_2(1);
				out_2(2);
				out_4(64);
				out_4(256);
				out_2(4);
				out_2(16);
				
				out_c("data");
				out_4(4 * noRecords);
			}
		
			out_2(tension / 100);
			out_2((short)(total_acc * 1000.0));

if(1==0)
			printf("%ld %ld %f %f %f %f\n",
					unix_time, tension, x_acc, y_acc, z_acc, total_acc);
		}
	}
	
	fclose(in);

	return noLines;
}

static void out_c(char *p)
{
	fputc(*p++, out);
	fputc(*p++, out);
	fputc(*p++, out);
	fputc(*p++, out);
}

static void out_4(long i)
{
	fputc((i >>  0) & 0xff, out);
	fputc((i >>  8) & 0xff, out);
	fputc((i >> 16) & 0xff, out);
	fputc((i >> 24) & 0xff, out);
}

static void out_2(short i)
{
	fputc((i >> 0) & 0xff, out);
	fputc((i >> 8) & 0xff, out);
}

