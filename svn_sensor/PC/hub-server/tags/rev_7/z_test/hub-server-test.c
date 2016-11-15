#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void usage(void)
{
	fprintf(stderr, "usage: hub-server-test <start-seqn> <end-seqn> <letter-len>\n");
	exit(EXIT_FAILURE);
}

main(int argc, char *argv[])
{
	long start, end, len;
	int n;
	char ch = 'a';

	if(argc != 4)
		usage();
	
	start = atol(argv[1]);
	end = atol(argv[2]);
	len = atol(argv[3]);
	
	if(start < 0 || end < 0 || start >= end)
	{
		fprintf(stderr, "Inconsistent args: start is %ld, end is %ld, len is %ld\n", 
					start, end, len);
		exit(EXIT_FAILURE);
	}
	
	while(start <= end)
	{
		printf("%ld ", start);
		
		for(n = 0; n < len; n += 1)
		{
			printf("%c", ch);
			ch += 1;
			if(ch > 'z')
				ch = 'a';
		}
		
		printf("\n");
		
		start += 1;
	}

	sleep(60);
	
	exit(EXIT_SUCCESS);
}
