/******************************************************************************
* File Name          : CAN_error_msgs.c
* Date First Issued  : 12/17/2013
* Board              : Discovery F4
* Description        : PC<->gateway--error msg monitoring
*******************************************************************************/
/*

*/
#include "CAN_error_msgs.h"


/* Error counters */

/*
USB_PC_get_msg_mode: return error codes--
ctr index	return code
0        	 -1 = completed, but bad checksum
1 		 -2 = completed, but too few bytes to be a valid CAN msg
2             	 -3 = bad mode selection code
3            	 -4 = 'read' returned an error
	 If message completion has no errors, but following compression does-- 
4		 -5 = Too few bytes to a valid 29b compressed msg
5		 -6 = dlc// payload ct too large (> 8) in a 29 bit id msg
6		 -7 = dlc doesn't match byte count in a 29 bit id msg
7		 -8 = Too few bytes to a valid 11b compressed msg
8		 -9 = dlc// payload ct too large (> 8) in a 11 bit id msg
9		 -10 = dlc doesn't match byte count in a 11 bit id msg

CAN_gateway_send: return error codes--
10		-1 = dlc greater than 8; 
11		-2 = illegal extended address

"One-off" errors
12		-1 = CAN msg (gatef) buffer overrun

*/
#define FIRSTGROUPSIZE	10
#define	SECONDGROUPSIZE	2
#define THIRDGROUPSIZE  1

/* Set an array of counters for the total of all the different error return codes. */
#define TOTALERRCTSIZE	(FIRSTGROUPSIZE +  SECONDGROUPSIZE + THIRDGROUPSIZE)	// Array with error count accumulators
u32 err_ctrs[TOTALERRCTSIZE];

/* **************************************************************************************
 * void Errors_USB_PC_get_msg_mode(int x);
 * @brief	: Count errors
 * @param	: Subroutine return from: USB_PC_get_msg_mode
 * ************************************************************************************** */
void Errors_USB_PC_get_msg_mode(int x)
{
	if ( x > 0 ) return; // Expect negative numbers
	x = 1 - x; // Make -1 to -n run 0 to + (n-1), i.e an index
	if (x >= FIRSTGROUPSIZE) return; // Assure it is in range.
	err_ctrs[x] += 1;	// Add to error count
	return;
}
/* **************************************************************************************
 * void Errors_CAN_gateway_send(int x);
 * @brief	: Count errors
 * @param	: Subroutine return from: USB_PC_get_msg_mode
 * ************************************************************************************** */
void Errors_CAN_gateway_send(int x)
{
	if ( x > 0 ) return; // Expect negative numbers
	x = 1 - x;
	if ( x >= SECONDGROUPSIZE)  return;
	err_ctrs[x + FIRSTGROUPSIZE] += 1;
	return;
}
/* **************************************************************************************
 * void Errors_misc(int x);
 * @brief	: Count errors
 * @param	: Subroutine return for "one-off" counts
 * ************************************************************************************** */
void Errors_misc(int x)
{
	if ( x > 0 ) return; // Expect negative numbers
	x = 1 - x;
	if ( x >= THIRDGROUPSIZE)  return;
	err_ctrs[x + (FIRSTGROUPSIZE + SECONDGROUPSIZE) - 1 ] += 1;
	return;
}
/* **************************************************************************************
 * struct CANERR2 Errors_get_count(void);
 * @brief	: Return next error count number and error count
 * @return	: array index; error count
 * ************************************************************************************** */
static int CAN_error_idx = 0;	// Index for 'get' of counters

struct CANERR2 Errors_get_count(void)
{
	struct CANERR2 err;
	err.idx = CAN_error_idx;
	err.ct  = err_ctrs[CAN_error_idx];
	CAN_error_idx += 1; if (CAN_error_idx >= TOTALERRCTSIZE) CAN_error_idx = 0;
	return err; // Return count, and step 
}


