/******************************************************************************
* File Name          : tim3_ten.h
* Date First Issued  : 04/04/2015
* Board              : f103
* Description        : Timer for polling functions in tension.c
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TIM3_TEN
#define __TIM3_TEN

#define TIM3_PRIORITY	0x50	// Timer interrupt priority

/******************************************************************************/
void tim3_ten_init(uint32_t t);
/* @brief	: Initialize TIM3 that produces interrupts used for timing measurements
 * @param	: t = number of APB1 bus ticks to count down
*******************************************************************************/


extern void (*tim3_ten_ptr)(void);	// Address of function to call at timer complete tick
extern void (*tim3_ten_ll_ptr)(void);	// Low level interrupt trigger function callback
extern unsigned int tim3_ticks;	// Running count of timer ticks
extern unsigned int tim3_rate ; // ticks per second

#endif 
