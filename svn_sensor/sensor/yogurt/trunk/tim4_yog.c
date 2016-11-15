/******************************************************************************
* File Name          : tim4_yog.c
* Date First Issued  : 08/14/2015
* Board              : f103
* Description        : Timer for yogurt.c fan control
*******************************************************************************/
/*
This timer runs the PWM for the fan voltage (speed) control. 
Frame rate 100 usec.

Routine does not use interrupts.

PB8 CH3 cooling fan control
*/

#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/gpio.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/bkp.h"
#include "libopenstm32/timer.h"
#include "bit_banding.h"

#include "interrupt_priority.h"
#include "tim4_yog.h"
#include "common_can.h"
#include "pinconfig_all.h"

extern unsigned int pclk1_freq;	// ABP1 bus clock frequency, e.g. 24000000

void 	(*tim4_yog_ptr)(void) = NULL;	// Address of function to call at timer complete tick
void 	(*tim4_yog_ll_ptr)(void) = NULL;	// Low Level function call

/* Fan speed control. */
const struct PINCONFIGALL pin_Fan = {(volatile u32 *)GPIOB,  8, OUT_AF_PP, MHZ_50};
/******************************************************************************
 * void tim4_yog_init(void);
 * @brief	: Initialize TIM4 that produces interrupts used for timing measurements
*******************************************************************************/
void tim4_yog_init(void)
{
	/* ----------- TIM4  ---------------------------------------------------------*/
	/* Enable bus clocking for TIM4 */
	RCC_APB1ENR |= RCC_APB1ENR_TIM4EN;

	/* Enable bus clocking for alternate function */
	RCC_APB2ENR |= (RCC_APB2ENR_AFIOEN);

	/* Configure timer output pin. */
	pinconfig_all( (struct PINCONFIGALL *)&pin_Fan);

	/* Set a PWM frame rate. */
	TIM4_PSC =  0;	// divide by 1 -> 1/64E6 system rate
	TIM4_ARR = 64000-1;	// PWM frame = 1000 usec

	/* Buffer the reload register. */
	// Bit 7 ARPE: Auto-reload preload enable; Bit 0 CEN: counter enable
	TIM4_CR1 |= (1 << 7) | (1 << 0);

	/* Capture/compare mode register CH3 */
	//     PWM mode 1 (upcounting) | preload enable
	TIM4_CCMR2 = (0x6 << 4) | (1 << 3); //

	/* Cause an update to initialize shadow registers. */
	TIM4_EGR = (1 << 3) | (1 << 0);	// Set the UG CC3G bits

	/* TIMx capture/compare enable register (TIMx_CCER) */
	//NOTE: output pin drives FET
	//            CC3P       CC3E
	TIM4_CCER = (0 << 9) | (1 << 8); // active low | output on pin

	return;
}/******************************************************************************
 * void tim4_yog_setpwm(uint16_t t);
 * @brief	: Set next PWM count 
 * @param	: t = timer tick count (0-6399)
*******************************************************************************/
void tim4_yog_setpwm(uint16_t t)
{
	TIM4_CCR3 = t;
	return;
}


