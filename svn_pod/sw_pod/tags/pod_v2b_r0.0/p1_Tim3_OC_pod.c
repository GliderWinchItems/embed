/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : p1_Tim3_OC_pod.c
* Hackeroo           : deh
* Date First Issued  : 11/13/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Output compare interrupts for replacing 32 KHz 8192 timer interrupts
* Note               : See 'gps_1pps.h' for '.h' file associated with this routine
*******************************************************************************/
/*
This is hack of 'p1_Tim3_OC_pod.c'.
10-11-2012 Modified for 32 KHz RTC 8192 Hz interrupt replacement
*/

/*
NOTE:
TIM3 CH1*--
An unsigned int supplies the estimated number of processor ticks for one second,
e.g. 2400609.  This routine counts 366 OC interrupts, which is 366*65536 ticks,
i.e. 23986176, sets up the 367th OC for the difference, in this example, 14433.
When the 367th interrupt is serviced the 32 KHz tick count is saved.  The difference
between this and the previous count is the number of 32 KHz osc ticks in one second.


*/
/*
10-11-2012
The RTC 32 KHz timer generates a 8192 Hz interrupt.  The 32 KHz ISR routine steers the interrupts so
that it phases onto the GPS 1 Hz interrupts (tim2).

*/
/*
Subroutine call references shown as "@n"--
@1 = ../tickadjust.c
@2 = ../gps_packetize.c
*/

#define TIM3_BASIC_COUNT	5859	// counts per interrupt (see above)
#define TIM3_ISR_COUNT		8192	// Number of interrupts in one second

#include "spi1ad7799.h"
#include "PODpinconfig.h"
#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/gpio.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/rtc.h"
#include "libopenstm32/pwr.h"
#include "libopenstm32/bkp.h"
#include "libopenstm32/timer.h"

#include "libmiscstm32/systick1.h"

#include "p1_common.h"


/* Subroutine prototype */
void pseudo_p1_RTC_IRQHandler(void);

/* -------- These came from 32KHz_p1.c -------- */
void 		(*p1_rtc_secf_ptr)(void);	// Address of function to call during RTC_IRQHandler of SECF (Seconds flag)
short		sPreTickReset;			// Reset count for adjusting time for temperature effects and freq offset


/* Time counts used by all */
volatile struct ALLTIMECOUNTS	strAlltime;	// In-memory rtc count, linux real time, and differences
/* -------------------------------------------- */


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
	/* ----------- TIM3 CH1* ------------------------------------------------------------------------*/
	/* Enable bus clocking for TIM3 (p 335 for beginning of section on TIM2-TIM7) */
	RCC_APB1ENR |= RCC_APB1ENR_TIM3EN;		// (p 105) 

	/* Enable bus clocking for alternate function */
	RCC_APB2ENR |= (RCC_APB2ENR_AFIOEN);		// (p 103) 

	/* Load initial time count p 386 */
	TIM3_ARR = TIM3_BASIC_COUNT;	// Default count

	/* Buffer the reload register p 371 */
	TIM3_CR1 |= (1 << 7);	// Bit 7 ARPE: Auto-reload preload enable

	/* Control register 1 */
	TIM3_CR1 |= TIM_CR1_CEN; 			// Counter enable: counter begins counting (p 371,2)

	/* Set and enable interrupt controller for doing software interrupt */
	NVICIPR (NVIC_TIM6_IRQ, TIM6_IRQ_PRIORITY );	// Set interrupt priority ('../lib/libusartstm32/nvicdirect.h')
	NVICISER(NVIC_TIM6_IRQ);			// Enable interrupt controller for RTC ('../lib/libusartstm32/nvicdirect.h')

	/* Set and enable interrupt controller for TIM3 interrupt */
	NVICIPR (NVIC_TIM3_IRQ, TIM3_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_TIM3_IRQ);			// Enable interrupt controller for TIM1
	
	/* Enable input capture interrupts */
	TIM3_DIER |= (TIM_DIER_UIE);	// Enable CH1 capture interrupt and counter overflow (p 376,7)

	return;
}

/*#######################################################################################
 * ISR routine for TIM3
 *####################################################################################### */
/* The following is measured by the GPS and gets set every second, once the GPS up and settled down. */
unsigned int tim3_tickspersec = 48000000;	// Default number of counter ticks for one second

unsigned int tim3_running_sum;		// Carries running sum of fractional interrupt duration

void TIM3_IRQHandler(void)
{
	int temp;

	if ( (TIM3_SR & 0x01) != 0)	// If the bit not on there is interrupt bogus'ness
	{
		MMIO32_BIT_BAND(&TIM3_SR,0x00) = 0;	// Reset overflow flag

		/* Load time count for the interrupt that follows the next one (i.e. ARR buffering is enabled) p 386 */

		/* Ticks are added to make total come out to one second.  With 8192 interrupts per second, if the number of 
		   number of actual ticks in one second is divided by 8192 the integer result is the number of ticks in each
		   interrupt duration.  This will be lower than the total required unless it divides evenly and there is no
		   remainder.  The fractional amount (i.e. the remainder of the dividsion) of the division is the number of ticks 
		   that must be added over the course of the 8192 interrupts.  The fractional amount is merely the low order 13 bits 
		   of the number of ticks per second.  If this amount is added to a running sum, when the sum exceeeds 8191,
		   (bits 14 and higher), then the running sum, shifted right 13 bits is the amount (which will be zero or one) 
		   to be added to the whole/integer interrupt duration.  After adding the "13 bit overflow", bits 14 and up
		   are set to zero, but fractional amount sum remains. */

		tim3_running_sum += (tim3_tickspersec & 8191);		// Add the fractional time
		TIM3_ARR = (tim3_tickspersec  + tim3_running_sum) >> 13;// Add "13 bit overflow count" to whole amount
		tim3_running_sum &= 8191;				// Reset count that is above 13 bits
		
		/* Here--same interrupt rate as the 32768 Hz osc divided by 4 */
		pseudo_p1_RTC_IRQHandler();		// Same handling as the RTC interrupt	

		temp = MMIO32_BIT_BAND(&TIM3_SR,0x00);	// Readback register bit to be sure flag has cleared

	}
	return;
}
/******************************************************************************
 * void pseudo_p1_RTC_IRQHandler(void);
 * @brief	: Handle timing phase locking as the 32 KHz isr would have done
*******************************************************************************/
static unsigned short DIFtmpctrPrev;	// For updating flag

/* NOTE: This is a hack of 32KHz_p1.c.*/
void pseudo_p1_RTC_IRQHandler(void)
{
	unsigned int uitemp;
		/* Time keeping is adjusted +/- in 1/8192th sec ticks.  The tension ADC and accelerometer are sampled at 1/4 this rate (2048/sec) */
		strAlltime.sPreTick += 1;
		if (strAlltime.sPreTick >= PRL_DIVIDE_PRE)	// Divide 4 completed?
		{ // Here, yes, 32768 Osc has been divided by 16 (4 in hardware, 4 with this counter)

			/* The routine to adjust the time for freq error due to temperature sets sPreTickReset (@2) */
			strAlltime.sPreTick = sPreTickReset;	// Reset divide count (usually to zero, but maybe +1 or -1)
			if (sPreTickReset != 0 ) 		// Only one tick adjustment at a time.
				sPreTickReset = 0;		// Clear tick adjust back to zero.

			/* Update the in-memory counter that mirrors the CNT register (differing by adjustment for drift) */

			/* Disable interrupts, since TIM2 is a higher interrupt priority the following might be caught 
                           during the increment. This is needed because the input capture due to the 1 pps pulse 
                           stores strAlltime.SYS.ull */
			TIM2_DIER &= ~(TIM_DIER_CC2IE | TIM_DIER_UIE);	// Disable CH2 capture interrupt and counter overflow (p 315)
			uitemp = TIM2_DIER;				// Readback ensures that interrupts have locked
	
			/* THIS IT!  The time tick counter used by all just and fair-minded citizens.  As you probably have surmised
                           this counter runs at 2048 per sec.  This is the RTC_CNT synchronized and shifted right two bits (since
                           the RTC_CNT counter is running at 8192 per sec).  */
			strAlltime.SYS.ull += 1;			// RTC_CNT, adjusted for drift

			TIM2_DIER |= (TIM_DIER_CC2IE | TIM_DIER_UIE);	// RE-enable CH2 capture interrupt and counter overflow (p 315)

			
			/* GPS adjustment of time.  'DIF is setup in 'gps_packetize.c' */
			if (DIFtmpctr > DIFtmpctrPrev)				// Is there a new 'DIF value?
			{ // Here, yes.  There is a new difference to be handled.
				DIFtmpctrPrev = DIFtmpctr;			// Update new data flag ctr
				if (DIFjamflag == 1)				// Take the difference in one big step? (@1)(@2)
				{ // Here, yes, take update in one step
					strAlltime.SYS.ll += strAlltime.DIF.ll;	// Make the adjustment
					strAlltime.nTickAdjust = 0;		// Zero out 'tickadjust.c' vars
					DIFjamflag = 0;				// Reset the jam flag 
				}
			}	

			/* Bring adjust time to GPS via ticks */
			if (TickAdjustflg != 0 ) // Do we have a new tick adjust value?
			{ // Here, yes.
				TickAdjustflg = 0;			// Reset flag
				strAlltime.nTickAdjustRTC = strAlltime.nTickAdjust;// Get local copy of value
				strAlltime.nTickAdjust = 0;		// Reset the sender's value
			}

			/* Adjust one tick at a time unti 'nTickAdjustRTC' is exhausted */
			if (strAlltime.nTickAdjustRTC != 0)		// Done? (Usually zero)
			{ // Here. no.
				if (strAlltime.nTickAdjustRTC < 0)	// Drop ticks?
				{ // Here, negative--we need to drop ticks
					sPreTickReset = +1;		// Drop one tick the next cycle
					strAlltime.nTickAdjustRTC += 1;	// Count up to zero
				}
				else
				{ // Here, positive--we need to add ticks
					sPreTickReset = -1;		// Add one tick the next cycle
					strAlltime.nTickAdjustRTC -= 1;	// Count down to zero
				}
			}

			/* Trigger a pending interrupt for TIM6, which will cause a chain of tick related routines to execute */
			NVICISPR(NVIC_TIM6_IRQ);	// Set pending (low priroity) interrupt for TIM6 ('../lib/libusartstm32/nvicdirect.h')

			return;
		}
}
/*#######################################################################################
 * ISR routine for TIM6 (low priority level)
 *####################################################################################### */

void p1_TIM6_IRQHandler(void)
{
/* This interrupt is caused by the RTC interrupt handler when further processing is required */
	/* Call other routines if an address is set up */
	if (p1_rtc_secf_ptr != 0)	// Having no address for the following is bad.
		(*p1_rtc_secf_ptr)();	// Go do something (e.g. poll the AD7799)	
	
	return;
}
