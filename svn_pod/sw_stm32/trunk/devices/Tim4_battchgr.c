/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : Tim4_battchgr.c
* Author             : deh
* Date First Issued  : 01/24/2012
* Board              : STM32F103VxT6_pod_mm
* Description        : PWM using TIM4_CH1 (remapped)
* Note               : For use with 'linear_batt_chgr.c'
*******************************************************************************/
/*
NOTE:


*/
#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/gpio.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/bkp.h"
#include "libopenstm32/timer.h"

#include "PODpinconfig.h"	// gpio configuration for board

#include "bit_banding.h"





/******************************************************************************
 * void Tim4_battchgr_init(u16 ARRcount);
 * @brief	: Initialize Tim4_CH1* for PWM output
 * @param	: ARRcount: clock tick count (sets frequency of PWM)
*******************************************************************************/
void Tim4_battchgr_init(void)
{
	/* Setup the gpio pin--PD12 */
	GPIO_BRR(GPIOA)  = (1<<12);			// Be sure bit is reset before making it an output (jic)
	configure_pin ((volatile u32 *)GPIOD, 12);	// Configure for pushpull output

	/* ----------- PD12 Encoder_1 chan B:TIM4_CH1 (remapped) (USART3_RTS*)  ----------------------*/
	/* Enable bus clocking for TIM4 (p 335 for beginning of section on TIM2-TIM7) */
	RCC_APB1ENR |= RCC_APB1ENR_TIM4EN;		// (p 105) 

	/* Enable bus clocking for alternate function */
	RCC_APB2ENR |= (RCC_APB2ENR_AFIOEN);		// (p 103) 

	/* Remap TIM4_CH1 from PB6 to PD12 */
	AFIO_MAPR |= AFIO_MAPR_TIM4_REMAP;	// p 164 TIM4 remapping. p 170

/* p 355
The PWM mode can be selected independently on each channel (one PWM per OCx
output) by writing 110 (PWM mode 1) or ‘111 (PWM mode 2) in the OCxM bits in the
TIMx_CCMRx register. You must enable the corresponding preload register by setting the
OCxPE bit in the TIMx_CCMRx register, and eventually the auto-reload preload register (in
upcounting or center-aligned modes) by setting the ARPE bit in the TIMx_CR1 register.
*/

/* 355
Pulse width modulation mode allows you to generate a signal with a frequency determined
by the value of the TIMx_ARR register and a duty cycle determined by the value of the
TIMx_CCRx register.
*/


	/* Control register 1 */
	//            ARPE       CMS (2)   
	TIM4_CR1 |= (1 << 7) | (0x2 << 5) ;
	
	/* Set auto-reload register (ARR), p 355, 386 */
	TIM4_ARR = ARRcount;	// Set number of ticks in PWM

	TIM1_CCR1 = 0; 

	/* Set and enable interrupt controller for TIM2 interrupt */
	NVICIPR (NVIC_TIM4_IRQ, TIM4_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_TIM4_IRQ);			// Enable interrupt controller for TIM1
	

	return;
}

/*#######################################################################################
 * ISR routine for TIM4
 *####################################################################################### */
void TIM4_IRQHandler(void)
{

	if ( (TIM4_SR & TIM_SR_CC1IF) ! = 0 )
	{
	}


	return;
}

