/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : p1_gps_1pps.c
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

#include "bit_banding.h"

#include "p1_common.h"
#include "pinconfig_all.h"

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

/* TIM1 CH1 pin configuration */
const struct PINCONFIGALL pa8  = {(volatile u32 *)GPIOA, 8, OUT_AF_PP, MHZ_50};

/******************************************************************************
 * void p1_GPS_1pps_init(void);
 * @brief	: Initialize PC13->PE9 RTC-to-timer connection
*******************************************************************************/
void p1_GPS_1pps_init (void)
{
	/*  Setup TIM1 CH1: PA8 for alternate function push/pull output p 156, p 163  */
	pinconfig_all((struct PINCONFIGALL *)&pa8);	

	/* ----------- TIM1 CH1* ------------------------------------------------------------------------*/
	/* Setup TIM1 CH1 (p 269 for beginning of section on TIM1,8 */
	/* Set up for input capture (p 352) */

	/* Enable bus clocking for TIM1 and alternate function */
	RCC_APB2ENR |= (RCC_APB2ENR_TIM1EN | RCC_APB2ENR_AFIOEN);		// (p 103) 

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
	NVICIPR (NVIC_TIM1_UP_IRQ, TIM1UP_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_TIM1_UP_IRQ);			// Enable interrupt controller for TIM1

	/* Enable input capture interrupts */
	TIM1_DIER |= (TIM_DIER_CC1IE | TIM_DIER_UIE);	// Enable CH1 capture interrupt and counter overflow (p 315)

	return;


}
/******************************************************************************
 * unsigned p1_long long Tim1_gettime_ll(void);
 * @brief	: Retrieve the extended timer counter count
 * @return	: Current timer count as an unsigned long long
*******************************************************************************/
unsigned long long p1_Tim1_gettime_ll(void)
{
	union TIMCAPTURE64 strX;
	strTim1cnt.us[0] = TIM1_CNT;	// (p 327) Get current counter value
	/* This 'do' takes care of the case where the counter turns over during the execution */
	do
	{ // Loop if the overflow count changed since the beginning
		strX = strTim1cnt;		// Get current extended count
		strTim1cnt.us[0] = TIM1_CNT;	// (p 327) Get current counter value
	}
	while ( ( strX.ll & ~0xffffLL)  !=  (strTim1cnt.ll & ~0xffffLL) );	// Check if count changed on us
	return strX.ll;	
}

/******************************************************************************
 * unsigned int p1_Tim1_gettime_ui(void);
 * @brief	: Retrieve the extended timer counter count
 * @return	: Current timer count as an unsigned int
*******************************************************************************/
unsigned int p1_Tim1_gettime_ui(void)
{
	union TIMCAPTURE64 strX;

	strTim1cnt.us[0] = TIM1_CNT;	// (p 327) Get current counter value (16 bits)

	/* This 'do' takes care of the case where the counter turns over during the execution */
	do
	{ // Loop if the overflow count changed since the beginning
		strX.ui[0] = strTim1cnt.ui[0];	// Get low order word of current extended count
		strTim1cnt.us[0] = TIM1_CNT;	// (p 327) Get current counter value (16 bits)
	}
	while ( strX.us[1] != strTim1cnt.us[1] );// Check if extended count changed on us

	return strX.ui[0];			// Return lower 32 bits
}

/******************************************************************************
 * struct TIMCAPTRET32 p1_Tim1_inputcapture_ui(void);
 * @brief	: Retrieve the extended capture timer counter count and flag counter
 * @brief	: Lock interrupts
 * @return	: Current timer count and flag counter in a struct
*******************************************************************************/
struct TIMCAPTRET32 p1_Tim1_inputcapture_ui(void)
{
	struct TIMCAPTRET32 strY;			// 32b input capture time and flag counter
	int	tmp;

	TIM1_DIER &= ~(TIM_DIER_CC1IE | TIM_DIER_UIE);	// Disable CH1 capture interrupt and counter overflow (p 315)
	tmp = TIM1_DIER;				// Readback ensures that interrupts have locked

	strY.flg = uiTim1ch1_Flag; 			// Get flag counter
	strY.ic  = strTim1m.ui[0];			// Get 32b input capture time
	TIM1_DIER |= (TIM_DIER_CC1IE | TIM_DIER_UIE);	// Enable CH1 capture interrupt and counter overflow (p 315)
	
	return strY;
}


/*#######################################################################################
 * ISR routine for TIM1
 *####################################################################################### */
static int nTim1interruptctr;	// Count 8192 interrupts
static volatile unsigned int uiTim1icPrev;
volatile unsigned int uiTim1onesec;

volatile unsigned int Debug_TIM1;

void p1_TIM1_CC_IRQHandler(void)
{
	unsigned int uiSR = TIM1_SR & 0x03;	// Get capture & overflow flags
	unsigned int uiIC = TIM1_CCR1;		// Read the captured count which resets the capture flag
	volatile unsigned int temp;



	switch (uiSR)	// There are three cases where we do something.  The "00" case is bogus?
	{
	case 0x00:	// "Flagless interrupt" that is legitimate.
//Debug_TIM1 += 1;
nTim1Debug0 = uiIC;


	case 0x02:	// Catpure flag only
		nTim1interruptctr += 1;
		if (nTim1interruptctr >= TIM1INTERRUPTCOUNT) 
		{
			nTim1interruptctr = 0;	// Reset interrupt counter			

			/* Store the times */
			strTim1.us[0] = uiIC;			// Save capture time
			strTim1.us[1] = strTim1cnt.us[1];	// Extended time of upper 16 bits of lower order 32 bits
			
			uiTim1onesec = strTim1.ui[0] - uiTim1icPrev;
			uiTim1icPrev = strTim1.ui[0];

			strTim1.ui[1] = strTim1cnt.ui[1];	// Extended time of upper 32 bits of long long
			uiTim1ch1_Flag += 1;			// Advance the flag counter to signal mailine IC occurred
			strTim1m = strTim1;			// Update buffered value
		
		}
		temp = TIM1_SR;		// Readback to be sure flag is off before exiting
			
		return;


	case 0x03:
		// Both flags are on	
		TIM1_SR = ~0x1;			// Reset overflow flag

		nTim1interruptctr += 1;
		if (nTim1interruptctr >= TIM1INTERRUPTCOUNT) 
		{
			nTim1interruptctr = 0;	// Reset interrupt counter			

			/* Store the times */
			// Set up the input capture with extended time
			strTim1.us[0] = uiIC;			// Save capture time
			strTim1.us[1] = strTim1cnt.us[1];	// Extended time of upper 16 bits of lower order 32 bits
			strTim1.ui[1] = strTim1cnt.ui[1];	// Extended time of upper 32 bits of long long
			// Adjust input capture: Determine which flag came first.  If overflow came first increment the overflow count
			if (strTim1.us[0] < 0x8000)		// Is the capture time in the lower half of the range?
			{ // Here, yes.  The IC flag must have followed the overflow flag, so we 
				// First copy the extended time count upper 48 bits (the lower 16 bits have already been stored)
				strTim1.ll	+= 0x10000;	// Increment the high order 48 bits
			}
			strTim1m = strTim1;			// Update buffered value

			uiTim1onesec = strTim1.ui[0] - uiTim1icPrev;
			uiTim1icPrev = strTim1.ui[0];
		
			uiTim1ch1_Flag += 1;			// Advance the flag counter to signal mailine IC occurred	
	
		}

		temp = TIM1_SR;		// Readback register bit to be sure is cleared
	
		return;

	case 0x01:
Debug_TIM1 += 1;

		return;
	}
	// Here, a flagless, and therefore bogus interrupt.

	return;
}
/*#######################################################################################
 * ISR routine for TIM1
 *####################################################################################### */
void p1_TIM1_UP_IRQHandler(void)
{ //NOTE: Must be the same interrupt priority as "CC_IRQHandler"
	volatile unsigned int temp;

	TIM1_SR = ~0x1;			// Reset overflow flag

	strTim1cnt.ll	+= 0x10000;		// Increment the high order 48 bits of the timer counter

	temp = TIM1_SR;		// Readback register bit to be sure is cleared
	
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



