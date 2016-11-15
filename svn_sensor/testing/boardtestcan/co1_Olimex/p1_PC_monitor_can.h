/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : pp1_PC_monitor_can.h
* Author             : deh
* Date First Issued  : 03/26/2013
* Board              : Olimex CO, (USART2)
* Description        : Output current CAN data to PC
*******************************************************************************/
/*


*/
#ifndef __PC_MONITOR_CAN
#define __PC_MONITOR_CAN

#define SIZEMCOMMAND	10	// Number of chars in 'm' command as received


/******************************************************************************/
void p1_PC_can_monitor_msgs(void);
/* @brief 	: Output current tension data to PC
*******************************************************************************/
void p1_PC_can_monitor_retrieve_id(void);
/* @brief 	: Output current tension data to PC for incoming CAN msgs that match id
*******************************************************************************/

extern u8 monitor_can_state;	// 'm' command progress 
extern u8 monitor_can_state_n;	// 'n' command progress

#endif

