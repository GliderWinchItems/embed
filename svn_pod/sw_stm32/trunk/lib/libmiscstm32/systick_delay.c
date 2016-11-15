/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : systick_delay.c
* Person             : deh
* Date First Issued  : 03/24/2012
* Description        : Convenience for using systick for delays
*******************************************************************************/
#include "../libmiscstm32/systick1.h"
#include "../libmiscstm32/systick1r.h"
/*******************************************************************************/

extern unsigned int sysclk_freq;

/* ************************************************************
* void delay_systick(unsigned int nDelay);
* @brief	: Delay loop based on systick timer
* @param	: arg = time delay in milliseconds
***************************************************************/
void delay_systick(unsigned int nDelay)
{
	unsigned int systickx;	// SYSTICK count at end of delay
	systickx = SYSTICK_getcount32() - (nDelay*(sysclk_freq/1000));	// Compute tick count to assure delay
	while ( ((int)(SYSTICK_getcount32()- systickx)) > 0);	// Time loop
	return;
}
/* ************************************************************
* unsigned int delay_systick_t0(unsigned int nDelay);
* @brief	: Get ending time for a delay based on systick timer
* @param	: Time in milliseconds
* @return	: systick time when time duration has elapsed
***************************************************************/
unsigned int delay_systick_t0(unsigned int nDelay)
{
	return SYSTICK_getcount32() - (nDelay*(sysclk_freq/1000));	// Compute tick count to assure delay
}
/* ************************************************************
* int delay_systick_check(unsigned int uiT0);
* @brief	: Check if systick time duration as expired
* @param	: systick time for end of delay duration
* @return	: zero or negative = Time expired; positive ticks remaining
***************************************************************/
unsigned int delay_systick_check(unsigned int uiT0)
{
	return ((int)(SYSTICK_getcount32()- uiT0));
}
