/******************************************************************************
* File Name          : countdowntimer.c
* Date First Issued  : 11/04/2013
* Board              : Discovery F4
* Description        : Countdown timers
*******************************************************************************/
/* 
01/23/2014 Follinwing rev 196 TIM12 instead of TIM9


This implements countdown timers and switchdebouncers using a timer.

Two types of structs are used.  One holds the parameters for the timers.  The other
for switches.  The code here only handles the timers.  Two output captures are used.
One handles the timers.  The other will call a routine if a pointer is set, which
would be expected to be a pointer to 'switchdebounce.c' which handles the switches
that need debouncing.  That point is initialized when when 'debouncesw_init()' is
called.

Initialization sequence--
timer_debounce_init();

  followed by adding the timers and switches, e.g. the following adds three timers--
countdowntimer_add(&timer_blink_org_led);
countdowntimer_add(&timer_blink_red_led);
countdowntimer_add(&timer_printfmsg);

The arguments for '_add is a pointer to the struct with the values initialized for the
particular type of timer.  See 'countdowntimer.c' for examples.  Timers can be--

1) one-shot: When the count down reaches zero the .flag is incremented and nothing further happens
2) repetitive: a repetition count is set and each time the count reaches zero it reloads.
3) callback: in 1) and 2) above a subroutine address can be set in the struct and that subroutine
   will be called when the count reaches zero.
4) polling: the .flag in the struct can be checked in a polling loop to determine if the count has
   reached zero.  Whenever the count reaches zero the flag is incremented.
5) reloading: the .lod in the struct is used to reload a one-shot count.  This count can be reloaded
   at anytime.  The .lod count is normally zero.  When it is set with a non-zero value that value is
   picked up and loaded into the running counter upon the next timer interrupt.  The .lod value is
   then set to zero.

The timer structs are linked when the '_add routine is called.  The address of the "next" struct in 
the linked list is added.  Upon interrupt the timer and switch routines start with the last struct added, 
handling the counting, call back, reloading, flag, etc., then if the "next" struct pointer is not zero, 
continue until the last (going backwards) reaches the struct where the "next" pointer is zero (which is 
the first struct added.

The number of timers and switches that can be accommodated is limited by the memory and/or time to
process available.

The tick time is 1/2 ms.  This is set in the timer intialization routine.

Two output captures are used and these are offset so that the processing load for the timers and 
switches is split.

*/
#include "nvicdirect.h" 
#include "libopencm3/stm32/f4/gpio.h"
#include "libopencm3/stm32/f4/rcc.h"
#include "libopencm3/stm32/f4/timer.h"
#include "libopencm3/stm32/nvic.h"


#include "DISCpinconfig.h"
#include "countdowntimer.h"

void toggle_led (int lednum);

/* This holds the root (last added, but beginning) of the linked list of structs. */
static struct COUNTDOWNTIMER*  ptimhead  = 0;  // Ending struct in list has NULL

/* Subroutine prototypes */
static void do_timers(struct COUNTDOWNTIMER* p);

// This holds the address of the function that will be called
void 	(*timer_sw_ptr)(void) = 0;	// Function pointer for CH2 timer interrupt

static char init_sw = 0;

/* **************************************************************************************
 * int timer_debounce_init(void);
 * @brief	: Initialize timer and related
 * @param	:
 * @return	: 
 * ************************************************************************************** */
int timer_debounce_init(void)
{
	if (init_sw != 0) return 0;	// Init just once
	init_sw = 1;

	ptimhead = 0;	// JIC!

/* Usage-- CH1 = countdown timers; CH2 = pushbutton/switch debouncers.  Stagger the servicing
   using two channels so that the load under interrupt is spread between the two. */

	/* ----------- TIM12 ------------------------------------------------------------------------*/
	/* Enable bus clocking for TIM12 */
	RCC_APB1ENR |= (1<<6);		// (p 244) 

	/* This is based on 84 MHz AHPB2 bus rate */
	TIM12_PSC |= 420;	// Count clock freq = 84000000 / 420 -> 200,000 counts per sec
#define TIM12INCREMENT	100	// Produce output capture interrupts at 2,000 per sec. (1/2 ms per tick)

	/* Set the two channels so they are 1/2 way apart. */
	TIM12_CCR1 = 0;	
	TIM12_CCR2 = TIM12INCREMENT/2;	

	/* TIMx capture/compare mode register 1  */
	TIM12_CCMR1 = 0;

	/* Compare/Capture Enable Reg (p 324,5) */
	TIM12_CCER = 0;			 

	/* Set and enable interrupt controller for TIM12 interrupt */
	NVICIPR (NVIC_TIM8_BRK_TIM12_IRQ, TIM12_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_TIM8_BRK_TIM12_IRQ);			// Enable interrupt controller for TIM1
	
	/* Enable input capture interrupts */
	TIM12_DIER |= 0x06;	// Enable capture interrupt for CH1 and CH2 but not Update

	/* Control register 1 */
	TIM12_CR1 |= TIM_CR1_CEN; 	// Counter enable: counter starts counting.

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
	ptimhead = ptr;		// Timer becomes active when this instruction completes

	return;
}

/* ######################################################################################
TIM12 interrupt for CH1 and CH2
   ###################################################################################### */
void TIM8_BRK_TIM12_IRQHandler(void)
{
	__attribute__ ((unused))  u32 dummy;

	if (TIM12_SR & 0x2)	
	{ // Here CH1 OC
		TIM12_SR = ~0x2;	// Reset CH1 OC flag
		TIM12_CCR1 += TIM12INCREMENT;	// Next interrupt time
		do_timers(ptimhead);	// Yes, go check them.
	}

	if (TIM12_SR & 0x4)	
	{ // Here CH2 OC.  Generally for debouncing switches on the spi2/parallel-serial hardware
		TIM12_SR = ~0x4;	// Reset CH2 OC flag
		TIM12_CCR2 += TIM12INCREMENT;	// Next interrupt time
		/* Call other routines if an address is set up */
		if (timer_sw_ptr != 0)	// Do something with with chan2?
			(*timer_sw_ptr)();	// Call a function.
	}
	
	TIM12_SR = ~0x1;	// Reset event jic
	dummy = TIM12_SR; // Prevent tail chaining
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



