/******************************************************************************
* File Name          : tim4_yog.h
* Date First Issued  : 08/14/2015
* Board              : f103
* Description        : Timer for yogurt.c fan control
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TIM4_YOG
#define __TIM4_YOG

/******************************************************************************/
void tim4_yog_init(void);
/* @brief	: Initialize TIM3 for PWM and other timing
*******************************************************************************/
void tim4_yog_setpwm(uint16_t);
 /* @brief	: Set next PWM count 
 * @param	: timer tick count
*******************************************************************************/

#endif 
