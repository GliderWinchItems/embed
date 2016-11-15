/******************************************************************************
* File Name          : CAN_test_msgs.h
* Date First Issued  : 11/29/2013
* Board              : Discovery F4
* Description        : PC<->gateway--generate CAN format msgs for testing
*******************************************************************************/

#ifndef __CAN_TEST_MSGS
#define __CAN_TEST_MSGS

#include "common_misc.h"
#include "common_can.h"

/* **************************************************************************************/
void CAN_test_msg_init(void);
/* @brief	: Initialize time for generating time msgs
 * ************************************************************************************** */
struct CANRCVBUF* CAN_test_msg_PC(void);
/* @brief	: Generate a CAN test msg that goes to the PC
 * @return	:  0 = no msg 
 *  		:  not zero = pointer to buffer with msg
 * ************************************************************************************** */
struct CANRCVBUF* CAN_test_msg_CAN(void);
/* @brief	: Generate a CAN test msg that goes to the CAN bus
 * @return	:  0 = no msg 
 *  		:  not zero = pointer to buffer with msg
 * ************************************************************************************** */

#endif 

