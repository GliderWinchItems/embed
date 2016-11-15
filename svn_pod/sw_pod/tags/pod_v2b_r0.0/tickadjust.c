/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : tickadjust.c
* Hacker	     : deh
* Date First Issued  : 08/20/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Adc thermistor reading adjustment of tick time
*******************************************************************************/
/*
Summary--

The RTC interrupts are at a rate of 32768/16 -> 8192 per second.  These are counted up to 
four by a counter that is nominally starts at zero.  By setting the starting count of this
counter time can be adjusted.  Starting at one drops one 8192 tick, and starting at -1 adds 
one 8192 tick.  

The 32 KHz osc on these boards tends to be slow, so ticks need to be dropped to make up for
the interrupt rate being too slow.  Increased or decreased temperature around a "turn point"
of 25 deg C decreases the frequency by the square of the temperature difference from the
turn-point temperature.

The estimated error therefore consists of two components.  One is the constant frequency offset
and the other is the error due to temperature.  The offset error is simple a constant in
the 'calibration.c' struct.  The temperature error comes from a table lookup that uses the
thermistor ADC reading to index into a table.  Both errors are in ppm (parts-per-million) * 100.

The temperature table lookup returns the error as a positive number.  Positive errors call for
dropping ticks, negative for adding ticks.

The 8 MHz processor xtal osc is about one order of magnitude better than the 32 KHz xtal with
regard to frequency versus temperature.  Unlike the 32 KHz xtal where the error curve is parabolic,
the 8 MHz xtal is a cubic.  The first zero slope of the curve occurs somewhere around 0 deg C, a
temperature lower than the range of interest, and about 45 deg which is at the upper end of
the range of interest.  A polynomial regression on experimental data generates coefficients
that are used to compute the error versus temperature for the 8 MHz xtal.

When the unit is in 'normal_run' mode the 32 KHz xtal is timed.  A hardware connection between
the 32 KHz calibration pin, MCO, to TIM1 allows TIM1 to accumulate bus ticks for about
one second, i.e. the 32768 Hz osc is divided by 64 in the processor, and the TIM1 interrupt
routine accumulates the time for 512 interrupts.  This gives a count nominally, 24E6, but typically
24000700.  The slower the 32 KHz osc, the higher the count above the nominal 24E6.  However,
the freq of the 8 MHz osc is a factor.  

When the GPS is present and giving good fixes the GPS 1 PPS pulse causes input captures on TIM2
which provide the number of TIM1 bus ticks between 1 PPS pulses.  Nominally 48E6; typically 
48001103.  

Using the 32 KHz and 8 MHz measurements, or 8 MHz estimate, the 32 KHz osc error can be computed.  Since
the 8 MHz frequency is better than that of the 32 KHz, the estimate is improved over using just the
predicted error, given the temperature, of the 32 KHz osc alone.

When the unit is not in the 'normal_run' mode the "up" time is too short to take a 1 second measurement,
in which case the 32 KHz error is based solely on the temperature and lookup table.

Once the 32 KHz osc error is computed, it is add to an error accumulator.  The estimated error is added 
to the accumulator and ticks added or dropped with the error accumulation exceeds the time interval of
one-half the interval of one tick.

Approximately every ten seconds the error estimated.  The error is added   When the error exceeds 1/2 of
one tick adjustment in the positive direction one tick is dropped, and the amount of time 
of the adjustment subtracted from the accumulator.  The signs are reversed for negative 
errors.

One 8192 tick adjustment shifts the time by 122,070 ns.  Errors begin at ppm*100.  Multiplying
by ten puts them in a ppb (parts per billion), or 1 ns error per second time.  Muliplying
by the number of seconds between error checks (10 seconds), yields the number of ns error
estimated to have accumulated in the period since the last check.  When this number exceeds
1/2 the ns that one tick dropped (or added, in the negative case) a tick is dropped (or added).

When the unit goes into STANDBY, the error accumulator and the next time to check are 
maintained in the BKP registers which are held during STANDBY.

For "cold starts" (power down/up reset), the accumulator is started at zero and the 
next time to check initialized to the time increment.
*/

/*
Subroutine call references shown as "@n"--
@1 = svn_pod/sw_pod/trunk/pod_v1/32KHz_p1.c
@2 = svn_pod/sw_stm32/trunk/lib/libsupportstm32/adcppm_cal_1.c
@3 = svn_pod/sw_stm32/trunk/lib/libsupportstm32/adctherm_cal.c
@4 = svn_pod/sw_stm32/trunk/lib/libsupportstm32/adcppm_cal.c
@5 = svn_pod/sw_pod/trunk/p1_normal_run.c
@6 = svn_pod/sw_pod/trunk/p1_common.h
@7 = svn_pod/sw_stm32/trunk/lib/libsupportstm32/calibration.c,.h
*/

#include "libopenstm32/rtc.h"			// For realtime clock
#include "libopenstm32/bkp.h"			// For rtc backup registers
#include "libmiscstm32/clockspecifysetup.h"	// For clock setup
#include "libmiscstm32/systick1.h"		// SYSTICK routine

	
#include "p1_common.h"		// Just about everything is in this

#include "calibration.h"	// Calibration structure 
#include "adcppm_cal.h"		// Calibration routine for temp error
#include "adcppm_cal_1.h"	// Calibration routine for temp error

/* 1E9/8192 -> 122070.31 (ns /tick adjust), i.e. at 8192 interrupts/sec, one tick is about 122 usec */
#define NS_TIME_ERR_PER_ADJUSTABLE_TICK	(1000000000/8192)	// Duration of one tick (ns)
//#define NS_TIME_ERR_PER_ADJUSTABLE_TICK	122070	// Duration of one tick (ns)

/* Default calibrations for various devices (@7) */
extern struct CALBLOCK strDefaultCalib;		// Default calibration

/* This counter is incremented by the RTC Secf interrupts (see 32KHz_p1.c RTC_IRQHandler) (@1) */
extern short sPreTickReset;	// Counter used for adjusting time for temperature effects and freq offset

/* Used by '32KHz_p1.c' for adjusting time to GPS */
unsigned char DIFtmpctr;		// Increments each GPS storing into 'ullDIFtmp'
unsigned char DIFjamflag;		// Flag: 0 = do not jam, not zero = jam 
unsigned char TickAdjustflg;		// Increments each update of strAlltime.nTickAdjust
unsigned char TickAdjustOTO;		// One-Time-Only.  Set to 1 tells deepsleep_run that it is OK to shutdown


/* 
An accumulator keeps a running sum of the errors--
Errors are specified in ppm * 100, but the accumulator is scaled
to ns/4 so that a 1/2 RTC tick duration scales to a whole number.

The error returned from the table lookup for temperature deviation is positive, but the
frequency deviation is always negative.  At 25 deg C the deviation due to temperature is zero, 
and the freq declines with temperatures above and below this "turnpoint".  This means that
as the temperature deviates from 25 deg C, the freq becomes lower, which means the 
ticks need to be added when the accumulated error becomes equal to 1/2 of a tick duration.

The osc initial offset error (specified at 25 deg C) can be negative as well as positive
depending on the hardware, so it is conceivable that ticks would need to be subtracted
when the accumulated error fell to 1/2 of a tick duration.

'strAlltime.uiNextTickAdjTime' is a tick count (2048 basis) that is used to determine when 
enough time has elapses that a tick adjustment is worth doing.  This count is set to zero 
each time the GPS steers the SYS tick count to a zero difference with the GPS.  When it is
not being reset by the GPS, then it accumulates time ticks.  

When 'NextTickAdjuTime' accumulation exceeds the time increment for doing an adjustment (currently, 
nominally 10 secs) this count is divided by the time increment (in case it happens to be
greater than one which could occur if the deepsleep time intervals are longer than
the time increment for doing an update).  The division result then is used to subtract the 
amount of time "used" with the time adjustment.  This allows a somewhat variable time
between wakeup from STANDBY without add/dropping time between tick adjustments.
*/

int Debug_nError;
int Debug_nErrorT;
int Debug_nErrorO;

/* Address this routine will go to upon completion */
void 	(*rtc_tickadjustdone_ptr)(void);// Address of function to call to go to for further handling under RTC interrupt

int nAdcppm_temp_latest;	// Latest calibration table lookup (ppm*1000)
/******************************************************************************
 * void rtc_tickadjust(void);
 * @brief	: Pick up ADC therm reading and call RTC_tickadjust
 ******************************************************************************/
/*
This routine is entered from the chain of calls when there is a 32 KHz secf interrupt and
the counter reaches (nominally) 4.  The TIM6 interrupt, set to a very low priority, is forced which
calls a routine and the exit of that routine may call another routine.
HENCE--this routine is entered at a nominal rate of 2048 per second under a low level interrupt
priority (see 'p1_32KHz.c').
*/
void rtc_tickadjust(void)
{
	if (uiThermistoradcflag != uiThermistoradcflagPrev)	// Is a thermistor reading ready?
	{ // Here, yes.
		uiThermistoradcflagPrev = uiThermistoradcflag;	// Reset the comparator
//		RTC_tickadjust(uiThermistor_tickadjust);	// Do a tick adjust with this temp reading
	}

	/* If the address for the next function is not set, simply return. */
	if (rtc_tickadjustdone_ptr != 0)	// Having no address for the following is bad.
		(*rtc_tickadjustdone_ptr)();	// Go do something (e.g. poll the AD7799)
	return;
}

/* 
====================================================================================
The following routines in this file are called at the level of the main polling loop        
====================================================================================
*/


/******************************************************************************
 * int rtc_tick_edit(unsigned int uiTIM2);
 * @brief	: Check for out-of-range data, given gps TIM1, TIM2 tick data
 * @param	: uiTIM2: processor ticks between GPS 1 PPS interrupts 
 * @return	: 0 = OK, 1 = fail;
 ******************************************************************************/
/* Nominal bus freq for TIM1 24000000; for TIM2 48000000 */
#define TIM2HILIMIT	48005000	// Max allowable
#define TIM2LOLIMIT	47995000	// Min allowable

int rtc_tick_edit(unsigned int uiTIM2)
{
	if (uiTIM2 > TIM2HILIMIT) return 1;
	if (uiTIM2 < TIM2LOLIMIT) return 1;
	return 0;
}
/******************************************************************************
 * void gps_tickadjust(void);
 * @brief	: Adjust tick time to GPS
 ******************************************************************************/
 void gps_tickadjust(void)
{

	/* Handle large differences in one big step; small differences in small steps over a period of time */
	if (strAlltime.DIF.ll != 0)	// Good chance it is zero.
	{
		if ((strAlltime.DIF.ll > TICKJAMTOLERANCE) || (strAlltime.DIF.ll < -TICKJAMTOLERANCE))
		{ // Here we will want to take the update in one jump, (forwards or backwards)
			DIFjamflag = 1;
		}
		else
		{ // Here we will do the adjustment over a period of time, one sub-tick per tick
			DIFjamflag = 0;
			/* Save number of 8192/sec ticks required to adjust SYS to time */
			strAlltime.nTickAdjust = -(strAlltime.DIF.n[0] << PRL_DIVIDE_PRE_ORDER);
			TickAdjustflg += 1;
		}
		DIFtmpctr += 1;
	}
	return;
}

