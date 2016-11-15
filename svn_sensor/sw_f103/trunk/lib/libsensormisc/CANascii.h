/******************************************************************************
* File Name          : CANascii.h
* Date First Issued  : 11/17/2014
* Board              : STM32F103
* Description        : Send 'putc' chars to CAN msgs 
*******************************************************************************/
#include "../../../../svn_common/trunk/common_can.h"
#include "libmiscstm32/printf.h"
#include "../../../../svn_common/trunk/can_driver.h"

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAN_ASCII
#define __CAN_ASCII

/******************************************************************************/
void CANascii_init(void);
/* @brief	: Initialize
 ******************************************************************************/
void CANascii_poll(struct CANRCVBUF* pcan);
/* @brief	: Retreive CAN msgs from 
 * @param	: Pointer to CAN msg
 ******************************************************************************/
void CANascii_putc(char c);
/* @brief	: Put chars from putc into CAN msgs
 ******************************************************************************/
struct CANRCVBUF* CANascii_get(void);
/* @brief	: Get pointer to non-high priority CAN msg buffer
 * @return	: struct with pointer to buffer, ptr = zero if no new data
 ******************************************************************************/
/***** ######## UNDER INTERRUPT ###########  **********************************/
struct CANRCVBUF* CANascii_send(void);
/* @brief	: Enter from systickLOpriority3X_ptr pointer
 ******************************************************************************/

#endif


