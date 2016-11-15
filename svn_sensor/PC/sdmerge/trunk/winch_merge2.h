/******************************************************************************
* File Name          : winch_merge2.h
* Date First Issued  : 06/25/2014
* Board              : any
* Description        : Combine CAN msgs into one line
*******************************************************************************/
#ifndef __WINCH_MERGE
#define __WINCH_MERGE

#include "common.h"

/*******************************************************************************/
int winch_merge_gettable(FILE* fp);
/* @brief 	: Get flat file with CAN id versus many things
 * @param	: fp = input file
 * @return	:  0 = OK
 *		: -1 = error
*******************************************************************************/
int winch_merge_msg(char* pc);
/* @brief	: Main passes a CAN msg to this routine
 * @brief	: Lookup id, convert payload and place in output field array
 * @param	: pc = pointer to beginning of CAN msg
 * @param	: size = something?
 * @return	:  0 = OK
 *		: -1 = payload size out of range
 *		: -2 = CAN id not found
************************************************************************************************************* */
char* winch_merge_outputline(char* p, char* pmax);
/* @brief	: Build the winch output from the payload field strings 
 * @param	: p = pointer to output buffer
 * @param	: pmax = pointer to end+1 of whatever output buffer is provided
 * @return	: pointer to string terminator
************************************************************************************************************* */
void winch_merge_printtable(void);
/* @brief	: Nice listing of the cantable 
 ************************************************************************************************************* */
void winch_merge_printffields(void);
/* @brief	: Print payload field layout
************************************************************************************************************* */
void winch_merge_init(void);
/* @brief	: Reset things for next tick interval
************************************************************************************************************* */
void winch_merge_printsummary(void);
/* @brief 	: Summary of things
 * @param	: fp = input file
*******************************************************************************/
void winch_merge_print_id_counts(void);
/* @brief 	: List ids in cantable and the counts for each
*******************************************************************************/


#endif

