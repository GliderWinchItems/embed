/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : p1_PC_accel.h
* Author             : deh
* Date First Issued  : 10/01/2011
* Board              : STM32F103VxT6_pod_mm (USART1)
* Description        : Output current accelerometer data to PC

*******************************************************************************/
/*


*/
#ifndef __P1_MONITOR_ACCEL
#define __P1_MONITOR_ACCEL

#define PCACCELINCREMENT	5	// Tenth seconds between monitoring outputs

/******************************************************************************/
void p1_PC_monitor_accel(void);
/* @brief 	: Output current data to PC
*******************************************************************************/

#endif

