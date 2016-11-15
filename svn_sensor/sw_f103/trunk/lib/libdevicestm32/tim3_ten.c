/******************************************************************************
* File Name          : tim3_ten.c
* Date First Issued  : 04/04/2015
* Board              : f103
* Description        : Timer for polling functions in tension.c
*******************************************************************************/
/*
This timer routine will trigger the same low level interrupt as the CAN receive
routine for not-top-priority CAN msgs ("void USB_LP_CAN_RX0_IRQHandler(void)", 
located in canwinch_pod_common_systick2048.c).

This trigger is necessary for the situation where there are no incoming CAN msgs
*/

#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/gpio.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/bkp.h"
#include "libopenstm32/timer.h"
#include "bit_banding.h"

#include "interrupt_priority.h"
#include "tim3_ten.h"
#include "common_can.h"

extern unsigned int pclk1_freq;	// ABP1 bus clock frequency, e.g. 24000000

void 	(*tim3_ten_ptr)(void) = NULL;	// Address of function to call at timer complete tick
void 	(*tim3_ten_ll_ptr)(void) = NULL;	// Low level function call

/* Reduce the rate for low level interrupt triggering. */
#define TIM3LLTHROTTLE	4 	// Trigger count
unsigned int throttlect = 0;

/* Running count of timer ticks. */					
unsigned int tim3_ticks;	// Running count of timer ticks
/******************************************************************************
 * void tim3_ten_init(uint32_t t);
 * @brief	: Initialize TIM3 that produces interrupts used for timing measurements
 * @param	: t = number of APB1 bus ticks to count down
*******************************************************************************/
void tim3_ten_init(uint32_t t)
{
	/* ----------- TIM3 ------------------------------------------------------------------------*/
	/* Enable bus clocking for TIM3 */
	RCC_APB1ENR |= RCC_APB1ENR_TIM3EN;		// (p 105) 

	/* Enable bus clocking for alternate function */
	RCC_APB2ENR |= (RCC_APB2ENR_AFIOEN);		// (p 103) 

	/* Load initial time count p 386 */
	TIM3_ARR =  t;// Default count

	/* Buffer the reload register p 371 */
	TIM3_CR1 |= (1 << 7);	// Bit 7 ARPE: Auto-reload preload enable

	/* Control register 1 */
	TIM3_CR1 |= TIM_CR1_CEN; 			// Counter enable: counter begins counting

	/* Set and enable interrupt controller for TIM3 interrupt */
	NVICIPR (NVIC_TIM3_IRQ, TIM3_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_TIM3_IRQ);			// Enable interrupt controller for TIM1
	
	/* Enable input capture interrupts */
	TIM3_DIER |= (TIM_DIER_UIE);	// Enable CH1 capture interrupt and counter overflow

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

		/* Running count of timer ticks. */					
		tim3_ticks += 1;

		/* Call other routines if an address is set up */
		if (tim3_ten_ptr != 0)	// Skip if ptr not set
			(*tim3_ten_ptr)();	// Go do something

		/* Trigger a low priority interrupt to poll for CAN msgs. */
		if (throttlect >= TIM3LLTHROTTLE) // Time to trigger a low level interrupt?
		{ // Yes.
			throttlect = 0;	// Reset
			NVICISPR(NVIC_I2C1_ER_IRQ);	// Set pending (low priority) interrupt ('../lib/libusartstm32/nvicdirect.h')
		}

		temp = MMIO32_BIT_BAND(&TIM3_SR,0x00);	// Readback register bit to be sure it is cleared			
	}
	return;

}
/*#######################################################################################
 * ISR routine for FIFO 0 low priority level
 *####################################################################################### */
void I2C1_ER_IRQHandler_ten(void)
{
	/* Call other routines if an address is set up */
	if (tim3_ten_ll_ptr != 0)	// Having no address for the following is bad.
		(*tim3_ten_ll_ptr)();	// Go do something
	return;
}
