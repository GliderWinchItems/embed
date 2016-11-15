/******************************************************************************
* File Name          : tim3_yog.h
* Date First Issued  : 08/06/2015
* Board              : f103
* Description        : Timer for yogurt.c
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TIM3_YOG
#define __TIM3_YOG

#define TIM3_PRIORITY	0x50	// Timer interrupt priority

/******************************************************************************/
void tim3_yog_init(void);
/* @brief	: Initialize TIM3 for PWM and other timing
*******************************************************************************/
void tim3_yog_setpwm(uint16_t);
 /* @brief	: Set next PWM count 
 * @param	: timer tick count
*******************************************************************************/


extern void (*tim3_yog_ptr)(void);	// Address of function to call at timer complete tick
extern void (*tim3_yog_ll_ptr)(void);	// Low Level interrupt trigger function callback

#endif 
