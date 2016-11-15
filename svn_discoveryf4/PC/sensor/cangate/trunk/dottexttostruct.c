/******************************************************************************
* File Name          : dottexttostruct.c
* Date First Issued  : 01/01/2015
* Board              : PC
* Description        : Strip out '//v' lines from .txt file which hold structs 
*******************************************************************************/
/*
cd ~/svn_discoveryf4/PC/sensor/cangate/trunk
gcc dottexttostruct.c -o dottexttostruct -Wall && checkstruct ~/svn_discoveryf4/PC/sensor/cangate/test/testmsg2B.txt
*/
#include <string.h>
#include <stdio.h>

	#define LINESIZE 256
	char buf[LINESIZE];

/******************************************************************************
*******************************************************************************/
int main(void)
{
	while ( (fgets (&buf[0],LINESIZE,stdin)) != NULL)	// Get a line from stdin
	{
		if (strncmp(buf, "//v",3) == 0)
		{
			printf("%s",&buf[3]);
		}
	}
	return 0;
}

