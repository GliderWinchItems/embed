/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : Tim3_pod.c
* Hackeroo           : deh
* Date First Issued  : 08/06/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Routines that use PC13->PE9 for measuring 1 pps gps vs 32 KHz osc
* Note               : Sue 'gps_1pps.h' with this routine
*******************************************************************************/
/*
NOTE:

TIM3 CH1*--

A 1 pps pulse from the gps, 0-5v, applied to PC6, TIM3 CH1* (remapped) (or TIM8 CH1), 
captures the 1 pps rising edge.

This makes use of PC6, available on the Encoder_2_B pin.  This is a 5v tolerant pin.

With both timers the overflow flag interrupt adds to the extended counts (upper 48 bits 
of the long long) as well as the extended counts for the capture times.  The capture
register stores in the low 16 bits of the long long upon capture interrupt.

There is a special case where the overflow flag and capture flags are both on when the routine
is executed.


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

#include "gps_1pps.h"
#include "bit_banding.h"



/* Various bit lengths of the timer counters are handled with a union */
// Timer counter extended counts
volatile union TIMCAPTURE64	strTim3cnt;	// 64 bit extended TIM3 CH1 timer count
// Input capture extended counts
volatile union TIMCAPTURE64	strTim3;	// 64 bit extended TIM3 CH1 capture
volatile union TIMCAPTURE64	strTim3m;	// 64 bit extended TIM3 CH1 capture

/* The readings and flag counters are updated upon each capture interrupt */
volatile unsigned short		usTim3ch1_Flag;		// Incremented when a new capture interrupt serviced, TIM3CH1*


/******************************************************************************
 * void Tim3_pod_init(void);
 * @brief	: Initialize Tim3 for input capture
*******************************************************************************/
void Tim3_pod_init(void)
{

// Delay starting TIM3 counter
//volatile int i;
//for (i = 0; i < 1000; i++);	// 1000 results in 13060 count difference between the two timers

	/* ----------- TIM3 CH1* ------------------------------------------------------------------------*/
	/* Enable bus clocking for TIM3 (p 335 for beginning of section on TIM2-TIM7) */
	RCC_APB1ENR |= RCC_APB1ENR_TIM3EN;		// (p 105) 

	/* Enable bus clocking for alternate function */
	RCC_APB2ENR |= (RCC_APB2ENR_AFIOEN);		// (p 103) 

	/* Remap TIM3 */
	AFIO_MAPR |= AFIO_MAPR_TIM3_REMAP_FULL_REMAP; 	// (p 182) (remap TIM3 to PC6-PC9)

	/* TIMx capture/compare mode register 1  */
	TIM3_CCMR1 |= TIM_CCMR1_CC1S_IN_TI1;		// (p 379,380)CC1 channel is configured as input, IC1 is mapped on TI1

	/* Compare/Capture Enable Reg (p 324,5) */
	//  Configured as input: rising edge trigger
	TIM3_CCER |= 0x01;				// Capture Enabled (p 384,5) 

	/* Control register 2 */
	// Default: The TIMx_CH1 pin is connected to TI1 input (p 372)

	/* Control register 1 */
	TIM3_CR1 |= TIM_CR1_CEN; 			// Counter enable: counter begins counting (p 371,2)

	/* Set and enable interrupt controller for TIM3 interrupt */
	NVICIPR (NVIC_TIM3_IRQ, TIM3_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_TIM3_IRQ);			// Enable interrupt controller for TIM1
	
	/* Enable input capture interrupts */
	TIM3_DIER |= (TIM_DIER_CC1IE | TIM_DIER_UIE);	// Enable CH1 capture interrupt and counter overflow (p 376,7)

	return;
}
/******************************************************************************
 * unsigned long long Tim3_gettime_ll(void);
 * @brief	: Retrieve the extended timer counter count
 * @return	: Current timer count as an unsigned long long
*******************************************************************************/
unsigned long long Tim3_gettime_ll(void)
{
	union TIMCAPTURE64 strX;
	strTim3cnt.us[0] = TIM3_CNT;	// (p 327) Get current counter value (16 bits)
	/* This 'do' takes care of the case where the counter turns over during the execution */
	do
	{ // Loop if the overflow count changed since the beginning
		strX = strTim3cnt;			// Get current extended count
		strTim3cnt.us[0] = TIM3_CNT;	// (p 327) Get current counter value (16 bits)
	}
	while ( ( strX.ll & ~0xffffLL)  !=   (strTim3cnt.ll & ~0xffffLL) );	// Check if count changed on us
	return strX.ll;	
}
/******************************************************************************
 * unsigned int Tim3_gettime_ui(void);
 * @brief	: Retrieve the extended timer counter count
 * @return	: Current timer count as an unsigned int
*******************************************************************************/
unsigned int Tim3_gettime_ui(void)
{
	union TIMCAPTURE64 strX;			

	strTim3cnt.us[0] = TIM3_CNT;	// (p 327) Get current counter value (16 bits)

	/* This 'do' takes care of the case where the counter turns over during the execution */
	do
	{ // Loop if the overflow count changed since the beginning
		strX.ui[0] = strTim3cnt.ui[0];	// Get low order word of current extended count
		strTim3cnt.us[0] = TIM3_CNT;	// (p 327) Get current counter value (16 bits)
	}
	while ( strX.us[1] != strTim3cnt.us[1] );// Check if extended count changed on us
	return strX.ui[0];			// Return lower 32 bits
}
/******************************************************************************
 * struct TIMCAPTRET32 Tim3_inputcapture_ui(void);
 * @brief	: Retrieve the extended capture timer counter count and flag counter
 * @brief	: Lock interrupts
 * @return	: Current timer count and flag counter in a struct
*******************************************************************************/
struct TIMCAPTRET32 Tim3_inputcapture_ui(void)
{
	struct TIMCAPTRET32 strY;			// 32b input capture time and flag counter
	int	tmp;

	TIM3_DIER &= ~(TIM_DIER_CC1IE | TIM_DIER_UIE);	// Disable CH1 capture interrupt and counter overflow (p 315)
	tmp = TIM3_DIER;				// Readback ensures that interrupts have locked

	strY.ic  = strTim3m.ui[0];			// Get 32b input capture time
	strY.flg = usTim3ch1_Flag; 			// Get flag counter
	TIM3_DIER |= (TIM_DIER_CC1IE | TIM_DIER_UIE);	// Enable CH1 capture interrupt and counter overflow (p 315)
	
	return strY;
}
/*#######################################################################################
 * ISR routine for TIM3
 *####################################################################################### */
void TIM3_IRQHandler(void)
{
	volatile unsigned int temp;

	unsigned short usSR = TIM3_SR & 0x09;	// Get capture & overflow flags

	switch (usSR)	// There are three cases where we do something.  The "00" case is bogus.
	{

	case 0x01:	// Overflow flag only
			TIM3_SR = ~0x1;				// Reset overflow flag
			strTim3cnt.ll	+= 0x10000;		// Increment the high order 48 bits of the timer counter
			temp = TIM3_SR;				// Readback register bit to be sure is cleared
			return;

	case 0x00:	// Case where ic flag got turned off by overlow interrupt reset coinciding with ic signal
	case 0x08:	// Catpure flag only
			strTim3.us[0] = TIM3_CCR1;		// Read the captured count which resets the capture flag
			strTim3.us[1] = strTim3cnt.us[1];	// Extended time of upper 16 bits of lower order 32 bits
			strTim3.ui[1] = strTim3cnt.ui[1];	// Extended time of upper 32 bits of long long
			 usTim3ch1_Flag += 1;			// Advance the flag counter to signal mailine IC occurred		
			strTim3m = strTim3;			// Update buffered value
			temp = TIM3_SR;				// Readback register bit to be sure is cleared
			return;

	case 0x09:	// Both flags are on	

			// Take care of overflow flag
			TIM3_SR = ~0x1;				// Reset overflow flag

			// Set up the input capture with extended time
			strTim3.us[0] = TIM3_CCR1;		// Read the captured count which resets the capture flag
			strTim3.us[1] = strTim3cnt.us[1];	// Extended time of upper 16 bits of lower order 32 bits
			strTim3.ui[1] = strTim3cnt.ui[1];	// Extended time of upper 32 bits of long long
			// Adjust inpute capture: Determine which flag came first.  If overflow came first increment the overflow count
			if (strTim3.us[0] < 0x8000)		// Is the capture time in the lower half of the range?
			{ // Here, yes.  The IC flag must have followed the overflow flag, so we 
				// First copy the extended time count upper 48 bits (the lower 16 bits have already been stored)
				strTim3.ll	+= 0x10000;	// Increment the high order 48 bits
			}
			usTim3ch1_Flag += 1;			// Advance the flag counter to signal mailine IC occurred
			strTim3m = strTim3;			// Update buffered value				
			strTim3cnt.ll	+= 0x10000;		// Increment the high order 48 bits of the timer counter

			temp = TIM3_SR;				// Readback register bit to be sure is cleared

			return;
	
	}
	return;
}

