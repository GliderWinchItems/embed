/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : cmd_p.c
* Author	     : deh
* Date First Issued  : 09/21/2013
* Board              : PC
* Description        : Program load for one unit
*******************************************************************************/


#include "cmd_p.h"
static int testopen(char* p);

extern FILE *fpList;	// canldr.c File with paths and program names

int ldfilesopen_sw;	// 0 = .bin & .srec files have not been opened

/******************************************************************************
 * int cmd_p_init(char* p);
 * @brief 	: Reset 
 * @param	: p = pointer to line entered on keyboard
 * @return	: -1 = too few chars.  0 = OK.
*******************************************************************************/
static u32 linenumber;
int cmd_p_init(char* p)
{
	#define LINESIZE	256
	char list[LINESIZE];
	linenumber = 0;
	ldfilesopen_sw = 0;	// Show everybody the files have not been opened.

	/* First list out the available options */
	rewind(fpList);
	while ( (fgets (&list[0],LINESIZE,fpList)) != NULL)
	{
		linenumber += 1;
		printf("%3i %s",linenumber, list); 
	}
	rewind(fpList);
	printf("Enter a line number, or number beyond last number to enter path & name\n");
	return 0;
}
int cmd_p_init1(char* p)
{
	#define LINESIZE	256
	char list[LINESIZE];
	int j;

	sscanf(p, "%i",&j);
	if (j > linenumber)
	{ // Here, enter the path and name
		printf("Enter path and name \n");
		return 2;
	}
	// Here, select the line with the path and name
	rewind(fpList);	linenumber = 0;
	while ( (fgets (&list[0],LINESIZE,fpList)) != NULL)
	{
		linenumber += 1;
		if (j == linenumber)
		{
			printf("%3i %s",j, list); 
			break;
		}
	}		
	/* Test that the path and name will open the files (.bin & .srec) */
	if (testopen(&list[0]) != 0) {return 1;ldfilesopen_sw=0;}
	ldfilesopen_sw=1;	// Show files are open
	return 0;
}
int cmd_p_init2(char* p)
{
	if (testopen(p) != 0) {return 1;ldfilesopen_sw=0;}
	ldfilesopen_sw=1;	// Show files are open
	return 0;
}
/******************************************************************************
 * static int testopen(char* p);
 * @brief 	: 
*******************************************************************************/
static FILE *fpBin;
static FILE *fpSrec;


static int testopen(char* p)
{
	#define LINESIZE	256
	char c[LINESIZE];
	char* x;
	int r = 0;
	
	/* Check for .srec file */
	strcpy(c,p); x = strchr(c,'\n'); *x = 0;
	strcat(c,".srec");
//printf("testopen: %s\n",c);
	if ( (fpSrec = fopen (c,"r")) == NULL)
	{
		printf("File did not open: %s\n",c);
		r = 1;
	}	

	/* Check for .bin file */
	strcpy(c,p); x = strchr(c,'\n'); *x = 0;
	strcat(c,".bin");
//printf("testopen: %s\n",c);
	if ( (fpBin = fopen (c,"rb")) == NULL)
	{
		printf("File did not open: %s\n",c);
		r = 2;
	}
	if (r == 0)	
		printf("Both .bin and .srec files opened--we are good to go\n");
	else
		printf("Maybe the .bin and .srec files need to be built (./mm)\n");	

	return 0;
}


/******************************************************************************
 * void cmd_p_timeout(void);
 * @brief 	: 
*******************************************************************************/
void cmd_p_timeout(void)
{
	return;
}
/******************************************************************************
 * void cmd_p_do_msg(struct CANRCVBUF* p);
 * @brief 	: Output msgs for the id that was entered with the 'm' command
*******************************************************************************/
/*
This routine is entered each time a CAN msg is received, if command 'm' has been
turned on by the hapless Op typing 'm' as the first char and hitting return.
*/
void cmd_p_do_msg(struct CANRCVBUF* p)
{

	return;
}

