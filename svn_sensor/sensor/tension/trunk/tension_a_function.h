/******************************************************************************
* File Name          : tension_a_function.h
* Date First Issued  : 03/26/2015
* Board              : f103
* Description        : Tension function
*******************************************************************************/

#ifndef __TENSION_A_FUNCTION
#define __TENSION_A_FUNCTION

#include <stdint.h>
#include "common_misc.h"
#include "common_can.h"

 // -----------------------------------------------------------------------------------------------

/* **************************************************************************************/
int tension_a_function_init(void);
/* @brief	: Initialize
 * return	:  + = table size count of mismatch
 *		:  0 = OK
 *		: -1 = count is zero 
 * ************************************************************************************** */
void tension_a_function_poll(struct CANRCVBUF* pcan);
/* @brief	: Handle incoming CAN msgs
 * @param	; pcan = pointer to CAN msg buffer
 * @return	: Nothing for the nonce
 * ************************************************************************************** */

extern struct TENSIONLC ten_a;	// Tension parameters


#endif 

