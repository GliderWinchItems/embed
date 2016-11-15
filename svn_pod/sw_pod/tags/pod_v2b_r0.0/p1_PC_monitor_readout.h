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

#endif

