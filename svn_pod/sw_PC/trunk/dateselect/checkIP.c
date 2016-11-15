/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : checkIP.c
* Creater            : deh
* Date First Issued  : 08/19/2012
* Board              : Linux PC
* Description        : Edit .profile and do switch for svn repositories
*******************************************************************************/
/*

08/18/2012 example--
gcc checkIP.c -o checkIP && ./checkIP 72.77.164.207

08/19/2012 compile and execute via script, then 'cat' the results--
gcc checkIP.c -o checkIP && ./updateFITIP 72.77.164.207 &&  cat ~/.profile
gcc checkIP.c -o checkIP && gcc svnrelocateedit.c -o svnrelocateed && ./updateFITIP 72.77.164.207
*/

#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#define LINESIZE 1024
char buf[LINESIZE];
char vv[96];

/* File name that has "export FITIP"-- Ubuntu, Fedora FC14, Suse */
#define NUMFILENAMES	3
char *bashfile[NUMFILENAMES] = {".profile",".bash_profile",".bashrc"};

/* Subroutine prototypes */
char * spintodot(char *p);
int conversion(char * p);
/* ************************************************************************************************************ */
/*  Yes, this is where it starts.                                                                               */
/* ************************************************************************************************************ */
int main(int argc, char **argv)
{
	char * p;
	int n[4];
	int i;
	int nCt;
	FILE *fpIn;
	FILE *fpOut;


	/* Check if argument was supplied */
	if (argc != 2)
	{
		printf("Command requires one argument, e.g. checkIP 72.77.164.207\n");
		return 1;
	}

	/* Edit argument with new IP address */
	p = argv[1]; nCt = 0;
	/* Count number of 'dots' */
	while (*p != 0)
	{
		if (*p++ == '.') nCt += 1;
	}
	if (nCt != 3)	// Correct number?
	{
		printf ("IP address error: use dotted decimal for, i.e. we expect 3 'dots', e.g\n72.77.164.207\n");
		return 1;
	}
	
	/* Check for non-numeric digits */
	p = argv[1];
	while (*p != 0)
	{
		if ((*p != '.') &&  (((*p - '0') < 0) || ((*p - '0') > 9)))
		{
			printf ("IP address error: one or more digits is not  0 - 9\n");
			return 1;
		}
		p++;
	}

	/* Check for 8 bit numbers */
//	sscanf (argv[1],"%u.%u.%u.%u",&n[0],&n[1],&n[2],&n[3]);
	p = argv[1];
	if ( (n[0] = conversion(p)) < 0) return 1;	// 
	p = spintodot(p);
	if ( (n[1] = conversion(p)) < 0) return 1;	// 
	p = spintodot(p);
	if ( (n[2] = conversion(p)) < 0) return 1;	// 
	p = spintodot(p);
	if ( (n[3] = conversion(p)) < 0) return 1;	// 

	printf("%d.%d.%d.%d\n",n[0],n[1],n[2],n[3]);

	for (i = 0; i < 4; i++)
	{
		if (n[i] > 255)
		{
			printf("IP address error: one or more number greater than 255\n");
			return 1;
		}
	}

	/* Edit .profile file to change export of environmental variable $FITIP to new address */

	for ( i = 0; i < NUMFILENAMES; i++)
	{
	/* Open read file  */
		if ( (fpIn = fopen (bashfile[i],"r")) == NULL)
		{
			 continue;
		}
		if ( (fpOut = fopen (".x.tmp","w")) == NULL)
		{
			printf ("\n.x.tmp file (temporary for %s) did not open\n",bashfile[i]); continue;
		}
		break;
	}
	if ( i >= NUMFILENAMES ) return 1;

	printf ("Opening: %s\n",bashfile[i]);

	/* Edit */
	nCt = 0;	
	while ( (fgets (&buf[0],LINESIZE,fpIn)) != NULL)
	{
		if ( (strncmp ("export FITIP=",buf,13)) == 0)
		{
			sprintf(vv,"export FITIP=%d.%d.%d.%d\n",n[0],n[1],n[2],n[3]);
			fputs(vv,fpOut);
			nCt += 1;
		}
		else
		{
			fputs(buf,fpOut);
		}
		
	}
	if (nCt == 0) {printf ("export FITIP= was not found\n");return 1;}
	if (nCt > 1)   printf ("export FITIP= multiple cases, %u (warning)\n",nCt);

	fclose(fpIn);
	fclose(fpOut);	// If we don't close this .x.tmp will appear empty to the following!

	sprintf (vv,"cp .x.tmp %s",bashfile[i]);
	nCt = system (vv);
	if (nCt != 0)  {printf ("copy from .x.tmp to %s failed\n",vv); return 1;}
	
	printf ("UPDATED: %s\n",bashfile[i]);

	return 0;

}
/* ************************************************************************************************************ */
/*  Convert ascii up to next '.'                                                                                */
/* return: negative for no data											*/
/* ************************************************************************************************************ */
int conversion(char * p)
{
	int x = 0;
	int i = 0;

	while (i < 18)
	{

		if ((*p == 0) || (*p == 0x0a) || (*p == '.'))
		{
			break;	
		}
		x = x*10 + (*p - '0');
//printf("%d %c %d\n",x,*p, i);
		p++;
		i += 1;
	}
	if ((i == 0) || (i > 3))
	{
		printf("IP address error: Missing char, or too many between dots\n");
		return -1;
	}

	return x;
}	
/* ************************************************************************************************************ */
/*  Convert ascii up to next '.'                                                                                */
/* return: pointer to '.'											*/
/* ************************************************************************************************************ */
char * spintodot(char *p)
{
	int i = 0;
	while ((*p++ != '.') && (i < 15));
	return p;
}

