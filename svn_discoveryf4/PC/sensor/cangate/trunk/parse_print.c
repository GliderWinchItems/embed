/******************************************************************************
* File Name          : parse_print.c
* Date First Issued  : 12/26/2014
* Board              : PC
* Description        : Various print formats for data from parse.c
*******************************************************************************/


#include "gatecomm.h"
#include "PC_gateway_comm.h"	// Common to PC and STM32
#include "USB_PC_gateway.h"
#include "common_fixedaddress.h"
#include "common_highflash.h"
#include "ctype.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "parse.h"

/*******************************************************************************
 * static int cmpfuncHEX (const void * a, const void * b);
 * @brief 	: Compare function for => qsort <= (see man pages)--using pointer table
 * @return	: -1, 0, +1
*******************************************************************************/
static int cmpfuncHEX (const void * a, const void * b)
{
	const struct CANIDETAL *A = (struct CANIDETAL *)a;
	const struct CANIDETAL *B = (struct CANIDETAL *)b;
	struct CANIDETAL* pA = *(struct CANIDETAL **)A;
	struct CANIDETAL* pB = *(struct CANIDETAL **)B;

	long long aa = pA->canid;
	long long bb = pB->canid;

	if ((aa - bb) > 0 ) return 1;
	else
	{
		if ((aa - bb) < 0 ) return -1;
		return 0;
	}
}
/*******************************************************************************
 * static int cmpfuncNAME (const void * a, const void * b);
 * @brief 	: Compare function for => qsort <= (see man pages)--using pointer table
 * @return	: -1, 0, +1
*******************************************************************************/
static int cmpfuncNAME (const void * a, const void * b)
{
	const struct CANIDETAL *A = (struct CANIDETAL *)a;
	const struct CANIDETAL *B = (struct CANIDETAL *)b;
	struct CANIDETAL* pA = *(struct CANIDETAL **)A;
	struct CANIDETAL* pB = *(struct CANIDETAL **)B;

	int cmp = strcmp(&pA->name[0],&pB->name[0]);

	if (cmp > 0 ) return 1;
	else
	{
		if (cmp < 0 ) return -1;
		return 0;
	}
}
/*******************************************************************************
 * static int cmpfuncDESCRIPTION (const void * a, const void * b);
 * @brief 	: Compare function for => qsort <= (see man pages)--using pointer table
 * @return	: -1, 0, +1
*******************************************************************************/
static int cmpfuncDESCRIPTION (const void * a, const void * b)
{
	const struct CANIDETAL *A = (struct CANIDETAL *)a;
	const struct CANIDETAL *B = (struct CANIDETAL *)b;
	struct CANIDETAL* pA = *(struct CANIDETAL **)A;
	struct CANIDETAL* pB = *(struct CANIDETAL **)B;

	int cmp = strcmp(&pA->description[0],&pB->description[0]);

	if (cmp > 0 ) return 1;
	else
	{
		if (cmp < 0 ) return -1;
		return 0;
	}
}
/*******************************************************************************
 * void parse_print_columnheader(char* pc);
 * @brief 	: Print column head for listing
 * @param	: pc = pointer to string to print (this routine includes trailing '\n'
*******************************************************************************/
void parse_print_columnheader(char* pc)
{
	printf("%s\n",pc);
	printf("Columns:\n");
	printf("1) sequence number\n");
	printf("  |   2) Line number in input file\n");
	printf("  |    |  3) CAN ID name (gleaned from #define lines in input file)\n");
	printf("  |    |   | \t\t\t\t4) hex CAN ID assigned\n");
	printf("  |    |   | \t\t\t\t | \t   5) Usage reference count\n");
	printf("  |    |   | \t\t\t\t | \t    |  | 6) CAN ID description\n");
	return;
}
/*******************************************************************************
 * void parse_print_sort(struct CANIDETAL* p, int i);
 * @brief 	: Print one line of info from CANIDETAL
 * @param	: p = Pointer into struct array for struct to be printed
 * @para	: i = seq number
*******************************************************************************/
void parse_print_sort(struct CANIDETAL* p, int i)
{
	int j;
	j = (strlen(&p->name[0])+1)/8; 	// Adjust tabs for variable legth names
	if (j == 1) printf("%3i %4i %s\t\t\t0x%08X %2u @%s\n",i,p->linenumber+1,&p->name[0],p->canid,p->refct,&p->description[0]);
	if (j == 2) printf("%3i %4i %s\t\t0x%08X %2u @%s\n",i,p->linenumber+1,&p->name[0],p->canid,p->refct,&p->description[0]);
	if (j >= 3) printf("%3i %4i %s\t0x%08X %2u @%s\n",i,p->linenumber+1,&p->name[0],p->canid,p->refct,&p->description[0]);
	return;
}
/*******************************************************************************
 * static struct CANIDETAL* pointerarray_get(struct CANIDETAL pidlist[], int idsize);
 * @brief 	: Get memory space and initialize for sorting CANIDETAL struct array
 * @param	: pidlist = pointer to struct array to hold defined CAN ID list extracted from file
 * @param	: idsize = size of array
*******************************************************************************/
static struct CANIDETAL** pointerarray_get(struct CANIDETAL pidlist[], int idsize)
{
	/* Space for an array of pointers for the sort. */
	struct CANIDETAL **p = malloc(idsize * sizeof(struct CANIDETAL));
	if (p == NULL)
	{printf("ARGH! malloc failed in parse_print, pointerarray_get\n"); return NULL;}

	/* Fill array of pointers with pointer to elements in CAN ID array */
	int i;
	for (i = 0; i < idsize; i++)
		p[i] = &pidlist[i];
	return p;
}
/*******************************************************************************
 * void parse_list_by_fileorder(struct CANIDETAL pidlist[], int idsize);
 * @brief 	: List CAN ID table in original file order
 * @param	: pidlist = pointer to struct array to hold defined CAN ID list extracted from .txt file
 * @param	: idsize = size of array
*******************************************************************************/
void parse_list_by_fileorder(struct CANIDETAL pidlist[], int idsize)
{
	int i;
	for (i = 0; i < idsize; i++)
		parse_print_sort(&pidlist[i],i+1);
	return;
}
/*******************************************************************************
 * void parse_list_by_name(struct CANIDETAL pidlist[], int idsize);
 * @brief 	: List CAN ID table by CAN ID NAME
 * @param	: pidlist = pointer to struct array to hold defined CAN ID list extracted from .txt file
 * @param	: idsize = size of array
*******************************************************************************/
void parse_list_by_name(struct CANIDETAL pidlist[], int idsize)
{
	struct CANIDETAL** p = pointerarray_get(pidlist, idsize);
	if (p == NULL) {printf("ARGGHH! parse_list_by_name, pointer failed\n"); return;}

	int i;
	/* Sort by count of instances for each CAN ID. */
	qsort(p, idsize, sizeof(struct CANIDETAL*), cmpfuncNAME);

	for (i = 0; i < idsize; i++)
		parse_print_sort(p[i],i+1);

	free(p);
	return;
}
/*******************************************************************************
 * void parse_list_by_hex(struct CANIDETAL pidlist[], int idsize);
 * @brief 	: List CAN ID table by CAN ID HEX
 * @param	: pidlist = pointer to struct array to hold defined CAN ID list extracted from .txt file
 * @param	: idsize = size of array
*******************************************************************************/
void parse_list_by_hex(struct CANIDETAL pidlist[], int idsize)
{
	struct CANIDETAL** p = pointerarray_get(pidlist, idsize);
	if (p == NULL) {printf("ARGGHH! parse_list_by_hex, pointer failed\n"); return;}

	int i;
	/* Sort by count of instances for each CAN ID. */
	qsort(p, idsize, sizeof(struct CANIDETAL*), cmpfuncHEX);

	for (i = 0; i < idsize; i++)
		parse_print_sort(p[i],i+1);

	free(p);
	return;
}
/*******************************************************************************
 * void parse_list_by_description(struct CANIDETAL pidlist[], int idsize);
 * @brief 	: List CAN ID table by DESCRIPTION
 * @param	: pidlist = pointer to struct array to hold defined CAN ID list extracted from .txt file
 * @param	: idsize = size of array
*******************************************************************************/
void parse_list_by_description(struct CANIDETAL pidlist[], int idsize)
{
	struct CANIDETAL** p = pointerarray_get(pidlist, idsize);
	if (p == NULL) {printf("ARGGHH! parse_list_by_hex, pointer failed\n"); return;}

	int i;
	/* Sort by count of instances for each CAN ID. */
	qsort(p, idsize, sizeof(struct CANIDETAL*), cmpfuncDESCRIPTION);

	for (i = 0; i < idsize; i++)
		parse_print_sort(p[i],i+1);

	free(p);
	return;
}
/******************************************************************************
 * void parse_printf_sub(struct LDPROGSPEC *p, int i, int j);
 * @brief 	: List CAN IDs that have no references in parameter, calibration, CANID sections ("//i" lines)
 * @param	: p = pointer to struct array holding input file data
 * @param	: i = first column number
 * @param	: j = index in subsys array
*******************************************************************************/
void parse_printf_sub(struct LDPROGSPEC *p, int i, int j)
{
	int k;
	k = (strlen(&p->c_ldr.name[0])+1)/8; 	// Adjust tabs for variable legth names
	if (k == 1) printf("%3i %s\t\t\t%s\t%2u\t%s\t@%s\n",i,&p->c_ldr.name[0],&p->subsys[j].name[0],p->subsys[j].number,&p->subsys[j].structname[0],&p->subsys[j].description[0]);
	  if (k == 2) printf("%3i %s\t\t%s\t%2u\t%s\t@%s\n",i,&p->c_ldr.name[0],&p->subsys[j].name[0],p->subsys[j].number,&p->subsys[j].structname[0],&p->subsys[j].description[0]);
	    if (k >= 3) printf("%3i %s\t%s\t%2u\t%s\t@%s\n",i,&p->c_ldr.name[0],&p->subsys[j].name[0],p->subsys[j].number,&p->subsys[j].structname[0],&p->subsys[j].description[0]);
	return;
}

