/******************************************************************************
* File Name          : countdowntimer.h
* Date First Issued  : 11/04/2013
* Board              : Discovery F4
* Description        : Countdown timers
*******************************************************************************/


#ifndef __COUNTDOWN_TIMERS
#define __COUNTDOWN_TIMERS

#define TIM12_PRIORITY	0xE0	// Interrupt priority

struct COUNTDOWNTIMER
{
	u32	ctr;			// Countdown counter
	u32	flag;			// Timeout flag, increments each time countdown goes to zero
	u32	rep;			// Countdown repetition
	u32	lod;			// Load value
	void 	(*func)(void);		// Callback pointer
	void*	pnext;	// Pointer to next struct
};


/* ************************************************************************************** */
int timer_debounce_init(void);
/* @brief	: Initialize timer and related
 * @param	:
 * @return	: 
 * ************************************************************************************** */
void countdowntimer_add(struct COUNTDOWNTIMER* ptr);
/* @brief	: Link timer struct to the countdown timer list
 * @param	: ptr = Pointer to struct with parameters to be used
 * @note	: No checking of the struct for valid values made--BE CAREFUL
 * ************************************************************************************** */

extern void 	(*timer_sw_ptr)(void);

#endif 

