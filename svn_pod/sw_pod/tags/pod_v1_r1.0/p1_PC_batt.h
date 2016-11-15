/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : p1_PC_batt.h
* Author             : deh
* Date First Issued  : 09/10/2011
* Board              : STM32F103VxT6_pod_mm (USART1)
* Description        : Output current data to PC
*******************************************************************************/
/*


*/
#ifndef __P1_BATT_MONITOR
#define __P1_BATT_MONITOR

#define PCBATINCREMENT	5	// Tenth seconds between battery|temp monitoring outputs

/******************************************************************************/
void p1_PC_gps(void);
/* @brief 	: Output current data to PC
*******************************************************************************/

#endif

