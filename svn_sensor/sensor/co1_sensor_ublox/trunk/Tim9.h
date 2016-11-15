/******************************************************************************
* File Name          : Tim9.h
* Date First Issued  : 05/11/2014
* Board              : Sensor
* Description        : Simple timing
*******************************************************************************/
#ifndef __TIM9
#define __TIM9


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

/* Runing tick count used by others */
extern uint32_t tim9_tick_ctr;	// 1/2 ms ticks
extern uint32_t tim9_tick_rate; // Ticks per millisecond

#endif 

