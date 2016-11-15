/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : rtctimers.h
* Hackeroos          : deh
* Date First Issued  : 07/08/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : ADC routines for pod
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RTCTIMERS
#define __RTCTIMERS

#define NUMBERRTCTIMERS	4			// Number of RTC timers

/******************************************************************************/
void rtctimers_countdown(void);
/*  @brief  countdown rtctimers
*******************************************************************************/

/* Array of timer counts (@7) */
/* NOTE: RTC countdown timer utilization --
0 - MAX232 timeout
1 - Active mode shutdown timeout
2 - GPS wait for good fix
3 - Pushbutton counter timeout (5 pushes within X secs shutsdown to deepsleep)
*/
extern volatile int 	nTimercounter[NUMBERRTCTIMERS];		// Count down RTC Secf Interrupt

/* Address to go to upon completion (@7) */
extern void 	(*rtc_timerdone_ptr)(void);		// Address of function to call for 'rtctimers_countdown' to go to for further handling

#endif 
