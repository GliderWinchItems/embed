/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : systick_delay.c
* Person             : deh
* Date First Issued  : 03/24/2012
* Description        : Convenience for using systick for delays
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SYSTICK_DELAY
#define __SYSTICK_DELAY

/* *************************************************************/
void delay_systick(unsigned int nDelay);
/* @brief	: Delay loop based on systick timer
 * @param	: arg = time delay in milliseconds
***************************************************************/
unsigned int delay_systick_t0(unsigned int nDelay);
/* @brief	: Get ending time for a delay based on systick timer
 * @param	: Time in milliseconds
 * @return	: systick time when time duration has elapsed
***************************************************************/
int delay_systick_check(unsigned int uiT0);
/* @brief	: Check if systick time duration as expired
 * @param	: systick time for end of delay duration
 * @return	: zero or negative = Time expired; positive ticks remaining
***************************************************************/

#endif

