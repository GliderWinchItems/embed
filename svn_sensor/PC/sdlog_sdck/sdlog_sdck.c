/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "sdcardio.h"
#include "sdcard_scan.h"

/* Command line variables
 */
int scanlogblk = (1==0);
int32_t lowblk = 0;
int32_t highblk = 0x7fffffff;

int scanlogpid = (1==0);
int64_t lowpid = 0;
int64_t highpid = 0x7fffffffffffffff;

char *sdcard = NULL;

/* Function prototypes.
 */
void usage(void);
void do_command_line(int argc, char *argv[]);

/* Main starts here.
 */
int main(int argc, char *argv[])
{
	int num_blocks;
	
	do_command_line(argc, argv);
	
	if(argc < 2) usage();
	
	sdcard_scan(sdcard);
	
	sdcard_close();
	exit(EXIT_SUCCESS);
}

void do_command_line(int argc, char *argv[])
{
	int n;
	char *s;
	
	if(argc < 2) usage();
	
	for(argc-=1,argv++; argc > 0; argc-=1,argv++)
	{
		s = "--sdcard="; n = strlen(s);
		if(strncmp(*argv, s, n) == 0)
		{
			sdcard = *argv + n;
		}

		s = "--scanlogblk"; n = strlen(s);
		if(strncmp(*argv, s, n) == 0)
		{
			fprintf(stderr, "%s not implemented yet!\n", s);
			exit(EXIT_FAILURE);
		}	
	}
}

void usage(void)
{
	#define	P(msg) fprintf(stderr, msg)

	P("# usage: $ sdlog_sdck {<switches> }*\n");
	P("#   where: <switches> ::=\n");
	P("#     --sdcard=<filename>\n");
	P("#     --scanlogblk{=<lowblk>{:<highblk>}}\n");

	exit(EXIT_FAILURE);
}

