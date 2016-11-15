/******************************************************************************
* File Name          : cmd_q.c
* Date First Issued  : 12/27/2014
* Board              : PC
* Description        : CAN bus loader file: edit-check only
*******************************************************************************/

/*
cd ~/svn_discoveryf4/PC/sensor/cangate/trunk
./mm && ./cangate 127.0.0.1 32123 ../test/testmsg2B.txt

*/

#include "cmd_q.h"
#include "gatecomm.h"
#include "parse.h"
#include "parse_print.h"
#include "winch_merge2.h"	// struct CANCAL, plus some subroutines of use

#define NOPRINTF	0	// parse.c: 0 = printf, not zero = don't printf

/* Subroutine declarations */
static void cmd_q_case1(int idsize, int ret);
static void cmd_q_by_fileorder(int idsize);
static void cmd_q_by_name(struct CANIDETAL pidlist[], int idsize);
static void cmd_q_by_hex(struct CANIDETAL pidlist[], int idsize);
static void cmd_q_by_description(struct CANIDETAL pidlist[], int idsize);
static void cmd_q_orphans(struct LDPROGSPEC* p, int idsize);

/* From cangate */
extern FILE *fpList;	// canldr.c File with paths and program names
extern int fdp;		// File descriptor for input file

/* List of program specs */
#define NUMPROGS	64	// Number of load specs
static struct LDPROGSPEC  ldp[NUMPROGS];
//static int loadpathfileIdx = 0; // Index in program number
//static int loadpathfileMax = 0; // Index+1 of last program number

/* List of CAN IDs extracted from input file. */
#define SIZECANID	512	// Max number array will hold
static struct CANIDETAL canidetal[SIZECANID];
static int idsize;	// Number stored in the foregoing array.


/******************************************************************************
 * static char cmd_q_menu(void);
 * @brief 	: 
 * @param	: 
 * @return	: char entered
*******************************************************************************/
static char cmd_q_menu(void)
{
	char c;
	char buf[64];
	int i = 0;

	while (1==1)
	{
		printf("1 - List: SUMMARY\n");
		printf("2 - List: CAN ID by ORDER IN INPUT FILE\n");		
		printf("3 - List: CAN ID by HAME\n");
		printf("4 - List: CAN ID by HEX\n");
		printf("5 - List: CAN ID by DESCRIPTION\n");
		printf("6 - List: CAN ID ORPHANS\n");
		printf("7 - List: Parameters, calibrations, CAN ID's used by subsystem\n");
		printf("8 - List: SUBSYSTEMS\n");
		printf("x - quit this command\n");
		printf("Enter code: ");
		i = 0;
		while (i == 0)
		i = read (STDIN_FILENO, buf, 64);	// Read one or more chars from keyboard
		c = buf[0];
		printf("The char that was entered by the Hapless Op was: %c\n",c);
		if ( ((c >= '1') && (c <= '8')) || (c == 'x') )return c;
	}
	return 0;
}
/******************************************************************************
 * int cmd_q_init(char *p);
 * @brief 	: Run edit check on file
 * @param	: p = pointer to line entered on keyboard
 * @return	: 0 = OK.
*******************************************************************************/
int cmd_q_init(char *p)
{
	int ret;
	char code;

	/* Build a list of CAN IDs */
	idsize = parse_buildcanidlist(fpList, canidetal, SIZECANID);
	if (idsize < 0){printf("EXIT: FILE CAN ID EXTRACTION ERROR when building CAN ID list from file: %d\n", idsize);return -1; }

	/* Get the list of path/file for the CAN bus unit programs */
	ret = parse(fpList, &ldp[0], NUMPROGS, canidetal,idsize,NOPRINTF);	// Read (or re-read) input file and build list	
	if ( ret < 0 ) {printf("EXIT: FILE PARSE ERROR when passing file the second time\n");return -1; }

	code = cmd_q_menu();
	switch (code)
	{
	case 'x': // Done with command
		printf("Hit x<enter> again for main menu\n");
		return 0;

	case '1': // List: Summary 
		cmd_q_case1(idsize,ret);
		break;

	case '2': // List: CAN ID by order in file
		cmd_q_by_fileorder(idsize);

	case '3': // List: CAN ID by name
		cmd_q_by_name(&canidetal[0],idsize);
		break;
	
	case '4': // List: CAN ID by hex
		cmd_q_by_hex(&canidetal[0],idsize);
		break;

	case '5': // List: CAN ID by description
		cmd_q_by_description(&canidetal[0],idsize);
		break;
	
	case '6': // List: CAN ID ORPHANS
		cmd_q_orphans(&ldp[0],idsize);
		break;

	case '7': // List: Parameters, calibrations, CAN ID's used by subsystem
		printf("NOT IMPLEMENTED\n");
		break;

	case '8': // List subsystems
		cmd_q_subsystems(&ldp[0], idsize);
		break;


	}
	

	return 0;
}

/******************************************************************************
 * static void cmd_q_case1(int idsize, int ret);
 * @brief 	: List summary
*******************************************************************************/
static void cmd_q_case1(int idsize, int ret)
{
	int i; 
	int skip_crc_ct = 0;
	int skip_unit_ct = 0;
	int skip_calib_ct = 0;
	struct LDPROGSPEC *p;

	printf("\nSUMMARY:\n");
	printf("Number of CAN ID's in #define: %4d\n", idsize);
	printf("Number of units defined      : %4d\n", ret);
	for (i = 0; i < idsize; i++)
	{
		p = &ldp[i];
		if (p->force != 0) skip_crc_ct += 1;	
		if (p->skip_calib != 0) skip_calib_ct += 1;	
		if (p->skip_unit != 0) skip_unit_ct += 1;	
	}
	printf("Number of units crc skip     : %4d\n", skip_crc_ct);
	printf("Number of units calib skip   : %4d\n", skip_calib_ct);
	printf("Number of units unit skip    : %4d\n", skip_unit_ct);
	printf("Number of units loaded       : %4d\n", (ret - skip_unit_ct) );
	return;
}
/******************************************************************************
 * static void cmd_q_by_fileorder(int idsize);
 * @brief 	: List CAN ID by order in file
*******************************************************************************/
static void cmd_q_by_fileorder(int idsize)
{
	parse_print_columnheader("\nCAN IDs in INPUT FILE ORDER");
	parse_list_by_fileorder(&canidetal[0], idsize);
	return;
}
/******************************************************************************
 * static void cmd_q_by_name(struct CANIDETAL pidlist[], int idsize);
 * @brief 	: List CAN ID table sorted by CAN ID NAME
 * @param	: pidlist = pointer to struct array to hold defined CAN ID list extracted from .txt file
 * @param	: idsize = size of array
*******************************************************************************/
static void cmd_q_by_name(struct CANIDETAL pidlist[], int idsize)
{
	parse_print_columnheader("\nCAN IDs sorted by NAME");	

	parse_list_by_name(&canidetal[0], idsize);

	return;
}
/******************************************************************************
 * static void cmd_q_by_hex(struct CANIDETAL pidlist[], int idsize);
 * @brief 	: List CAN ID table sorted by CAN ID HEX
 * @param	: pidlist = pointer to struct array to hold defined CAN ID list extracted from .txt file
 * @param	: idsize = size of array
*******************************************************************************/
static void cmd_q_by_hex(struct CANIDETAL pidlist[], int idsize)
{
	parse_print_columnheader("\nCAN IDs sorted by HEX VALUE");	

	parse_list_by_hex(&canidetal[0], idsize);

	return;
}
/******************************************************************************
 * static void cmd_q_by_description(struct CANIDETAL pidlist[], int idsize);
 * @brief 	: List CAN ID table sorted by CAN ID NAME
 * @param	: pidlist = pointer to struct array to hold defined CAN ID list extracted from .txt file
 * @param	: idsize = size of array
*******************************************************************************/
static void cmd_q_by_description(struct CANIDETAL pidlist[], int idsize)
{
	parse_print_columnheader("\nCAN IDs sorted by DESCRIPTION");	

	parse_list_by_description(&canidetal[0], idsize);

	return;
}
/******************************************************************************
 * static void cmd_q_orphans(struct LDPROGSPEC *p, int idsize);
 * @brief 	: List CAN IDs that have no references in parameter, calibration, CANID sections ("//i" lines)
 * @param	: p = pointer to struct array holding input file data
 * @param	: idsize = size of array
*******************************************************************************/
static void cmd_q_orphans(struct LDPROGSPEC *p, int idsize)
{
	int i,j;
	int ctr = 1;
	parse_print_columnheader("\nCAN ID that are ORPHANs");	
	for (i = 0; i < idsize; i++)
	{
		for (j = 0; j < ldp[i].idct; j++)
		{
			parse_print_sort(&ldp[i].c_app[j],ctr++);
		}
	}
	return;

}

/******************************************************************************
 * void cmd_q_subsystems(struct LDPROGSPEC *p, int idsize);
 * @brief 	: List subsystems
 * @param	: p = pointer to struct array holding input file data
 * @param	: idsize = size of array
*******************************************************************************/
void cmd_q_subsystems(struct LDPROGSPEC *p, int idsize)
{
	int i,j;
	int ctr = 1;

	printf("\nSUBSYSTEMS LIST\n");
	for (i = 0; i < idsize; i++)
	{
		for (j = 0; j < p[i].subsysnum; j++)
		{
			if (p->subsysnum > 0)
			{
				parse_printf_sub(&p[i],ctr++, j);
			}	
		}
	}

	return;
}
