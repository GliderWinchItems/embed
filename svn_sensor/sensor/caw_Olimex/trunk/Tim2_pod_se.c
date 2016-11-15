/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : Tim2_pod.c
* Hackeroo           : deh
* Date First Issued  : 01/05/2013
* Board              : STM32F103VxT6_pod_mm
* Description        : Input capture with TIM2 CH1 (PA1) on POD board.
* Note               : Use 'gps_1pps.h' with this routine
*******************************************************************************/
/*
NOTE:
Ref Manual RM0008 Rev 14, p 350

TIM2 CH2 (PA1)--

A 1 pps pulse from the gps is divided externally to 3.3v and applied to PA1 configured
for TIM2 CH2.

The timer overflow flag interrupt adds to the extended counts (upper 48 bits 
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
volatile union TIMCAPTURE64	strTim2cnt;	// 64 bit extended TIM2 CH1 timer count
// Input capture extended counts
volatile union TIMCAPTURE64	strTim2;	// 64 bit extended TIM2 CH1 capture
volatile union TIMCAPTURE64	strTim2m;	// 64 bit extended TIM2 CH1 capture (main)

/* The readings and flag counters are updated upon each capture interrupt */
volatile unsigned short		usTim2ch2_Flag;		// Incremented when a new capture interrupt serviced, TIM2CH1*

/* Holds the RTC CNT register count that is maintained in memory when power is up */
extern unsigned int uiRTCsystemcounter;	// This mirrors the RTC CNT register, and is updated each RTC Secf interrupt

/******************************************************************************
 * void Tim2_pod_init(void);
 * @brief	: Initialize Tim2 for input capture
*******************************************************************************/
void Tim2_pod_init(void)
{

// Delay starting TIM2 counter
//volatile int i;
//for (i = 0; i < 1000; i++);	// 1000 results in 13060 count difference between the two timers

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

	/* Set and enable interrupt controller for TIM2 interrupt */
	NVICIPR (NVIC_TIM2_IRQ, TIM2_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_TIM2_IRQ);			// Enable interrupt controller for TIM1
	
	/* Enable input capture interrupts */
//	TIM2_DIER |= (TIM_DIER_CC2IE | TIM_DIER_UIE);	// Enable CH2 capture interrupt and counter overflow (p 393)
	TIM2_DIER |= (TIM_DIER_UIE);	// Enable Update Interrupt, counter overflow (p 393)

	return;
}
/******************************************************************************
 * unsigned long long Tim2_gettime_ll(void);
 * @brief	: Retrieve the extended timer counter count
 * @return	: Current timer count as an unsigned long long
*******************************************************************************/
unsigned long long Tim2_gettime_ll(void)
{
	union TIMCAPTURE64 strX;
	strTim2cnt.us[0] = TIM2_CNT;	// (p 327) Get current counter value (16 bits)
	/* This 'do' takes care of the case where the counter turns over during the execution */
	do
	{ // Loop if the overflow count changed since the beginning
		strX = strTim2cnt;			// Get current extended count
		strTim2cnt.us[0] = TIM2_CNT;	// (p 327) Get current counter value (16 bits)
	}
	while ( ( strX.ll & ~0xffffLL)  !=   (strTim2cnt.ll & ~0xffffLL) );	// Check if count changed on us
	return strX.ll;	
}
/******************************************************************************
 * unsigned int Tim2_gettime_ui(void);
 * @brief	: Retrieve the extended timer counter count
 * @return	: Current timer count as an unsigned int
*******************************************************************************/
unsigned int Tim2_gettime_ui(void)
{
	union TIMCAPTURE64 strX;			

	strTim2cnt.us[0] = TIM2_CNT;	// (p 327) Get current counter value (16 bits)

	/* This 'do' takes care of the case where the counter turns over during the execution */
	do
	{ // Loop if the overflow count changed since the beginning
		strX.ui[0] = strTim2cnt.ui[0];	// Get low order word of current extended count
		strTim2cnt.us[0] = TIM2_CNT;	// (p 327) Get current counter value (16 bits)
	}
	while ( strX.us[1] != strTim2cnt.us[1] );// Check if extended count changed on us
	return strX.ui[0];			// Return lower 32 bits
}
/******************************************************************************
 * struct TIMCAPTRET32 Tim2_inputcapture_ui(void);
 * @brief	: Retrieve the extended capture timer counter count and flag counter
 * @brief	: Lock interrupts
 * @return	: Current timer count and flag counter in a struct
*******************************************************************************/
struct TIMCAPTRET32 Tim2_inputcapture_ui(void)
{
	struct TIMCAPTRET32 strY;			// 32b input capture time and flag counter
	int	tmp;

	TIM2_DIER &= ~(TIM_DIER_CC2IE | TIM_DIER_UIE);	// Disable CH2 capture interrupt and counter overflow (p 315)
	tmp = TIM2_DIER;				// Readback ensures that interrupts have locked

/* The following is an alternate for to the two instructions above for assuring that the interrupt enable bits
   have been cleared.  The following results in exactly the same number of instructions and bytes as the above.
   The only difference is the last instruction is 'str' rather than 'ldr'. */
//	tmp = TIM2_DIER;
//	tmp &= ~(TIM_DIER_CC2IE | TIM_DIER_UIE);
//	TIM2_DIER = tmp;
//	TIM2_DIER = tmp;


	strY.ic  = strTim2m.ui[0];			// Get 32b input capture time
	strY.flg = usTim2ch2_Flag; 			// Get flag counter
	TIM2_DIER |= (TIM_DIER_CC2IE | TIM_DIER_UIE);	// Enable CH2 capture interrupt and counter overflow (p 315)
	
	return strY;
}
/*#######################################################################################
 * ISR routine for TIM2
 *####################################################################################### */
void TIM2_IRQHandler(void)
{
	volatile unsigned int temp;

	unsigned short usSR = TIM2_SR & 0x05;	// Get capture & overflow flags

	switch (usSR)	// There are three cases where we do something.  The "00" case is bogus.
	{


	case 0x01:	// Overflow flag only
			MMIO32_BIT_BAND(&TIM2_SR,0x00) = 0;	// Reset overflow flag
			strTim2cnt.ll	+= 0x10000;		// Increment the high order 48 bits of the timer counter
			temp = MMIO32_BIT_BAND(&TIM2_SR,0x00);	// Readback register bit to be sure is cleared
			break;

	case 0x00:	// Case where ic flag got turned off by overlow interrupt reset coinciding with ic signal

	case 0x04:	// Catpure flag only
			strTim2.us[0] = TIM2_CCR2;		// Read the captured count which resets the capture flag
			strTim2.us[1] = strTim2cnt.us[1];	// Extended time of upper 16 bits of lower order 32 bits
			strTim2.ui[1] = strTim2cnt.ui[1];	// Extended time of upper 32 bits of long long
			usTim2ch2_Flag += 1;			// Advance the flag counter to signal mailine IC occurred
			strTim2m = strTim2;			// Update buffered value		
			temp = MMIO32_BIT_BAND(&TIM2_SR,0x00);	// Readback register bit to be sure is cleared		
			break;

	case 0x05:	// Both flags are on	

			// Take care of overflow flag
			MMIO32_BIT_BAND(&TIM2_SR,0x00) = 0;	// Reset overflow flag

			// Set up the input capture with extended time
			strTim2.us[0] = TIM2_CCR2;		// Read the captured count which resets the capture flag
			strTim2.us[1] = strTim2cnt.us[1];	// Extended time of upper 16 bits of lower order 32 bits
			strTim2.ui[1] = strTim2cnt.ui[1];	// Extended time of upper 32 bits of long long
			// Adjust inpute capture: Determine which flag came first.  If overflow came first increment the overflow count
			if (strTim2.us[0] < 0x8000)		// Is the capture time in the lower half of the range?
			{ // Here, yes.  The IC flag must have followed the overflow flag, so we 
				// First copy the extended time count upper 48 bits (the lower 16 bits have already been stored)
				strTim2.ll	+= 0x10000;	// Increment the high order 48 bits
			}


			usTim2ch2_Flag += 1;			// Advance the flag counter to signal mailine IC occurred		
			strTim2m = strTim2;			// Update buffered value		

			strTim2cnt.ll	+= 0x10000;		// Increment the high order 48 bits of the timer counter

			temp = MMIO32_BIT_BAND(&TIM2_SR,0x00);	// Readback register bit to be sure is cleared

			break;
	
	}
	return;
}

