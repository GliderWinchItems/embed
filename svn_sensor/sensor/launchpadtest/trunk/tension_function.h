/******************************************************************************
* File Name          : tension_a_function.h
* Date First Issued  : 03/26/2015
* Board              : f103
* Description        : Tension function
*******************************************************************************/

#ifndef __TENSION_A_FUNCTION_Q
#define __TENSION_A_FUNCTION_Q

#include <stdint.h>
#include "common_misc.h"
#include "../../../../svn_common/trunk/common_can.h"

 // -----------------------------------------------------------------------------------------------

/* **************************************************************************************/
void tension_a_poll_init(void);
/* @brief	: Initialize
 * ************************************************************************************** */
void tension_a_function_poll(struct CANRCVBUF* pcan);
/* @brief	: Handle incoming CAN msgs
 * @param	; pcan = pointer to CAN msg buffer
 * @return	: Nothing for the nonce
 * ************************************************************************************** */



// Tension parameters that go in SRAM
extern struct TENSIONLC ten_a;	// Tension parameters

/* "Consumer" tension struct pointer */
extern struct TENSIONLC* pCten_a;	// 

#endif 

