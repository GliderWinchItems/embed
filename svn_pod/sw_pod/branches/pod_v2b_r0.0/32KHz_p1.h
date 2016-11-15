/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : 32KHz_p1.h
* Hackeroo           : deh
* Date First Issued  : 09/03/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : 32KHz osc/clock, customized for svn_pod/sw_pod/trunk/pod_v1
*******************************************************************************/
/*
References:
@1 = ~/svn_pod/sw_stm32/trunk/lib/libsupportstm32/tickadjust.c


*/
/*
There are four times of interest plus two vars--

1) RTC_CNT (32b) counter (hardware)

This counter is zero when there has been no power.  Once started this counter continues
to count when the unit is in STANDBY (processor not powered, but RTC and Backup registers
remain powered).

This counter can not easily be written given the small window for writing.  Hence, the strategy
is to not disturb the counter.  It can be easily read, however.

This rate for this counter has a slight offset and also varies with temperature.  This error
can be estimated and incorporated in the timekeeping.

2) Tick-counter (64b): Extended precision mirror of RCT_CNT hardware counter

This 64 bit counter mirrors the RTC_CNT but adds significant bits.  The RTC_CNT counter counts at 2048
per second.  It will turn over in about 24 days 6 hours 32 minutes.  Making the count longer provides
the 1/2048 tick plus enough capacity for the linux format time of seconds since year 1900.

The 32 KHz oscillator drives a prescalar which is set to three, which divides the oscillator by
four.  This gives an interrupt rate of 8192 per sec (122.0703125 us between interrupts).  The
interrupts are counted down by a factor that is nominally four, but occassionally a count of
five or three is done to correct for the oscillator errors.  The nominal rate of divide by
four yields a rate of 2048 per second.  This is the polling rate for the AD7799 and selection of 
adc readings.

The tick counter is driven at this polling rate, i.e. 2048 per second.

3) Linux time tick counter (64)

This counter is set by adding a "difference value" to the tick-counter, 2) above.  This difference
is the value required to adjust the tick-counter to real-time.

This count is used for time-stamping packets.  The high order 21 bits is no importance, so for
speeding up transmission over the USART, the high order byte can be discarded.


4) GPS time count

The rising edige of the 1 pps pulse from the GPS, causes TIM2 input capture to store the current
tick-counter, 2) above.  The time in the ascii NMEA sentence that follows is extracted and converted to 
linux format time (count of seconds since year 1900).  This time count then correlates the 
tick-counter with the date/time of the GPS.  The resolution is +/- 244 usec, i.e. +/- 1/2
of a tick count.

This count is stored shifted left 11 bits as an unsigned long long, making it the same as the tick-counter 
format.  The low order bits are zero and  represent the tick count at the time of the rising edge 
of the 1 pps pulse.  

5) RCT_CNT versus tick-counter difference (64b)

The linux time tick count is computed by adding a time difference to the tick-counter.  The linux time
tick count is used to for time-stamping packets, and displays of current time.

Note: This number is signed.

6) Time keeping

When the unit first powers up--

RCT_CNT starts at zero.  After the 32 KHz initialization it begins counting at 2048 per second.  At some point
the GPS begins getting good fixes and the difference between the tick-count stored by the 1 pps can be
computed.  The Linux time tick count then becomes correct.

When the unit goes to STANDBY--

Saved in the (16b) backup registers are--
 tick-counter (48b = 3 registers)
 difference   (48b = 3 registers)
 RTC_CNT      (32b = 2 registers)
              (Total 8 registers)

(Some parsimony is present in the above since the STM32F103VCT6 (128K flash, 20K ram) only has 20 bytes
of backup register space.  Going the full 64 bits would use it all up.  Hence, the unused high order
half-word is lopped off.)

When the unit comes out of STANDBY--

The program sync's to the RTC interrupt, and the RTC_CNT is read.  Before the next tick, the extended
tick time is setup by computing the number of RTC_CNT ticks that have transpired since the counts were
stored.  This count is then added to the saved tick-counter, which brings it up-to-date with the RTC_CNT.
Note, the tick-counter can be expected to be different than the RTC_CNT since there will be an accumulation
of ticks due to the freq error adjustments.

The linux time tick count, then follows from the tick-counter since it is merely the tick-counter plus the
difference.


So folks there it is.  Grand in every way.  Complete and, wickedly straighforward.  Set your watch by it.
Not immersible in water, however.


*/



/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __32KHZ_P1
#define __32KHZ_P1

/* Includes ------------------------------------------------------------------*/
#include "libopenstm32/common.h"	// Has things like 'u16' defined
#include "libopenstm32/gpio.h"
#include "libopenstm32/spi.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/rtc.h"
#include <time.h>
#include "p1_common.h"

/* This is the PRL divide-by */
#define PRL_DIVIDE_PRE_ORDER	2		// Power or 2 order for pre-scaling
#define PRL_DIVIDE_PRE		(1<<PRL_DIVIDE_PRE_ORDER)// PRL Divide by count (4)
#define	RTC_CLK_ORDER		15		// 32768KHz = (1<<15)
#define RTC_TICK_FREQ	((1<<RTC_CLK_ORDER)/PRL_DIVIDE_PRE)	// Freq (Hz) of the RTC_CNT & 'uiRTCsystemcounter' (in-memory ctr)

/* This is the divide-by after interrupt ticks divided by software */
#define PRL_DIVIDE_ORDER	4		// (Divide by 16 (total division))

#define PRL_DIVIDER	(1<<PRL_DIVIDE_ORDER)	// RTCCLK divider
#define ALR_INC_ORDER	(RTC_CLK_ORDER-PRL_DIVIDE_ORDER)	// (Divide by 2048)
#define ALR_INCREMENT	(1<<ALR_INC_ORDER)	// Number of pre-scaled clocks for one second
#define	ALR_EXTENSION		BKP_DR1		// Backup register that holds CNT register extension

/* Note: The lower four bits of the priority byte are not used.
 The higher the priority number, the lower the priority */
#define RTC_PRIORITY		0x20	// Interrupt priority for RTC interrupt

/*
When the RTC interrupt reaches the point where the interrupt has been serviced, but there
are more routines that need to be run, the RTC handler sets the interrupt pending bit
for TIM6 in the NVIC.  This interrupt is set to a low priority level.  The RTC handler
returns from interrupt, and the TIM6 interrupt then is serviced (unless there are other
higher level interrupts pending).  The TIM6 interrupt handler then calls the routines
which execute and return, and the returing TIM6 handler resets that interrupt level
active bit.
*/

/* We don't have TIM6 on this processor so we will use the interrupt */
#define TIM6_IRQ_PRIORITY	0xE0	// Interrupt priority for TIM6






/* The following variables are associated with the RTC clock */
extern void 			(*p1_rtc_secf_ptr)(void);	// Address of function to call during RTC_IRQHandler of SECF (Seconds flag)
extern volatile char		p1_cResetFlag;		// Out of a reset: 1 = 32 KHz osc was not setup; 2 = osc setup OK, backup domain was powered
extern short			sPreTickReset;		// Reset count for adjusting time for temperature effects and freq offset


/* Time counts used by all */
extern volatile struct ALLTIMECOUNTS	strAlltime;	// In-memory rtc count, linux real time, and differences

extern unsigned int tim3_tickspersec;

#endif 
