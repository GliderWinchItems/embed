/******************************************************************************
* File Name          : tim2_yog.c
* Date First Issued  : 09/28/2015
* Board              : f103
* Description        : Timer for voltage source oven heater 
*******************************************************************************/
/*


*/

#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/gpio.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/bkp.h"
#include "libopenstm32/timer.h"
#include "bit_banding.h"

#include "interrupt_priority.h"
#include "tim2_vcal.h"
#include "common_can.h"
#include "pinconfig_all.h"

#include "libmiscstm32/DTW_counter.h"

extern unsigned int pclk1_freq;	// ABP1 bus clock frequency, e.g. 24000000

void 	(*tim2_vcal_ptr)(void) = NULL;	// Address of function to call at timer complete tick
void 	(*tim2_vcal_ll_ptr)(void) = NULL;	// Low Level function call

/* Triac control pin is driven with 1/sec PWM frame. */
const struct PINCONFIGALL pin_FET = {(volatile u32 *)GPIOA,  2, OUT_AF_PP, MHZ_50};
/******************************************************************************
 * void tim2_vcal_init(void);
 * @brief	: Initialize TIM2 that produces interrupts used for timing measurements
*******************************************************************************/
void tim2_vcal_init(void)
{
	/* ----------- TIM2  ---------------------------------------------------------*/
	/* Enable bus clocking for TIM2 */
	RCC_APB1ENR |= RCC_APB1ENR_TIM2EN;		// (p 105) 

	/* Enable bus clocking for alternate function */
	RCC_APB2ENR |= (RCC_APB2ENR_AFIOEN);		// (p 103) 

	/* Configure timer output pin. */
	pinconfig_all( (struct PINCONFIGALL *)&pin_FET);

	/* Set a PWM frame of one second */
	TIM2_PSC =  999;	// divide by 1000 -> 1/64000 tick rate
	TIM2_ARR = 64000;	// PWM frame = 1 second

	/* Buffer the reload register p 371 */
	// Bit 7 ARPE: Auto-reload preload enable; Bit 0 CEN: counter enable
	TIM2_CR1 |= (1 << 7) | (1 << 0);

	/* Capture/compare mode register CH3 */
	//     PWM mode 1 (upcounting) | preload enable
	TIM2_CCMR2 = (0x6 << 4) | (1 << 3); //

	/* Cause an update to initialize shadow registers. */
	TIM2_EGR = (1 << 3) | (1 << 0);	// Set the UG CC3G bits

	/* TIMx capture/compare enable register (TIMx_CCER) */
	// NOTE: output pin drives FET that drives opto-isolator
	//            CC3P       CC3E
	TIM2_CCER = (0 << 9) | (1 << 8); // active high | output on pin


	/* Set and enable interrupt controller for TIM2 interrupt */
	NVICIPR (NVIC_TIM2_IRQ, TIM2_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_TIM2_IRQ);			// Enable interrupt controller for TIM1
	
	/* Enable input capture interrupts */
	TIM2_DIER |= (TIM_DIER_UIE);	// Enable timer update interrupt

	return;
}
/******************************************************************************
 * void tim2_vcal_setpwm(uint16_t t);
 * @brief	: Set next PWM count 
 * @param	: t = timer tick count (0-63999)
*******************************************************************************/
void tim2_vcal_setpwm(uint16_t t)
{
	TIM2_CCR3 = t;
	return;
}
/*#######################################################################################
 * ISR routine for TIM2
 *####################################################################################### */
#include "PODpinconfig.h"
#include "pinconfig_all.h"
#include "SENSORpinconfig.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/gpio.h"

#define Use_I2C2_ER_IRQ

void TIM2_IRQHandler_vcal(void)
{
	__attribute__((__unused__))int temp;

	if ( (TIM2_SR & 0x01) != 0)	// Check timer interrupt update bit
	{
//		MMIO32_BIT_BAND(&TIM2_SR,0x00) = 0;	// Reset overflow flag
		TIM2_SR &= ~((1 << 3) | (1 << 0));	// Reset CH3 and UG flags		
					
		/* Call other routines if an address is set up */
		if (tim2_vcal_ptr != 0)	// Having no address for the following is bad.
			(*tim2_vcal_ptr)();	// Go do something

		/* Trigger a low priority interrupt to poll for CAN msgs. */
#ifdef Use_I2C2_ER_IRQ
		NVICISPR(NVIC_I2C2_ER_IRQ);	// Set pending (low priority) interrupt ('../lib/libusartstm32/nvicdirect.h')
#endif

		if ((GPIO_IDR(GPIOE) & (1<<LED6)) == 0)
		{ // Here, LED OFF
			GPIO_BSRR(GPIOE) = (1<<LED6);	// Set bits = LED off
		}
		else
		{ // HEre, LED is ON
			GPIO_BRR(GPIOE) = (1<<LED6);	// Reset bits = LED on
		}

		temp = MMIO32_BIT_BAND(&TIM2_SR,0x00);	// Readback register bit to be sure it is cleared			
	}
	return;

}
/*#######################################################################################
 * ISR routine for FIFO 0 low priority level
 *####################################################################################### */
#ifdef Use_I2C2_ER_IRQ
void I2C2_ER_IRQHandler_vcal(void)
{
	/* Call other routines if an address is set up */
	if (tim2_vcal_ll_ptr != 0)	// Having no address for the following is bad.
		(*tim2_vcal_ll_ptr)();	// Go do something
	return;
}
#endif
