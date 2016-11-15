/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : rtctimers.c
* Hackerees          : deh
* Date First Issued  : 07/08/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : RTC timers
*******************************************************************************/
/* Note--scheme for RTC (Secf) interrupt handling:
If 'rtc_secf_ptr' in ../devices/32KHz.c has been set with an address, the interrupt routine
then goes (call) to the address for further work.  The routine called can then do the same
thing, i.e. a chain of routines can be setup.

The general plan is that during the mainline initialization sequence the addres to 'rtctimers_countdown'
is setup for the 32KHz.c ISR routine.  Each interrupt the countdown timers are counted down if they
are not at zero.  On the tick that that reach zero a routine can be called if the address has been set.  

For a timer to be used, some routine sets a tick count in the timer, and then either loops waiting for the
count to reach zero, or continues after seting an address for a routine that will be called when the counter reaches.
*/

#include "rtctimers.h"


/* Address this routine will go to upon completion */
void 	(*rtc_timerdone_ptr)(void);		// Address of function to call for 'rtctimers_countdown' to go to for further handling

/* Array of addresses and timer counts */
void 	(*rtc_timer_ptr[NUMBERRTCTIMERS])(void);// Address of function to call during RTC_IRQHandler of SECF (Seconds flag)
volatile int 	nTimercounter[NUMBERRTCTIMERS];		// Count down RTC Secf Interrupt

/******************************************************************************
 * void rtctimers_countdown(void);
 *  @brief	: countdown rtctimers
*******************************************************************************/
void rtctimers_countdown(void)
{
	short i;
	for (i = 0; i < NUMBERRTCTIMERS; i++)
	{
		if (nTimercounter[i] > 0) 	// Countdown timer to zero
		{
			nTimercounter[i] -= 1;	
			if ( (nTimercounter[i] <= 0) && (rtc_timer_ptr[i] != 0) )// Time reached zero on this tick and we have an address of a subroutine
			(*rtc_timer_ptr[i])();	// Go do something 
		}
	}
	if (rtc_timerdone_ptr != 0)	// Having no address for the following is bad.
		(*rtc_timerdone_ptr)();	// Go do something (e.g. poll the AD7799)
	return;
}
