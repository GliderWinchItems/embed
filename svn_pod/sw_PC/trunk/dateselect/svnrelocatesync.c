/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : svnrelocatesync.c
* Creater            : deh
* Date First Issued  : 12/14/2014
* Board              : Linux PC
* Description        : Update svn mirror repository with new IP address
*******************************************************************************/
/*

12/14/2014: Hacked from svnrelocateedit.c (08/19/2012)

example to compile and execute--
cd ~/svn_pod/*PC/trunk/dat*
gcc svnrelocatesync.c -o svnrelocatesync && cd ~/svn_pod && svn info | ./svnrelocatesync
cd ~/svn_pod/*PC/trunk/dat*
gcc svnrelocatesync.c -o svnrelocatesync && ./syncFITIP 72.91.23.232


deh@dehmain:/svn_pod/db/revprops/0$ cat 0
K 8
svn:date
V 27
2011-05-21T19:56:19.362746Z
K 17
svn:sync-from-url
V 39
svn+sshtunnelfit://72.91.23.232/svn_pod
K 18
svn:sync-from-uuid
V 36
900eff0e-d345-40af-95fc-a62b781a0c03
K 24
svn:sync-last-merged-rev
V 3
871
END

*/

#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#define NUMLINES 100	// Number of lines in the file
#define LINESIZE 1024	// Max length of a line
char old[NUMLINES][LINESIZE];
char new[LINESIZE];
char vv[96];
char strsave[LINESIZE];

/* ************************************************************************************************************ */
/*  Yes, this is where it starts.                                                                               */
/* ************************************************************************************************************ */
int main(int argc, char **argv)
{
	FILE* fp;

	char * p;
	char * pp;
	unsigned int n[4];
	int i;
	int nCt = 0;
	int Goodflg = 0;

	/* Check if argument was supplied */
	if (argc != 3)
	{
		printf("Command requires two arguments, number: %d e.g.\nsvnreloateedit \n", argc);
		return 1;
	}
	printf("args: %s %s\n",argv[1],argv[2]);
FILE* fp1 = fopen("test.txt","w");
	
	if ( (fp = fopen(argv[1],"r+")) == NULL)
	{
		printf("file did not open\n"); return -1;
	}

	/* Check if we got anything piped in */
	printf("ORIGINAL INPUT FILE\n");
	while ( ((fgets (&old[nCt][0],LINESIZE,fp)) != NULL) && (nCt < NUMLINES))
	{
printf("%d: %s",nCt,&old[nCt][0]);
		if ( (strncmp("tun",&old[nCt][7],3) ) == 0)
		{ // Here, we found the line with the goodies
//			printf("%s",&old[nCt][0]);

			if ((nCt & 0x1) != 1)
			{
				printf("Line that matches svn+sshtunnel is not an odd index\n"); return -1;
			}

			/* Replace old IP with new IP */
			pp = &old[nCt][0];		
			while (*pp++ != '.') ; // Spin up to "."
			pp = strchr(pp,'/'); 	// Spin up to '/'
			if (pp == NULL)
			{
				printf("Last / not found\n"); return -1;
			}
			strcpy(strsave, pp);	// Save trailing path/file

			pp = &old[nCt][0];		
			while (*pp++ != '.') ;  // Spin up to "."
			while (*pp-- != '/');	// Back up to '/'
			pp +=2;			// Position for ip address
			strcpy (pp,argv[2]);	// Copy new IP address
			strcat (pp, strsave);	// Add trailing path/file
			
			/* Adjust line size */
			i = strlen(&old[nCt][0]);
			pp = &old[nCt-1][0];
			sprintf( (pp+2),"%u\n", (unsigned int)strlen(&old[nCt][0])-1 );
//			printf("%s",&old[nCt-1][0]);
//			printf("%s",&old[nCt][0]);
			Goodflg = 1;
			
		}
		nCt += 1;
	}

printf("\nCONVERTED OUTPUT\n");
	if (Goodflg == 0 )
	{
		printf("FAILED %d\n", nCt); return -1;
	}
int ret;
	rewind(fp);
	for (i = 0; i < nCt; i++)
	{
		printf("%s",&old[i][0]);
		ret = fprintf(fp,"%s",&old[i][0]);
		if (ret < 0)
			{printf ("#### FILE WRITE ERROR #### %d\n",ret);}
	}

	fclose(fp);
	return 0;

}

