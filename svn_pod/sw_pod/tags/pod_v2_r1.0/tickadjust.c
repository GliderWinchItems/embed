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

#include <math.h>	// We got lazy and used floating pt for the polynomial computation
	
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
		RTC_tickadjust(uiThermistor_tickadjust);	// Do a tick adjust with this temp reading
	}

	/* If the address for the next function is not set, simply return. */
	if (rtc_tickadjustdone_ptr != 0)	// Having no address for the following is bad.
		(*rtc_tickadjustdone_ptr)();	// Go do something (e.g. poll the AD7799)
	return;
}
/******************************************************************************
 * void RTC_tickadjust(unsigned int uiAdcTherm);
 * @param	: Adc reading for thermistor.
 * @brief	: Adjust RTC tick counter for accumulation of errors
 ******************************************************************************/
/* The above routine 'rtc_tickadjust' is the only routine that uses this
   routine.
*/
void RTC_tickadjust(unsigned int uiAdcTherm)
{
	int nErrorRate;
	int temp;
	int nNumerator;
	int nRecip;
	unsigned uiP8;
	unsigned uitemp;
	
	/* Is it time to check if a tick adjustment needs to be made */
	if ( strAlltime.uiNextTickAdjTime >= UPDATEINCREMENT)	// In memory RTC_CNT versus update time
	{ // Here, it is time to make a check
		strAlltime.uiNextTickAdjTime = 0;	// Advance to next update time

		/* Get error due to temperature (table lookup. (@4)) */
//		nAdcppm_temp_latest = adcppm_cal(uiAdcTherm,strDefaultCalib.adcppm );// Convert ADC reading to freq error (ppm * 100)

		/* Convert thermister adc reading to temperature (@3), then compute freq error (@2) */
		uitemp = adctherm_cal(uiAdcTherm) + strDefaultCalib.tmpoff;
	
		/* Compute error based on formula in spec sheet: error = a * (T-To)^2  */
//		nAdcppm_temp_latest = adcppm_cal_1(uitemp);// Convert temperature reading to freq error (ppm * 100)

		/* Do adjustments on the basis of ppb (parts per billion; ppm*1000; 1E-9) */
//		nAdcppm_temp_latest = (nAdcppm_temp_latest*TEMPTABLEADJUST)/100;	// Tweak effect of table, and scale to ppb

		/* Compute error based on polynomial approximation from experimental data */
//		nAdcppm_temp_latest = strAlltime.nPolyOffset;

		/* If we have a 1 sec reading for the 32 KHz osc, then use the temp adjusted estimate for one second of 8 MHz osc */
		// Arg 1 = processor ticks for 512 interrupts of 32 KHz osc MCO pin (1 second) 
		// Arg 2 = processor ticks estimated by polynomial approximation, for 1 second, given the temperature reading
		uiP8 = strAlltime.nOscOffset8+strAlltime.nPolyOffset8+48000000;
		if ( (rtc_tick_edit(uiTim1onesec,uiP8)) == 0)// Are the readings reasonable?
		{ // Here yes, the two readings are reasonable. 
			/* Compute 32 KHz error rate in parts per billion */
			nNumerator = (int)(2*uiTim1onesec - uiP8);
			nRecip     = (int)uiP8 / nNumerator;
			nErrorRate = (1000000000 / nRecip);
		}
		else
		{ // Here, revert to 32 KHz error computation without the benefit of measurement using the 8 MHz osc
			nErrorRate = nAdcppm_temp_latest + strAlltime.nOscOffset32;
		}

Debug_nErrorT= uitemp;


		/* Add in fixed error of osc */
//		nErrorRate += strDefaultCalib.ppmoff;	// Note: this is signed.  Positive temp error means freq too low

Debug_nError = nErrorRate;

		/* Polynomial based error is computed in polling loop.  Pick up the value and use it here */
//		nErrorRate = strAlltime.nPolyOffset;

		/* The error for the time interval is the error rate times the time duration */
		nErrorRate *= TICKUPDATETIMESECS;	//

		/* Sum errors to accumulator */
		strAlltime.nTickErrAccum += nErrorRate; 	// Running accumulation of estimated error and adjustments made

		/* See if accumulated error over the threshold in the freq-too-low side */
		if (strAlltime.nTickErrAccum > (NS_TIME_ERR_PER_ADJUSTABLE_TICK/2))
		{ // Drop a tick (freq is low and we are getting behind...the usual case)
			temp = (strAlltime.nTickErrAccum/NS_TIME_ERR_PER_ADJUSTABLE_TICK) + 1;	// Number of ticks adjust
			strAlltime.nTickAdjust   -= temp;				 	// Increment number of ticks to drop
			strAlltime.nTickErrAccum -= temp * NS_TIME_ERR_PER_ADJUSTABLE_TICK; 	// Subtract duration of all ticks dropped
		}
		else
		{ // See if accumulated error over the threshold in the freq-too-high side 
			if (strAlltime.nTickErrAccum < (-NS_TIME_ERR_PER_ADJUSTABLE_TICK/2))
			{ // Add a tick (freq is too high and we are getting ahead...not the case for the first three boards built)
			// NOTE signs: 'temp' will be negative in the following--
				temp = (strAlltime.nTickErrAccum/NS_TIME_ERR_PER_ADJUSTABLE_TICK) + 1;// Number of ticks adjust
				strAlltime.nTickAdjust   -= temp;				// Increment number of ticks to add
				strAlltime.nTickErrAccum -= NS_TIME_ERR_PER_ADJUSTABLE_TICK; 	// Add duration of all ticks added
			}
		}
		TickAdjustflg = 1;	// This causes '32KHz_p1.c' to pick up 'strAlltime.nTickAdjust' on next 2048-rate tick
	}
	return;
}
/* 
====================================================================================
The following routines in this file are called at the level of the main polling loop        
====================================================================================
 ******************************************************************************
 * void RTC_tickadjust_entering_standby(void);
 * @brief	: Setup before going into STANDBY
 ******************************************************************************/
 
void RTC_tickadjust_entering_standby(void)
{
	unsigned int uiRtc_cnt;
	union LL_L_S strU;

	/* Synchronize with rtc interrupt ticks. */
	uiRtc_cnt = p1_RTC_tick_synchronize();	// Do the sync'ing loops (@1)
	
	/* At this point we have about 120 usec to accomplish the following work with the RTC_CNT */

	/* Note: backup registers are 16b and the address are not consecutive, so each must be done separately. */
	/* Pack all the needed values up and store in the (limited) backup register space */

	/* Compute difference between tick-counter and RTC_CNT.  Shift tick-counter left by 2 to put at the 
           8192 rate, and or in the software divide by 4 count, thus putting it on the same basis as the
           RTC_CNT register.  

	   Note: this saves the low order ticks (sPreTick).  It could be (!) that sPreTick is always zero, i.e. it should
           remain in synch with the RTC_CNT register.  If it doesn't then something is wrong, so by
           including it in saved values during STANDBY it can be checked. */

	/* Shift left the current tick count 2 bits, and save the pre-tick count (2 bits).  This results in a long long
           where the lower 32 bits are on the same basis as the RTC_CNT register, however the time is offset from the 
           RTC_CNT by that needed to put the time in linux time format. */

	//                                  shift left 2                                    and 0x03
	strU.ull = ( (strAlltime.SYS.ull << PRL_DIVIDE_PRE_ORDER) | (strAlltime.sPreTick & (PRL_DIVIDE_PRE-1)) );		

	/* RTC_CNT (32b) at the time of these saves.  When the unit comes out of STANDBY the difference bewteen the
	   saved count and the RTC_CNT register is the number of ticks that transpired and is then used to update the
           system tick time count (which is also saved) */
	BKP_DR1  = (uiRtc_cnt >> 16); 	// 0x1234 of 0x12345678
	BKP_DR2  = uiRtc_cnt;		// 0x5678 of 0x12345678
	
	/* Save SYS counter time with imbedded sPreTick (40 bits).  We can drop the high order byte since the 
	   linux time format EPOCH is offset.  40 bits allows more years than the useful life of this device. */
	BKP_DR3  = (strU.us[0]);
	BKP_DR4  = (strU.us[1]);
	BKP_DR5  = (strU.us[2]);
//union LL_L_S  tt = strU;

	/* Save current time error accumulator in backup registers */
	BKP_DR6  = (strAlltime.nTickErrAccum >> 16); 		// 0x1234 of 0x12345678
	BKP_DR7  =  strAlltime.nTickErrAccum;			// 0x5678 of 0x12345678
//long ee = strAlltime.nTickErrAccum;

	/* Save next time to check for a time adjustment in */
	BKP_DR8  = (strAlltime.uiNextTickAdjTime >> 16); 	// 0x1234 of 0x12345678
	BKP_DR9  =  strAlltime.uiNextTickAdjTime;		// 0x5678 of 0x12345678

//long aa = strAlltime.uiNextTickAdjTime;

	/* Save the xtal freq offset calibration.  This is done since in deepsleep mode the "on" time to adjust 
  	   for temperature is too short to bring up the SD Card and read the calibration.  The calibration is
           read from the SD Card when it is brought up by the pushbutton wakeup, or cold start, but then held
           in backup registers for deepsleep */
	BKP_DR10  = (strAlltime.nOscOffset32 & 0xffff);	// 0x5678 of 0x12345678

// Debugging	
//long oo = strAlltime.nOscOffset32;

//printf ("%10x %10x %10x %8d %8d %6d\n\r",tt.ul[0],tt.ul[1],uiRtc_cnt,ee,aa,oo);
//USART1_txint_send();
volatile long long x = 0;	// Wait loop to let USART to send last line before shutdown
while (x++ < 1000000);

	return;
}

/******************************************************************************
 * void RTC_tickadjust_init_exiting_standby(void);
 * @brief	: Setup after coming out of a STANDBY caused reset
 ******************************************************************************/
unsigned int Debug_dif;

void RTC_tickadjust_init_exiting_standby(void)
{
	unsigned int uiRtc_cnt;
	unsigned int uitemp;
	union LL_L_S strU;

	/* Synchronize with rtc interrupt ticks.
           We sync to the low order two bits, since the software is dividing by 4 
           before incrementing the tick-counter. */
	
	/* Synchronize with rtc interrupt ticks. */
	uiRtc_cnt = p1_RTC_tick_synchronize();	// Do the sync'ing loops (@1)

	/* At this point we have about 120 usec to accomplish the following work with the RTC_CNT */
	/* Assemble 32b words from the 16b backup registers.  Pay attention to signed & unsigned. */

	/* Restore tick counter (unsigned long long) */
	/* Restore difference between linux format time and tick counter */
	strU.us[0] = BKP_DR3;
	strU.us[1] = BKP_DR4;
	strU.us[2] = BKP_DR5;
	strU.us[3] = 0;

	/* Get RTC_CNT time when the backup registers were set before going to deepsleep */
	uitemp = (BKP_DR2 & 0xffff) | (BKP_DR1 << 16);	// RTC_CNT when values were saved

	/* Bring SYS tick counter (which was saved before going to STANDBY) up-to-date */
	strAlltime.SYS.ull = strU.ull + (uiRtc_cnt - uitemp); 	// Amount of time transpired between "now" and "saved"

// Debugging
Debug_dif = (uiRtc_cnt - uitemp);

	/* Extract low order two bits, and adjust for 2048 rate */
	strAlltime.sPreTick = ( strAlltime.SYS.ul[0] & (PRL_DIVIDE_PRE-1) );	// (& 0x03) Extract low order 2 bits
	strAlltime.SYS.ull  = ( strAlltime.SYS.ull >> (PRL_DIVIDE_PRE_ORDER) );	// Shift right 2

	/* At this point the SYS tick counter is setup */

	/* Restore current time error accumulator in backup registers */
	strAlltime.nTickErrAccum = (BKP_DR7 & 0xffff) | (BKP_DR6 << 16);

	/* Restore next time to check for a time adjustment in */
	strAlltime.uiNextTickAdjTime = (BKP_DR9 & 0xffff) | (BKP_DR8 << 16); 	// 0x1234 of 0x12345678

	/* Get the xtal freq offset calibration */
	strAlltime.nOscOffset32 = (signed short)BKP_DR10;

// Debugging
//printf ("%10x %10x\n\r",strU.ul[0],strU.ul[1]);
//USART1_txint_send();

	return;
}

/******************************************************************************
 * int rtc_offset_computation(unsigned int uiTIM1, unsigned int uiTIM2);
 * @brief	: Compute osc offset, given gps TIM1, TIM2 tick data
 * @param	: uiTIM1: processor ticks between 512 interrupts of 32768Hz/64-> on MCO pin
 * @param	: uiTIM2: processor ticks between GPS 1 PPS interrupts 
 * @return	: PPM*100
 ******************************************************************************/
int rtc_offset_computation(unsigned int uiTIM1, unsigned int uiTIM2)
{
	int nNumerator = (int)(2*uiTIM1 - uiTIM2);
	int nRecip = (int)uiTIM2 / nNumerator;
	return (100000000 / nRecip);
}
/******************************************************************************
 * int rtc_tick_edit(unsigned int uiTIM1, unsigned int uiTIM2);
 * @brief	: Check for out-of-range data, given gps TIM1, TIM2 tick data
 * @param	: uiTIM1: processor ticks between 512 interrupts of 32768Hz/64-> on MCO pin
 * @param	: uiTIM2: processor ticks between GPS 1 PPS interrupts 
 * @return	: 0 = OK, 1 = fail;
 ******************************************************************************/
/* Nominal bus freq for TIM1 24000000; for TIM2 48000000 */
#define TIM2HILIMIT	48005000	// Max allowable
#define TIM2LOLIMIT	47995000	// Min allowable
#define TIM1HILIMIT	24005000	// Max allowable
#define TIM1LOLIMIT	23995000	// Min allowable

int rtc_tick_edit(unsigned int uiTIM1, unsigned int uiTIM2)
{
	if (uiTIM1 > TIM1HILIMIT) return 1;
	if (uiTIM1 < TIM1LOLIMIT) return 1;
	if (uiTIM2 > TIM2HILIMIT) return 1;
	if (uiTIM2 < TIM2LOLIMIT) return 1;
	return 0;
}
/******************************************************************************
 * void	rtc_tick_adjust_ave_reset(void);
 * @brief	: Reset counts for averaging xtal calibration
 ******************************************************************************/
static unsigned int rtc_ave_accum;
static unsigned int rtc_ave_count;

void	rtc_tick_adjust_ave_reset(void)
{
	rtc_ave_accum = 0;
	rtc_ave_count = 0;
	return;
}
/******************************************************************************
 * int rtc_tick_adjust_average(int uiOffset);
 * @brief	: Compute average of uiOffset
 * @param	: uiOffset: offset value (returned from 'rtc_offset_computation')
 * @return	: current average * 10
 ******************************************************************************/
static int rtc_ave_count_latest;
int rtc_tick_adjust_average(int uiOffset)
{
	rtc_ave_accum += uiOffset;
	rtc_ave_count += 1;
	if (rtc_ave_count >= 10)
	{
		rtc_ave_count_latest = (rtc_ave_accum/rtc_ave_count)*10;
		rtc_ave_accum = 0;
		rtc_ave_count = 0;
	}
	return rtc_ave_count_latest;
}
/******************************************************************************
 * struct TWO rtc_tick_adjust_filter(int nOffset);
 * @brief	: Compute cic filtered value of uiOffset (ppm*100)
 * @param	: uiOffset: offset value (returned from 'rtc_offset_computation')
 * @return	: {0,0} no new data; {1,value} new data (ppm*1000)
 ******************************************************************************/
#define TICKADJUSTDECIMATE	4	// Decimation number for filtering offsets
#define TICKADJUSTDISCARD	2	// Number of readings to discard after resetting
#define TICKADJUSTSCALE		9	// Number of bits to shift right
static struct CICLN2M3 cic_tickadj;

struct TWO rtc_tick_adjust_filter(int nOffset)
{
	struct TWO two = {0,0};

	if (cic_tickadj.usDecimateNum != TICKADJUSTDECIMATE)
	{
		cic_tickadj.usDecimateNum = TICKADJUSTDECIMATE;
		cic_tickadj.usDiscard = TICKADJUSTDISCARD;
	}
	cic_tickadj.nIn = nOffset;
	
	/* Return if no new data */
	if ( (cic_filter_l_N2_M3(&cic_tickadj)) == 0 ) return two;

	/* Setup the return struct */
	two.n1 = 1;	// Show that new data is being returned
	/* Scale the output for filter gain, and scale up by 10x */
	two.n2 = ((cic_tickadj.lout >> 4)*10) >> (TICKADJUSTSCALE-4);	

	return two;
}
/******************************************************************************
 * struct TWO rtc_tick_filtered_offset(unsigned int uiDiffTim1, unsigned int uiDiffTim2);
 * @brief	: Compute cic filtered value of uiOffset (ppm*1000);
 * @param	: DiffTim1: bus ticks between 1 sec of 32 KHz interrupts (TIM1)
 * @param	: DiffTim2: bus ticks between 1 PPS interrupts (TIM2)
 * @return	: n1 = 0: no new filtered data, n2 = unfiltered offset;
 * @return	: n1 = 1; new filtered data; n2 = filtered offset
 ******************************************************************************/
#define DISCARDFILTERPPMCT	6	// Number of readings to discard while filter initially "comes up"
static unsigned int uiDiscardPPM;	// Counter for discarding initial calibration filter output

struct TWO rtc_tick_filtered_offset(unsigned int uiDiffTim1, unsigned int uiDiffTim2)
{
	struct TWO two = {0,0};
	int temp;

	/* If number of processor ticks for TIM1, and TIM2 are reasonable, compute ppm*100 error for 32 KHz osc */
	if (rtc_tick_edit(uiDiffTim1, uiDiffTim2) == 0)	// Check for valid values
	{ // Here, OK.  Now compute the error.

		/* First use DiffTim1, DiffTim2 to compute error */
		temp = rtc_offset_computation(uiDiffTim1,uiDiffTim2);

		/* Run offset through a 3 section cic filter */
		two = rtc_tick_adjust_filter(temp);
		if ( two.n1 == 1)	// Does the filter have a new output?
		{ // Here, yes, there is new data
			/* Skip until we have sufficient readings filtered */
			if (uiDiscardPPM < (DISCARDFILTERPPMCT-1)) 
			{ // Here, filtering is still initializing
				uiDiscardPPM += 1; 	// Count upwards then stop				
			}
		}
		else
		{
			two.n2 = temp;
		}
	}
	return two;
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
/******************************************************************************
Coefficients for 3rd order polynomial computation.
 ******************************************************************************/
// 8 MHz processor oscillator
// From 2011105_2222.txt data/regression
const float MHz8_xtal_B[4]= 
{
 0.1309106933004727E+04,
 0.1497314188920444E-01,
-0.4098144829541804E-04,
 0.4772734187818172E-08
};

// 8 MHz processor oscillator
// From 20111111_1610_A.txt data/regression
const float MHz8_xtal_A[4]= 
{
 0.1141122138554342E+04,
 0.2290186694577369E+00,
-0.1176333870602867E-03,
 0.1398297745528303E-07
};

// 32 KHz RTC oscillator
const float KHz32_xtal[4]= 
{
 0.1564747299242941E+05,
-0.1060771088585940E+02,
 0.2501983122394449E-02,
 0.6708378364956507E-07
};

/******************************************************************************
 * int RTC_tickadjust_polynomial_n(const float p[], int nX);
 * @brief	: Compute estimated *total* freq error in parts per billion
 * @param	: Pointer to float array with coefficients
 * @param	: Temperature in degrees C * 100.
 * @return	: ppb estmiated offset
 ******************************************************************************/
int RTC_tickadjust_polynomial_n(const float p[], int nX)
{
	float x  = nX;
	int temp = (p[0] + x * (p[1] + x * (p[2] + p[3] * x)));	
	return temp;
}

/******************************************************************************
 * void RTC_tickadjust_poly_compute(void);
 * @brief	: Compute estimated *total* freq error in parts per billion for xtal osc
 * @return	: ppb estmiated offset
 ******************************************************************************/
/*
This routine is in the polling loop in 'p1_normal_run.c' (@5).  The routine
leaves the lastest temperature and oscillator error values in the struct ALLTIME (@6)
for everyone, even those not authorized by the highest authority, to use.
*/

void RTC_tickadjust_poly_compute(void)
{
	struct PKT_PTR	pp;
	struct PKT_BATTMP *pp_batt;

		pp = adc_packetize_get_battmp_tickadujst();	// 
		if (pp.ct == 0 )	// New adc data for monitoring purposes? (@1)
		{ // Here no new data
			return;
		}
		else
		{
			pp_batt = (struct PKT_BATTMP*)pp.ptr;	// Convert to packet ptr (@1)
	
			/* Average thermistor ADC reading (xxxxxx.x)  (@2) */
			strAlltime.uiAdcTherm = pp_batt->adc[0];
	
			/* Get the thermistor temperature */
			strAlltime.uiThermtmp = adctherm_cal(strAlltime.uiAdcTherm);	// Table look up of temp (deg C) given adc reading
	
			/* Adjust temp for calibration offset  */
			strAlltime.uiThermtmp = strAlltime.uiThermtmp+strDefaultCalib.tmpoff;

			/* Compute the 32 KHz xtal error using temperature and polynomial, but with tweak adjustments.  */
			strAlltime.nPolyOffset32 = RTC_tickadjust_polynomial_n(KHz32_xtal,strAlltime.uiThermtmp-strDefaultCalib.xtal_t) - strDefaultCalib.xtal_o;

			/* Compute the 8 MHz xtal error using the temperature and polynomial, but with tweak adjustment */
			strAlltime.nPolyOffset8 = RTC_tickadjust_polynomial_n(MHz8_xtal_A, strAlltime.uiThermtmp) - strDefaultCalib.xtal_o8;
		}
		return;
}
