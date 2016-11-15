/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : can_log.h
* Hackerees          : deh
* Date First Issued  : 01/24/2013
* Board              : Olimex co board
* Description        : Post CAN interrupt stuff
*******************************************************************************/
/* 

*/

#ifndef __CAN_LOG
#define __CAN_LOG

#include "common.h"	// Such as 'u32'
#include "common_can.h"	// CAN ID|mask layout and structs

/******************************************************************************/
void can_log_init(void);
/* @brief 	: A bit of initialization
*******************************************************************************/
void can_log_can(void);
/* @brief 	: Polling to write any available buffers to SD card
 * @return	: none
*******************************************************************************/
struct CANRCVSTAMPEDBUF* monitor_canbuf_get(void);
/* @brief 	: See if buffer with CAN msg is ready for sending to PC
 * @return	: pointer = 0 for data not ready, otherwise points buffer
*******************************************************************************/
u32 monitor_n_canbuf_get(void);
/* @brief 	: See if buffer with CAN id is ready for sending to PC
 * @return	: 0 = none, otherwise holds ID (ID == 0 not assigned)
*******************************************************************************/
void can_log_trigger_logging(void);
/* @brief 	: Set the low level interrupt to execute sd card write
*******************************************************************************/


/* Running count of CAN msgs written to SD card */
extern volatile u32 canlogct;
extern volatile u32 canlogctr;

/* Pointer to functions to be executed under a low priority interrupt */
extern void 	(*logpriority_ptr)(void);	// Address of function to call forced interrupt


#endif 



