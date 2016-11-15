/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : p1_shutdown.h
* Generator          : deh
* Date First Issued  : 09/11/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Power down sequence
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PWRSHUTDOWN
#define __PWRSHUTDOWN


/******************************************************************************/
void p1_shutdown_normal_run(void);
/* @brief	: Sequence for shutting down from the normal_run mode
 ******************************************************************************/
void p1_shutdown_deepsleep_run(void);
/* @brief	: Sequence for shutting down from the deepsleep_run mode
 ******************************************************************************/
void p1_shutdown_timer_reset(void);
/* @brief	: Reset the timeout timer that puts unit into deepsleep mode
 ******************************************************************************/


extern int nDebug_shutdown;

#endif 
