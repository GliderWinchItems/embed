/******************************************************************************
* File Name          : parse_print.h
* Date First Issued  : 12/26/2014
* Board              : PC
* Description        : Various print formats for data from parse.c
*******************************************************************************/

#ifndef __PARSEPRINTTXT
#define __PARSEPRINTTXT

#include "parse.h"
#include "winch_merge2.h"	// struct CANCAL, plus some subroutines of use


/*******************************************************************************/
 void printcanidlist(struct CANIDETAL pidlist[], int idx);
/*@brief 	: Print list of can id info
 * @param	: pidList= pointer to struct array with CAN ID info
 * @param	: idx = Number of entries in struct array
*******************************************************************************/
void parse_print_columnheader(char* pc);
/* @brief 	: Print column head for listing
 * @param	: pc = pointer to string to print (this routine includes trailing '\n'
*******************************************************************************/
 void parse_list_by_fileorder(struct CANIDETAL pidlist[], int idsize);
/* @brief 	: List CAN ID table in original file order
 * @param	: pidlist = pointer to struct array to hold defined CAN ID list extracted from .txt file
 * @param	: idsize = size of array
*******************************************************************************/
void parse_list_by_name(struct CANIDETAL pidlist[], int idsize);
/* @brief 	: List CAN ID table by CAN ID NAME
 * @param	: pidlist = pointer to struct array to hold defined CAN ID list extracted from .txt file
 * @param	: idsize = size of array
*******************************************************************************/
void parse_list_by_hex(struct CANIDETAL pidlist[], int idsize);
/* @brief 	: List CAN ID table by CAN ID HEX
 * @param	: pidlist = pointer to struct array to hold defined CAN ID list extracted from .txt file
 * @param	: idsize = size of array
*******************************************************************************/
void parse_list_by_description(struct CANIDETAL pidlist[], int idsize);
/* @brief 	: List CAN ID table by DESCRIPTION
 * @param	: pidlist = pointer to struct array to hold defined CAN ID list extracted from .txt file
 * @param	: idsize = size of array
*******************************************************************************/
void parse_print_sort(struct CANIDETAL* p, int i);
/* @brief 	: Print one line of info from CANIDETAL
 * @param	: p = Pointer into struct array for struct to be printed
 * @para	: i = seq number
*******************************************************************************/
void parse_printf_sub(struct LDPROGSPEC *p, int i, int j);
/* @brief 	: List CAN IDs that have no references in parameter, calibration, CANID sections ("//i" lines)
 * @param	: p = pointer to struct array holding input file data
 * @param	: i = first column number
 * @param	: j = index in subsys array
*******************************************************************************/


#endif
