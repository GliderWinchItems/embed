/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : p1_PC_monitor_readout.h
* Author             : deh
* Date First Issued  : 10/06/2011
* Board              : STM32F103VxT6_pod_mm (USART1)
* Description        : Output packet data, and monitor 
*******************************************************************************/
/*


*/
#ifndef __P1_MONITOR_READOUT
#define __P1_MONITOR_READOUT


/******************************************************************************/
int p1_PC_monitor_readout(void);
/* @brief	: Output packets to PC
 * @return	: 0 = OK; 1 = all packets read
*******************************************************************************/
void p1_PC_monitor_readout_datetime(void);
/* @brief	: Display date/time and pause readback operation
******************************************************************************/
void p1_PC_monitor_readout_launchtimes(void);
/* @brief	: Do readout using launchtime entries as directed from PC connection
*****************************************************************************/
void p1_PC_monitor_readout_pushbuttontimes(void);
/* @brief	: Do readout using pushbuttontime entries as directed from PC connection
*****************************************************************************/
void p1_PC_monitor_readout_launchtimes_reset(void);
/* @brief	: Reset/initalize switches and stuff for 'p1_PC_monitor_readout_launchtimes()'
*****************************************************************************/
void p1_PC_monitor_readout_pushbuttontimes_reset(void);
/* @brief	: Reset/initalize switches and stuff for 'p1_PC_monitor_readout_pushbuttontimes()'
*****************************************************************************/
int convert_k_command (char * p);
/* @brief	: Convert 'k' command with sub command and count
 * @return	: 0 = OK; non-zero = not usable.
******************************************************************************/
int convert_q_command (char * p);
/* @brief	: Convert 'k' command with sub command and count
 * @return	: 0 = OK; non-zero = not usable.
******************************************************************************/

#endif

