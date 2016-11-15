/******************************************************************************
* File Name          : Tim9.c
* Date First Issued  : 05/11/2014
* Board              : Sensor
* Description        : Countdown timers
*******************************************************************************/
/* 


*/
#include <stdint.h>
#include "nvicdirect.h" 
#include "gpio.h"
#include "rcc.h"
#include "timer.h"
#include "nvic.h"
#include "common_can.h"
#include "irq_priorities_co1.h"
#include "Tim9.h"


void toggle_led (int lednum);

/* These hold the root of the linked list of structs. */
static struct COUNTDOWNTIMER*  ptimhead  = 0;  // Last struct in list has NULL

/* Subroutine prototypes */
static void do_timers(struct COUNTDOWNTIMER* p);

/* Runing tick count used by others */
uint32_t tim9_tick_ctr = 0;	// 1/2 ms ticks
uint32_t tim9_tick_rate; 	// Ticks per second

// These hold the address of the function that will be called
void 	(*timer_sw_ptr)(void) = 0;	// Function pointer for CH2 timer interrupt

/* **************************************************************************************
 * int timer_debounce_init(void);
 * @brief	: Initialize timer and related
 * @param	:
 * @return	: 
 * ************************************************************************************** */
int timer_debounce_init(void)
{
	ptimhead = 0;
	
/* Usage-- CH1 = countdown timers; CH2 = pushbutton/switch debouncers.  Stagger the servicing
   using two channels so that the load under interrupt is spread between the two. */

	/* ----------- TIM9 ------------------------------------------------------------------------*/
	/* Enable bus clocking for TIM9 */
	RCC_APB2ENR |= RCC_APB2ENR_TIM9EN;		// (p 244) 

	/* This is based on 64 MHz AHPB2 bus rate */
	#define TIM9_PRESCALE 320
	#define TIM9INCREMENT	100	// Produce output capture interrupts at 2,000 per sec. (1/2 ms per tick)
	extern unsigned int pclk2_freq;	// APB2 freq
	TIM9_PSC |= TIM9_PRESCALE;	// Count clock freq = 64000000 / 320 -> 200,000 counts per sec
	tim9_tick_rate = pclk2_freq / (TIM9_PRESCALE * TIM9INCREMENT);

	/* Set the two channels so they are 1/2 way apart (do spi switches separate from direct wired ones) */
	TIM9_CCR1 = 0;	
	TIM9_CCR2 = TIM9INCREMENT/2;	

	/* TIMx capture/compare mode register 1  */
	TIM9_CCMR1 = 0;

	/* Compare/Capture Enable Reg (p 324,5) */
	TIM9_CCER = 0;			 

	/* Set and enable interrupt controller for TIM9 interrupt */
	NVICIPR (NVIC_TIM1_BRK_IRQ, TIM9_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_TIM1_BRK_IRQ);			// Enable interrupt controller for TIM1
	
	/* Enable input capture interrupts */
//	TIM9_DIER |= 0x06;	// Enable capture interrupt for CH1 and CH2 and not Update
	TIM9_DIER |= 0x02;	// Enable capture interrupt for CH1 and CH2 and not Update

	/* Control register 1 */
	TIM9_CR1 |= TIM_CR1_CEN; 			// Counter enable: counter begins counting

	return 0;
}
/* **************************************************************************************
 * void countdowntimer_add(struct COUNTDOWNTIMER* ptr);
 * @brief	: Link timer struct to the countdown timer list
 * @param	: ptr = Pointer to struct with parameters to be used
 * @note	: No checking of the struct for valid values made--BE CAREFUL
 * ************************************************************************************** */
void countdowntimer_add(struct COUNTDOWNTIMER* ptr)
{
	if (ptr->rep > 0)
	{ // Here, repetition count is specified
		ptr->ctr = ptr->rep;
	}
	else
	{
		ptr->ctr = 0;	// Be sure we start with a zero'ed counter.
	}

	ptr->flag = 0;	// Ctr transitioned to zero flag off.

	/* Add to list (the 1st one added has ptr->next = 0) */ 
	ptr->pnext = ptimhead;	// ptr->pnext pts to previous timer struct in chain
	ptimhead = ptr;		// Timer become active when ptimhead when instruction completes

	return;
}

/* ######################################################################################
 * TIM9 interrupt for CH1 and CH2
   ###################################################################################### */
void TIM1_BRK_TIM9_IRQHandler (void)
{
	__attribute__((__unused__))volatile u32 dummy;

	if (TIM9_SR & 0x2)	
	{ // Here CH1 OC
		TIM9_SR = ~0x2;	// Reset CH1 OC flagtim9_tick_ctr
		TIM9_CCR1 += TIM9INCREMENT;	// Next interrupt time
		do_timers(ptimhead);		// Yes, go check them.

		tim9_tick_ctr += 1;	// Running count of ticks

		if (timer_sw_ptr != 0)		
			(*timer_sw_ptr)();	// Call a function.
	}

//	if (TIM9_SR & 0x4)	
//	{ // Here CH2 OC.  Generally for debouncing switches on the spi2/parallel-serial hardware
//		TIM9_SR = ~0x4;	// Reset CH2 OC flag
//		TIM9_CCR2 += TIM9INCREMENT;	// Next interrupt time
//		/* Call other routines if an address is set up */
//		if (timer_sw_ptr != 0)	// Do something with with chan2?
//			(*timer_sw_ptr)();	// Call a function.
//	}
	
	TIM9_SR = ~0x1;	// Reset event jic
	dummy = TIM9_SR; // Prevent tail chaining
	return;
}
/* =====================================================================================
Timer counters update, and callback
   ===================================================================================== */
static void do_timers(struct COUNTDOWNTIMER* p)
{
	while (p != 0) // When pnext is null there are no more to check
	{
		if (p->ctr > 0)	// Still timing?
		{ // Here yes.
			p->ctr -= 1;	// Decrement the counter
			if (p->ctr == 0)
			{
				p->flag += 1;		// Show that the ct transitioned to zero
				/* Call other routines if an address is set up */
				if (p->func != 0)	// Skip if no address is setup
					(*p->func)();	// Go do something
				if (p->rep > 0) // Does user want to repeat?
				{ // Here, yes.
					p->ctr = p->rep;	// Set counter with repetition count
				}
			}
		}
		if (p->lod > 0) // Load a new count?
		{ // Here, a new countdown time is waiting to be loaded
			p->ctr = p->lod;	// Set count down timer with new count
			p->lod = 0;		// Reset load count
		}
		p = p->pnext;		// Step to next timer in list
	} 

	return;
}



