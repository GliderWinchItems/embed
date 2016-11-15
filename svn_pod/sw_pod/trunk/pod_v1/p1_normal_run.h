/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : p1_normal_run.h
* Hackeroos          : caw, deh
* Date First Issued  : 08/30/2011
* Board              : STM32F103VxT6_pod_mm (USART1)
* Description        : Main program for version implementation
*******************************************************************************/

#ifndef __P1_NORMAL_RUN
#define __P1_NORMAL_RUN

#define MODE_ACTIVE 	1	// In active mode with tension readings and logging
#define MODE_DEEPSLEEP	0	// Wakeup was only to check battery and adjust time


/******************************************************************************/
void p1_normal_run(void);
/* @brief 	: Get started with the sequence when reset is comes out of STANDBY
*******************************************************************************/

extern unsigned int uiDebug_normX;

#endif

