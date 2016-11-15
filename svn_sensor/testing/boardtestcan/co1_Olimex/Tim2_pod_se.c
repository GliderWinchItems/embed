/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : Tim2_pod_se.c
* Hackeroo           : deh
* Date First Issued  : 01/05/2013
* Board              : STM32F103VxT6_pod_mm
* Description        : Input capture with TIM2 CH1 (PA1) on POD board.
* Note               : Use 'gps_1pps.h' with this routine
*******************************************************************************/
/*
NOTE:
Ref Manual RM0008 Rev 14, p 350


'Tim4_pod_se.c' is driven by GPS 1PPS (PB6) which captures the clock.  The
difference between clock readings gives the ticks in one second.  The difference
between these and the base clock rate, e.g. 64,000,000 ('pclk1_freq') is averaged.
The average (times the number of points in the average, i.e the running sum) is
used to time the interrupt rate.

 interrupts--

To adjust for frequency error--

For maximum range adjustment, 
64,000,000 clock
64 CAN ticks per second
Therefore, 1,000,000 clock ticks per CAN msg tick
1,000,000 - 32768 = 967,232
The smallest number for the countdown register is--
967,232 /17 = 56,896
The 18th interrupt duration can give a total of--
x > 967,232 
x < 1,032,768
Or about a +/- 3% range for adjustment.

To synchronize to the GPS--





*/
#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/gpio.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/bkp.h"
#include "libopenstm32/timer.h"

#include "gps_1pps_se.h"
#include "bit_banding.h"
#include "pinconfig_all.h"

/* The following is based on a TIM2 clock of 64,000,000. */
#define TIM2_INT_CT	18	// Number interrupts in one second
#define ARR_0to17 	56896	// 58,896 = interrupts 0 - 17
#define ARR_LAST	32768	// 18th interrupt nominal

/* Interrupt counter: number of interrupts per 1/64th second */
volatile int 	tim2_isr_ctr = TIM2_INT_CT;		// 	

extern unsigned int pclk1_freq;	// ABP1 bus clock frequency, e.g. 24000000

/* Pointers to functions to be executed under a low priority interrupt */
void 	(*tim2end_ptr)(void) = 0;	// Address of function to call forced interrupt

/******************************************************************************
 * void Tim2_pod_init(void);
 * @brief	: Initialize Tim2 for input capture
*******************************************************************************/
void Tim2_pod_init(void)
{

	/* Setup the gpio pin for PA1 is not needed as reset default is "input" "floating" */

	/* ----------- TIM2 CH1  ------------------------------------------------------------------------*/
	/* Enable bus clocking for TIM2 (p 335 for beginning of section on TIM2-TIM7) */
	RCC_APB1ENR |= RCC_APB1ENR_TIM2EN;		// (p 105) 

	/* Enable bus clocking for alternate function */
	RCC_APB2ENR |= (RCC_APB2ENR_AFIOEN);		// (p 103) 

// Delay starting TIM3 counter
//volatile int i;
//for (i = 0; i < 1000; i++);	// 1000 results in 13060 count difference between the two timers


	/* TIMx capture/compare mode register 1  (p 250 fig 125, and 379,80) */
	TIM2_CCMR1 |= TIM_CCMR1_CC2S_IN_TI1;		// (p 379,380)CC1 channel is configured as input, IC1 is mapped on TI1

	/* Compare/Capture Enable Reg (p 324,5) */
	//  Configured as input: rising edge trigger
	TIM2_CCER |= TIM_CCER_CC2E; 	// (1<<4);				// Capture Enabled (p 384,5) 

	/* Control register 2 */
	// Default: The TIMx_CH2 pin is connected to TI1 input (p 372)

	/* Control register 1 */
	TIM2_CR1 |= TIM_CR1_CEN; 			// Counter enable: counter begins counting (p 371,2)

	NVICIPR (NVIC_I2C2_ER_IRQ, NVIC_I2C2_ER_IRQ_PRIORITY );	// Set interrupt priority ('../lib/libusartstm32/nvicdirect.h')
	NVICISER(NVIC_I2C2_ER_IRQ);			// Enable interrupt controller ('../lib/libusartstm32/nvicdirect.h')

	/* Set and enable interrupt controller for TIM2 interrupt */
	NVICIPR (NVIC_TIM2_IRQ, TIM2_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_TIM2_IRQ);			// Enable interrupt controller for TIM1
	
	/* Enable input capture interrupts */
//	TIM2_DIER |= (TIM_DIER_CC2IE | TIM_DIER_UIE);	// Enable CH2 capture interrupt and counter overflow (p 393)
	TIM2_DIER |= (TIM_DIER_UIE);	// Enable Update Interrupt, counter overflow (p 393)

	return;
}

/*#######################################################################################
 * ISR routine for TIM2
 *####################################################################################### */
void TIM2_IRQHandler(void)
{
	volatile unsigned int temp;

	if ( (TIM2_SR & 0x01) != 0 )	// Check timer interrupt update bit
	{
		TIM2_SR = ~0x1;		// Reset update flag
		tim2_isr_ctr += 1;			// Count interrupts
		if (tim2_isr_ctr >=  TIM2_INT_CT )
		{  // 18th interrupt.
			tim2_isr_ctr = 0;
			TIM2_ARR = ARR_0to17;	// Next 17 are the same

			/* Trigger a pending interrupt, which will cause a chain of related routines to execute */
			NVICISPR(NVIC_I2C2_ER_IRQ);	// Set pending (low priority) interrupt for TIM1 ('../lib/libusartstm32/nvicdirect.h')
		}
		else
		{ // Next to last interrupt for one second.  This next one is variable
			if (tim2_isr_ctr ==  (TIM2_INT_CT - 1) )
			{
				TIM2_ARR = ARR_LAST;
			}
			else
			{ // All other interrupts are the same duration.
				TIM2_ARR = ARR_0to17;
			}
		}
	}
	temp = TIM2_SR;	// Prevent tail-chaining.  Readback register bit to be sure it is cleared	
	return;
}
/*#######################################################################################
 * ISR routine for TIM2 end of second low priority level
 *####################################################################################### */
void I2C2_ER_IRQHandler(void)
{
/* This interrupt is caused by the CAN FIFO 1 (time sync message) interrupt for further processing at a low interrupt priority */
	/* Call other routines if an address is set up */
	if (tim2end_ptr != 0)	// Having no address for the following is bad.
		(*tim2end_ptr)();	// Go do something
	return;
}


