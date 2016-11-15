/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : tim3_su.c
* Hackeroo           : deh
* Date First Issued  : 10/13/2012
* Board              : ../svn_sensor/hw/trunk/eagle/f103/CxT6
* Description        : Sensor Unit timer interrupt for clocking data measurement
*******************************************************************************/
/*
Strategy:
The timer counts up to the ARR register and interrupts.  The goal is to trigger a 
low priority level interrupt when the timer has counted 1/64th second of elapsed
time.  Since the timer counter is only 16 bits it requires counting multiple 
interrupts.

With a bus freq of 36E6 Hz and sample rate of 64 per sec the number of timer ticks
per 1/64th second is 562,500.  Splitting this count into 16 interrupts yields
35156 for each interrupt duration, plus 4 more.

Since the HSE osc freq varies, provision is made for adjusting the number of time ticks
per 1/64th second.  This is done by varying the count in the 16th interrupt, the
previous 15 interrupt durations being constant.


*/

#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/gpio.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/bkp.h"
#include "libopenstm32/timer.h"
#include "bit_banding.h"

#include "gps_1pps.h"

#include "interrupt_priority.h"
#include "tim3_su.h"

#define TIM3_NUMBITS_SUM	3	// Number of bits: in sum (for average of ticks in a second)
#define TIM3_NUMBITS_RATE	6	// Number of bits: number of sample periods per second
#define TIM3_NUMBITS_NUMINT	4	// Number of bits: interrupts per sample period (accommodate 16 bit counter limitation)

/* Derived */
#define TIM3_SAMPLERATE	(1<<TIM3_NUMBITS_RATE)		// Measurement rate, (samples per sec) (64)
#define TIM3_INT_CT	(1<<TIM3_NUMBITS_NUMINT)	// Number of interrupts per sample period (16)


/* 
If 15 interrupts are 35156, then the 16th can be shortened (by roughly 30000) or lengthened
(by roughly 30000).  The oscillator freq is within +/- 2% which would be +/- 11,250, so the
16th interrupt has sufficient lattitude to adjust for the frequency
*/

/* Interrupt counter: number of interrupts per 1/64th second */
volatile int 	tim3_isr_ctr = TIM3_INT_CT;		// 	

extern unsigned int pclk1_freq;	// ABP1 bus clock frequency, e.g. 24000000

/* The following is measured by the GPS via CAN and gets set every second. */
static unsigned int tim3_tickspersec;		// Default number of counter ticks for one second

static unsigned int tim3_running_sum;		// Carries running sum of fractional interrupt duration


void 		(*timtic_1_ptr)(void);	// Address of function to call at timer complete tick
/******************************************************************************
 * void tim3_su_init(void);
 * @brief	: Initialize TIM3 that produces interrupts used for timing measurements
*******************************************************************************/
void tim3_su_init(void)
{
	

	/* Initial number of ticks per second for the timer.  CAN sync'ing from CO will adjust this */
	tim3_tickspersec = pclk1_freq;

	/* ----------- TIM3 CH1* ------------------------------------------------------------------------*/
	/* Enable bus clocking for TIM3 (p 335 for beginning of section on TIM2-TIM7) */
	RCC_APB1ENR |= RCC_APB1ENR_TIM3EN;		// (p 105) 

	/* Enable bus clocking for alternate function */
	RCC_APB2ENR |= (RCC_APB2ENR_AFIOEN);		// (p 103) 


	/* Load initial time count p 386 */
	TIM3_ARR =  pclk1_freq/(TIM3_SAMPLERATE*TIM3_INT_CT);// Default count

	/* Buffer the reload register p 371 */
	TIM3_CR1 |= (1 << 7);	// Bit 7 ARPE: Auto-reload preload enable

	/* Control register 1 */
	TIM3_CR1 |= TIM_CR1_CEN; 			// Counter enable: counter begins counting (p 371,2)

	/* Set and enable interrupt controller for doing software interrupt */
	NVICIPR (NVIC_TIM6_IRQ, TIM6_IRQ_PRIORITY );	// Set interrupt priority ('../lib/libusartstm32/nvicdirect.h')
	NVICISER(NVIC_TIM6_IRQ);			// Enable interrupt controller for RTC ('../lib/libusartstm32/nvicdirect.h')

	/* Set and enable interrupt controller for TIM3 interrupt */
	NVICIPR (NVIC_TIM3_IRQ, TIM3_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_TIM3_IRQ);			// Enable interrupt controller for TIM1
	
	/* Enable input capture interrupts */
	TIM3_DIER |= (TIM_DIER_UIE);	// Enable CH1 capture interrupt and counter overflow (p 376,7)

	return;
}

/*#######################################################################################
 * ISR routine for TIM3
 *####################################################################################### */

void TIM3_IRQHandler(void)
{
	__attribute__((__unused__))int temp;

	if ( (TIM3_SR & 0x01) != 0)	// Check timer interrupt update bit
	{
		MMIO32_BIT_BAND(&TIM3_SR,0x00) = 0;	// Reset overflow flag
		tim3_isr_ctr += 1;
		switch (tim3_isr_ctr & (TIM3_INT_CT - 1))
		{			

		case 1:	// 16th interrupt is variable
			tim3_running_sum += tim3_tickspersec & ((1<<(TIM3_NUMBITS_RATE+TIM3_NUMBITS_SUM)) - 1);
			TIM3_ARR = (tim3_tickspersec+tim3_running_sum)>>(TIM3_NUMBITS_RATE+TIM3_NUMBITS_SUM);		
			tim3_running_sum &= ((1<<(TIM3_NUMBITS_RATE+TIM3_NUMBITS_SUM)) - 1);
			break;

		case 0:	// End of 16 interrupts

			/* Trigger a pending interrupt for TIM6, which will cause a chain of tick related routines to execute */
			NVICISPR(NVIC_TIM6_IRQ);	// Set pending (low priroity) interrupt for TIM6 ('../lib/libusartstm32/nvicdirect.h')

		default: // Here--not last, and not 16th.
			
			TIM3_ARR = tim3_tickspersec >> (TIM3_INT_CT+TIM3_NUMBITS_SUM+TIM3_NUMBITS_NUMINT);
		}

		temp = MMIO32_BIT_BAND(&TIM3_SR,0x00);	// Readback register bit to be sure it is cleared
			
	}
	return;

}
/*#######################################################################################
 * ISR routine for TIM6 (low priority level)
 *####################################################################################### */

void TIM6_IRQHandler(void)
{
/* This interrupt is caused by the RTC interrupt handler when further processing is required */
	/* Call other routines if an address is set up */
	if (timtic_1_ptr != 0)	// Having no address for the following is bad.
		(*timtic_1_ptr)();	// Go do something (e.g. poll the AD7799)	
	
	return;
}


