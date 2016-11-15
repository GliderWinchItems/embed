/******************************************************************************
* File Name          : winch_merge2.h
* Date First Issued  : 06/25/2014
* Board              : any
* Description        : Combine CAN msgs into one line
*******************************************************************************/
#ifndef __WINCH_MERGE
#define __WINCH_MERGE

#include "libopencm3/cm3/common.h"

#define MAXCANPAYTYPESZ	3	// Number of payload arrangement types implemented
#define MAXCANIDFORMAT	32	// Max number of chars for format field
#define MAXCANFIELDDESCRPT	40	// Max number chars for the field description

/* Field layout and calibration of a reading */
struct CANCAL
{
	int	dlc;		// Size of payload for this payload instance
	int	ixbegin;	// Byte number of first byte of payload field
	int 	ixend;		// Byte number of last byte of payload field
	int	sign;		// 1 = unsigned; -1 = signed
	int	fld;		// Field position in output line (-1 = skip)
	int	fix;		// fix/float: 0 = int, 1 = double
	int	iscale;		// Same as above, but for fix = 0
	int 	ioffset;
	double	scale;		// Multiple payload value by this number
	double	offset;		// calibration = ((float)payload * scale) - offset;
	char	format[MAXCANIDFORMAT]; // Format for converting calibrated reading to ascii
	char	c[MAXCANFIELDDESCRPT];	// Field description
};

#define MAXCANCALFIELDS	8		// Max number of calibrations in one CAN msg
#define MAXCANDESCRIPTION	64	// Max number of chars in description field
#define	MAXCDEFINE	40		// Max size of #define name

struct CANIDTABLE
{
	unsigned int	id;		// ID for this CAN msg
	int	n;			// Number of fields loaded
	int	ct;			// Count of instances for this id
	struct CANCAL cal[MAXCANCALFIELDS]; // Payload field layout
	char c[MAXCANDESCRIPTION];	// Description of this CAN msg
	char cdef[MAXCDEFINE];		// #define name

};

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
int winch_merge_getformat(struct CANCAL *pt, char * p, int linect);
/* @brief	: printf error msg in format field
 * @param	: ret = error code
 * @param	: line = line number of input file
 * @return	:  0 = OK; greater
 * 		: -1 = 1st '"' not found
 *		: -2 = 2nd '"' not found
 *		: -3 = No chars in format
 *		: -4 = '%' not found
 *		: -5 = More than one '%'
 *		: -6 = Too many chars in format 
**************************************************************************************************************** */
int getcalibfield(struct CANCAL *pt, char * p, int linect);
/* @brief	: printf error msg in format field
 * @param	: ret = error code
 * @param	: line = line number of input file
 * @return	:  0 = OK;
 * @param	: -1 = fixed pt specified and wrong number of values
 * @param	: -2 = floating pt specified and wrong number of values
 * @param	: -3 = Something other than fixed|float in the 'fix' variable
 * @param	: -4 = Output field position has already been used
**************************************************************************************************************** */
int winch_merge_getfielddescription(struct CANCAL *pt, char * p, int linect);
/* @brief	: printf error msg in format field
 * @param	: ret = error code
 * @param	: line = line number of input file
 * @return	: 0 = OK; less than zero = very bad;
**************************************************************************************************************** */
int winch_merge_printfpay1errors(struct CANCAL *pt, int linect);
/* @brief	: printf error msg in format field
 * @param	: ret = error code
 * @param	: line = line number of input file
 * @return	: 0 = OK; greater than zero = very bad;
**************************************************************************************************************** */


#endif

