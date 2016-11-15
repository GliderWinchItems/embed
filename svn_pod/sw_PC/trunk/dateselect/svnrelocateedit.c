/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : svnrelocateedit.c
* Creater            : deh
* Date First Issued  : 08/19/2012
* Board              : Linux PC
* Description        : Edit the output of svn info
*******************************************************************************/
/*

08/19/2012 example to compile and execute--
gcc svnrelocateedit.c -o svnrelocateedit && cd ~/svn_pod && svn info | ./svnrelocateedit
gcc svnrelocateedit.c -o svnrelocateedit && ./updateFITIP 72.77.164.207

*/

#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#define LINESIZE 1024
char old[LINESIZE];
char new[LINESIZE];
char vv[96];

/* ************************************************************************************************************ */
/*  Yes, this is where it starts.                                                                               */
/* ************************************************************************************************************ */
int main(int argc, char **argv)
{
	char * p;
	char * pp;
	unsigned int n[4];
	int i;
	int nCt = 0;

	/* Check if argument was supplied */
	if (argc != 2)
	{
		printf("Command requires two arguments, e.g.\nsvnreloateedit \n");
		return 1;
	}
	/* Check if we got anything piped in */
	while ( (fgets (&old[0],LINESIZE,stdin)) != NULL)
	{
		if ( (strncmp("URL",old,3)) == 0)
		{ // Here, we found the line with the goodies
//			printf("%s",&old[5]);
			nCt += 1;
			break;
		}
	}
	if (nCt != 1) return 1;

	/* Copy old, and insert new ip in 'new' */
	p= &new[0]; pp=&old[5];
	/* Copy everything up to first '.' in old ip address */
	while (*pp != '.') *p++ = *pp++;
	while (*p-- != '/');	// Back up to '/'
	p +=2;			// Position for ip address
	strcpy (p,argv[1]);	// Copy new IP address
	while (*pp++ != '/');	// Move old pointer forward to '/'
	p=&new[0]; pp --;		// 
	while (*p++ != 0);	// 
	p--;			// Point to '\0' in 'new'
	while (!((*pp == 0) || (*pp == '\n'))) // Don't copy '\n'
		*p++ = *pp++;	// Copy remainder of old to new
	*pp++= 0;		// Overwrite '\n' with '\0'
	*p++ = 0;		// Space between old and new URLs
		
	sprintf(vv,"svn switch --relocate %s %s\n",&old[5],new);	
	
	system (vv);
	return 0;

}

