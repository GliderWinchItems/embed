/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : gps_shutdown.h
* Author             : deh
* Date First Issued  : 10/19/2012
* Board              : STM32F103VxT6_pod_mm (USART1)
* Description        : Shutdown gps based on load-cell acitivity
*******************************************************************************/
/*


*/
#ifndef __GPS_SHUTDOWN
#define __GPS_SHUTDOWN

/******************************************************************************/
void gps_shutdown_poll(void);
/* @brief 	: Check for activity and shutdown/power-up gps
*******************************************************************************/
void triggerGPSon(int x);
/* @brief 	: Set the timer and turn the GPS on
 * @param	: x = ON time duration in 
*******************************************************************************/
void gps_powerdown(void);
/* @brief 	: Turn off GPS power
*******************************************************************************/

#endif

