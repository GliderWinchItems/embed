#include <stdio.h>
#include <stdlib.h>
#include <string.h>

main(int argc, char *argv[])
{
	int i;
	
	if(argc >= 3 &&
0 &&
	   (strcmp(argv[1], "-c") == 0))
	{
		for(i=2; i<argc; i+=1)
			printf("%s ", argv[i]);
		printf("\n");
		exit(EXIT_SUCCESS);
	}

	while(1) sleep(120);
}
