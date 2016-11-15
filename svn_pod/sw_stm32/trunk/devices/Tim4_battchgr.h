/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : Tim4_battchgr.h
* Author             : deh
* Date First Issued  : 01/24/2012
* Board              : STM32F103VxT6_pod_mm
* Description        : PWM using TIM4_CH1 (remapped)
* Note               : For use with 'linear_batt_chgr.c'
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TIM4BATTCHGR
#define __TIM4BATTCHGR

/* Note: The lower four bits of the priority byte are not used.
 The higher the priority number, the lower the priority */
#define TIM3_PRIORITY		0x40	// Interrupt priority for TIM4 interrupt



#endif
