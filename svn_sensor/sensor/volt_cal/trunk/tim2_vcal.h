/******************************************************************************
* File Name          : tim2_yog.h
* Date First Issued  : 09/28/2015
* Board              : f103
* Description        : Timer for voltage source oven heater 
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TIM2_VCAL
#define __TIM2_VCAL

#define TIM2_PRIORITY	0x50	// Timer interrupt priority

/******************************************************************************/
void tim2_vcal_init(void);
/* @brief	: Initialize TIM3 for PWM and other timing
*******************************************************************************/
void tim2_vcal_setpwm(uint16_t);
 /* @brief	: Set next PWM count 
 * @param	: timer tick count
*******************************************************************************/


extern void (*tim2_vcal_ptr)(void);	// Address of function to call at timer complete tick
extern void (*tim2_vcal_ll_ptr)(void);	// Low Level interrupt trigger function callback

#endif 
