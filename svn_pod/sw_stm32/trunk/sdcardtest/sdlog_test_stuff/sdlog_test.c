#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sdlog.h"

/* Global test case flags
 */
int write_pass_flag;
int read_pass_flag;

extern int fd;
extern int sdlog_err;

#define MYBUF_SIZE 512
char mybuf[MYBUF_SIZE];

long start_seqn, number_read;
long max_items;

main(int argc, char *argv[])
{
	int i;
	int item_num;
	int err;

	if(argc != 5)
	{
		fprintf(stderr,
			"usage: $ sdlog_test <sim_file> <rw_flags> <max_items> <start_seqn>\n");
		exit(1);
	}

	fd = open(argv[1], O_RDWR | O_SYNC);
	if(fd == -1)
	{
		printf("## Can't open \"%s\"\n", argv[1]);
		exit(1);
	}
	
	else if(strcmp(argv[2], "r") == 0)
		read_pass_flag = (1==1), write_pass_flag = (1==0);
	else if(strcmp(argv[2], "w") == 0)
		read_pass_flag = (1==0), write_pass_flag = (1==1);
	else if(strcmp(argv[2], "rw") == 0)
		read_pass_flag = (1==1), write_pass_flag = (1==1);
	else if(strcmp(argv[2], "wr") == 0)
		read_pass_flag = (1==1), write_pass_flag = (1==1);
	else
		exit(1);
		
	max_items = atol(argv[3]);
	start_seqn = atol(argv[4]);

	/* Start sdlog test
	 */
	err = sdlog_init();
	if(err != 0)
		printf("sdlog_init() returns %d\n", err);

	if(write_pass_flag)
	{
		int n;
		
		printf("## Starting WRITE pass\n");
		for(item_num=0; item_num < max_items; item_num += 1)
		{
			n = get_packet(mybuf, sizeof(mybuf));
			if(n <= 0)
				break;
			sdlog_write(mybuf, n);
		}
		printf("Number of items written is %d\n", item_num);
		sdlog_flush();
	}

	if(read_pass_flag)
	{
		int n;

		printf("## Starting READ pass\n");
		for(item_num=0; ; item_num += 1)
		{
			int n = sdlog_read_backwards(mybuf, MYBUF_SIZE);

			if (n == SDLOG_EOF)
			{
				printf("\n# sdlog_read_backwards returns EOF\n");
				break;
			}

			if(n >= sizeof(number_read))
			{
				memcpy((void *)&number_read, mybuf, sizeof(number_read));
				printf("%ld\n", number_read);
				continue;
			}
			
			printf("\n# sdlog_read_backwards returns %d/%d\n", n, sdlog_err);
		}
		printf("Number of items read is %d\n", item_num);
		sdlog_flush();
	}

	/* end sdlog test
	 */
	close(fd);
	exit(0);
}

int get_packet(char *buf, int n)
{
//	assert(n >= sizeof(start_seqn));
	memcpy(buf, (void *)&start_seqn, sizeof(start_seqn));
	start_seqn += 1;
	return sizeof(start_seqn);
}

void wf(int n) { printf("//wf(%d);\n", n); }
void wf2(int n, int m) { printf("//wf2(%d, %d);\n", n, m); }
void wfs(char *p) { printf("//wfs(\"%s\");\n", p); }
void wfull(unsigned long long ull) { printf("//wfull(%llu);\n", ull); }
