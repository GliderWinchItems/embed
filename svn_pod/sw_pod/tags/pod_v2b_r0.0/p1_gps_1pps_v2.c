/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : p1_gps_1pps_v2.c
* Hackeroo           : deh
* Date First Issued  : 07/22/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Routines that use PC13->PE9 for measuring 1 pps gps vs 32 KHz osc
*******************************************************************************/
/*
10-17-2011: Modified for 'pod_v1' to allow calibration of 32 KHz xtal
  Count 8192 interrupts between storing and setting the flag.  The 32768 Hz osc
  is divided by 4 to give 8192 interrupts per second.  The difference
  between input captures 8192 interrupts apart gives the processor clock ticks in
  1 second of GPS time duration.


10-20-2011
This version simplifies the measurement of the 32 KHz osc via the MCO pin by 
eliminating the use of the overflow interrupt to extend the time to one second.
In this routine each interrupt, which is within the range of the counter, when
the bus is running at 24 MHz, has the difference between it and the previous
interrupt added to a sum.  With the MCO running at 32768/64 Hz, the time between
interrupts when summed over 512 interrupts constitutes one second.


NOTE:

TIM1 CH1*--

This assumes the RTC initialization has been completed (so that the RTC and Backup has
been powered, enabled, etc.

The board connects PC13 to PE9.  This allows PC13 to be driven from the RTC and cause
input captures with TIM1 CH1* (remapped).  

Comparison of the captured time is then used to measure the frequency difference between
the 32 KHz osc and the 1 pps GPS time pulse.

For retrieving the current timer counter in extended format, the high order (48) bits are stored
and the latest timer counter register loaded into the low order 16 bits.  The upper bits are
then compared against the latest upper bits to see if there was any overflow interrupt during
the foregoing instructions.  If so, the process is repeated.

*/

#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/gpio.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/bkp.h"
#include "libopenstm32/timer.h"

#include "p1_common.h"

#define TIM1INTERRUPTCOUNT	512	// Number of interrupts to count between stores

volatile unsigned int nTim1Debug0;
unsigned int nTim1Debug1;
unsigned int nTim1Debug2;
unsigned int nTim1Debug3;


/* Various bit lengths of the timer counters are handled with a union */
// Timer counter extended counts
static volatile union TIMCAPTURE64	strTim1cnt;	// 64 bit extended TIM1 CH1 timer count
// Input capture extended counts
static volatile union TIMCAPTURE64	strTim1;	// 64 bit extended TIM1 CH1 capture
static volatile union TIMCAPTURE64	strTim1m;	// 64 bit extended TIM1 CH1 capture (main)

/* The readings and flag counters are updated upon each capture interrupt */
static volatile unsigned int		uiTim1ch1_Flag;		// Incremented when a new capture interrupt serviced, TIM1CH1*


/******************************************************************************
 * void p1_GPS_1pps_init(void);
 * @brief	: Initialize PC13->PE9 RTC-to-timer connection
*******************************************************************************/
void p1_GPS_1pps_init (void)
{
	/* Setup the gpio pin for PE9 is not needed as reset default is "input" "floating" */

	/* Setup the gpio pin for PC13 as output (PE9 is input).  Board has a PC13<->PE9 traced that connects these two pins */
	GPIO_CRH(GPIOC) &= ~((0x000f ) << (4*5));	// Clear CNF reset bit 01 = Floating input (reset state)
	GPIO_CRH(GPIOC) |=  (( (GPIO_CNF_OUTPUT_ALTFN_PUSHPULL <<2) | (GPIO_MODE_OUTPUT_50_MHZ) ) << (4*5));	
	
	/* Output the RTC on PC13 (ANTI-TAMP) pin) */
	/* NOTE: ASOE bit can only be turned off by a backup domain reset (i.e. pull power plug!) */

//	BKP_RTCCR |= BKP_RTCCR_ASOE;		// Set RTC ALR to be output on tamper pin (PC13) (p 75)

	/* NOTE: this alternative puts the RTC "Secf" on PC13 (ANTI-TAMP) pin */
//	BKP_RTCCR |= BKP_RTCCR_ASOE | BKP_RTCCR_ASOS;			// Set RTC Alarm to be output on tamper pin (PC13) (p 75)

	/* NOTE: Alternative to above: CCO bit is turned when the Vdd power is removed */
	BKP_RTCCR |= BKP_RTCCR_CCO;			// RTC divided by 64 (512 Hz) on tamper pin

	/* ----------- TIM1 CH1* ------------------------------------------------------------------------*/
	/* Setup TIM1 CH1 (p 269 for beginning of section on TIM1,8 */
	/* Set up for input capture (p 352) */

	/* Enable bus clocking for TIM1 and alternate function */
	RCC_APB2ENR |= (RCC_APB2ENR_TIM1EN | RCC_APB2ENR_AFIOEN);		// (p 103) 

	/* Remap TIM1 */
	AFIO_MAPR |= AFIO_MAPR_TIM1_REMAP_FULL_REMAP; // (p 165, 170) (remap TIM1 to PE7-12)

	/* Set up capture/compare mode register 1 (p 379) with Capture/Compare 1 selection (p 381) */
	TIM1_CCR1 |= (0x01<<0);	// 01: CC1 channel is configured as input, IC1 is mapped on TI1. (p 380,1)

	/* TIM1&TIM8 capture/compare mode register 1  */
	TIM1_CCMR1 |= TIM_CCMR1_CC1S_IN_TI1;		// (p 322) 0x01: CC1 channel is configured as input, IC1 is mapped on TI1

	/* Compare/Capture Enable Reg (p 324,5) */
	//  Configured as input: rising edge trigger
	TIM1_CCER |= 0x01;				// Capture Enabled (p 324,5) 

	/* Control register 2 */
	// Default: The TIMx_CH1 pin is connected to TI1 input (p 311)

	/* Control register 1 */
	TIM1_CR1 |= TIM_CR1_CEN; 			// Counter enable: counter begins counting (p 310,1)

	/* Set and enable interrupt controller for TIM1 interrupt for Compare/Capture flags */
	NVICIPR (NVIC_TIM1_CC_IRQ, TIM1CC_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_TIM1_CC_IRQ);			// Enable interrupt controller for TIM1

	/* Set and enable interrupt controller for TIM1 interrupt for Update Event (counter overflow) */
	 //NOTE: Must be the same interrupt priority as "CC_IRQHandler"
//	NVICIPR (NVIC_TIM1_UP_IRQ, TIM1UP_PRIORITY );	// Set interrupt priority
//	NVICISER(NVIC_TIM1_UP_IRQ);			// Enable interrupt controller for TIM1

	/* Enable input capture interrupts */
	TIM1_DIER |= (TIM_DIER_CC1IE);	// Enable CH1 capture interrupt (p 315)

	return;


}

/*#######################################################################################
 * ISR routine for TIM1
 *####################################################################################### */
static int nTim1interruptctr;			// Count number of interrupts
static volatile unsigned int uiIC_Prev;		// Previous one second count of processor ticks
static volatile unsigned int uiTim1accum;	// Accumulate time from each input capture
volatile unsigned int uiTim1onesec;		// Latest count of processor ticks for one second

volatile unsigned int Debug_TIM1;

void p1_TIM1_CC_IRQHandler(void)
{
	unsigned short uiIC = TIM1_CCR1;		// Read the captured count which resets the capture flag

	uiTim1accum += ((uiIC - uiIC_Prev) & 0xffff);	// Add ticks between interrupts
	uiIC_Prev = uiIC;

	nTim1interruptctr += 1;
	if (nTim1interruptctr >= TIM1INTERRUPTCOUNT) 
	{
		nTim1interruptctr = 0;	// Reset interrupt counter			
		uiTim1onesec = uiTim1accum;	// Save accumulate time 
		uiTim1accum = 0;		// Reset accumulator
	}
	return;

}
/*#######################################################################################
 * ISR routine for TIM1
 *####################################################################################### */
void p1_TIM1_UP_IRQHandler(void)
{
while(1==1);
	return;
}
/*#######################################################################################
 * ISR routine for TIM1
 *####################################################################################### */

void p1_TIM1_BRK_IRQHandler(void)
{
while(1==1);
	return;
}
/*#######################################################################################
 * ISR routine for TIM1
 *####################################################################################### */

void TIM1_TRG_COM_IRQHandler(void)
{
while(1==1);
	return;
}



