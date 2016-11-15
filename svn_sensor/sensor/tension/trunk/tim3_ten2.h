/******************************************************************************
* File Name          : tim3_ten2.h
* Date First Issued  : 04/04/2015, 06/17/2016
* Board              : f103
* Description        : Timer for polling functions in tension.c w AD7799 zero calib
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TIM3_TEN2
#define __TIM3_TEN2

#define TIM3_PRIORITY	0x50	// Timer interrupt priority/* Reduce the rate for low level interrupt triggering. */
#define TIM3LLTHROTTLE	4 	// Trigger count

/******************************************************************************/
void tim3_ten2_init(uint16_t t);
/* @brief	: Initialize TIM3 that produces interrupts used for timing measurements
 * @param	: t = number of APB1 bus ticks to count down
*******************************************************************************/


extern void (*tim3_ten2_ptr)(void);	// Address of function to call at timer complete tick
extern void (*tim3_ten2_ll_ptr)(void);	// Low level interrupt trigger function callback
extern unsigned int tim3_ten2_ticks;	// Running count of timer ticks

#endif 
