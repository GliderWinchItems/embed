/******************************************************************************
* File Name          : tim3_yog.c
* Date First Issued  : 08/06/2015
* Board              : f103
* Description        : Timer for yogurt.c
*******************************************************************************/
/*
This timer runs the PWM for the triac control and sets timing for counting
seconds, minutes, hours.

PB0 CH3 triac control

*/

#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/gpio.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/bkp.h"
#include "libopenstm32/timer.h"
#include "bit_banding.h"

#include "interrupt_priority.h"
#include "tim3_yog.h"
#include "common_can.h"
#include "pinconfig_all.h"

#include "libmiscstm32/DTW_counter.h"

extern unsigned int pclk1_freq;	// ABP1 bus clock frequency, e.g. 24000000

void 	(*tim3_yog_ptr)(void) = NULL;	// Address of function to call at timer complete tick
void 	(*tim3_yog_ll_ptr)(void) = NULL;	// Low Level function call

/* Triac control pin is driven with 1/sec PWM frame. */
const struct PINCONFIGALL pin_Triac = {(volatile u32 *)GPIOB,  0, OUT_AF_PP, MHZ_50};
/******************************************************************************
 * void tim3_yog_init(void);
 * @brief	: Initialize TIM3 that produces interrupts used for timing measurements
*******************************************************************************/
void tim3_yog_init(void)
{
	/* ----------- TIM3  ---------------------------------------------------------*/
	/* Enable bus clocking for TIM3 */
	RCC_APB1ENR |= RCC_APB1ENR_TIM3EN;		// (p 105) 

	/* Enable bus clocking for alternate function */
	RCC_APB2ENR |= (RCC_APB2ENR_AFIOEN);		// (p 103) 

	/* Configure timer output pin. */
	pinconfig_all( (struct PINCONFIGALL *)&pin_Triac);

	/* Set a PWM frame of one second */
	TIM3_PSC =  999;	// divide by 1000 -> 1/64000 tick rate
	TIM3_ARR = 64000;	// PWM frame = 1 second

	/* Buffer the reload register p 371 */
	// Bit 7 ARPE: Auto-reload preload enable; Bit 0 CEN: counter enable
	TIM3_CR1 |= (1 << 7) | (1 << 0);

	/* Capture/compare mode register CH3 */
	//     PWM mode 1 (upcounting) | preload enable
	TIM3_CCMR2 = (0x6 << 4) | (1 << 3); //

	/* Cause an update to initialize shadow registers. */
	TIM3_EGR = (1 << 3) | (1 << 0);	// Set the UG CC3G bits

	/* TIMx capture/compare enable register (TIMx_CCER) */
	// NOTE: output pin drives FET that drives opto-isolator
	//            CC3P       CC3E
	TIM3_CCER = (0 << 9) | (1 << 8); // active high | output on pin

	/* Set and enable interrupt controller for TIM3 interrupt */
	NVICIPR (NVIC_TIM3_IRQ, TIM3_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_TIM3_IRQ);			// Enable interrupt controller for TIM1
	
	/* Enable input capture interrupts */
	TIM3_DIER |= (TIM_DIER_UIE);	// Enable timer update interrupt

	return;
}
/******************************************************************************
 * void tim3_yog_setpwm(uint16_t t);
 * @brief	: Set next PWM count 
 * @param	: t = timer tick count (0-63999)
*******************************************************************************/
void tim3_yog_setpwm(uint16_t t)
{
	TIM3_CCR3 = t;
	return;
}
/*#######################################################################################
 * ISR routine for TIM3
 *####################################################################################### */
void TIM3_IRQHandler_yog(void)
{
	__attribute__((__unused__))int temp;

	if ( (TIM3_SR & 0x01) != 0)	// Check timer interrupt update bit
	{
//		MMIO32_BIT_BAND(&TIM3_SR,0x00) = 0;	// Reset overflow flag
		TIM3_SR &= ~((1 << 3) | (1 << 0));	// Reset CH3 and UG flags		
					
		/* Call other routines if an address is set up */
		if (tim3_yog_ptr != 0)	// Having no address for the following is bad.
			(*tim3_yog_ptr)();	// Go do something

		/* Trigger a low priority interrupt to poll for CAN msgs. */
		NVICISPR(NVIC_I2C1_ER_IRQ);	// Set pending (low priority) interrupt ('../lib/libusartstm32/nvicdirect.h')

		temp = MMIO32_BIT_BAND(&TIM3_SR,0x00);	// Readback register bit to be sure it is cleared			
	}
	return;

}
/*#######################################################################################
 * ISR routine for FIFO 0 low priority level
 *####################################################################################### */
void I2C1_ER_IRQHandler_yog(void)
{
	/* Call other routines if an address is set up */
	if (tim3_yog_ll_ptr != 0)	// Having no address for the following is bad.
		(*tim3_yog_ll_ptr)();	// Go do something
	return;
}
