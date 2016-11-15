/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : tickadjust.c
* Hacker	     : deh
* Date First Issued  : 08/20/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Adc thermistor reading adjustment of tick time
*******************************************************************************/
/*
Strategy: every X seconds (e.g. 10 seconds) the error is added to an accumulator.  When
the accumulator passes the amount of time that one-half tick dropped, or one-half tick added,
a tick adjustment is made.

The error accumulator and the next time to check are maintained in the BKP registers 
which are held during STANDBY.

For "cold starts" (power down/up reset), the accumulator is started at zero and the 
next time to check initialized to the time increment.

The association of tick counts to real time is not made in this routine.  The gps 1_PPS
(rising edge) causes a TIM2 interrupt which stores the current tick count.  The real time
that follows the 1_PPS pulse is extract from the ASCII data and stored in the SD card
along with the tick count saved at the TIM2 input capture interrupt.

*/

#include "libopenstm32/rtc.h"			// For realtime clock
#include "libopenstm32/bkp.h"			// For rtc backup registers
#include "libmiscstm32/clockspecifysetup.h"	// For clock setup
#include "libmiscstm32/systick1.h"		// SYSTICK routine


#include "tickadjust.h"

#include "gps_1pps.h"		// Timer routines for this app
#include "32KHz_p1.h"		// 32 KHz osc
#include "calibration.h"	// Calibration structure 
#include "adcppm_cal.h"		// Calibration routine for temp error

/* 8E9/2048 -> 3906250 (ns/8) */
#define TIMEERRORFORONETICK	8000000000/ALR_INC_ORDER	// Duration of one tick
#define SCALEPPM	80*TICKUPDATETIMESECS	// Scale ppm to match scale for duration of one tick

#define UPDATEINCREMENT	TICKUPDATETIMESECS*ALR_INC_ORDER	// (10 secs) * (2048 ticks/sec)

/* Default calibrations for various devices (see ../devices/calibration.c,.h) */
extern const struct CALBLOCK strDefaultCalib;		// Default calibration

/* Holds the RTC CNT register count that is maintained in memory when power is up */
/* This counter is incremented by the RTC Secf interrupts (see ../devices/32KHz.c RTC_IRQHandler) */
//extern volatile unsigned int strAlltime.SYS.ull;	// This mirrors the RTC CNT register, and is updated each RTC Secf interrupt
extern short sPreTickReset;	// Reset count for adjusting time for temperature effects and freq offset


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
*/

/* Error accumulator */
int nTickErrAccum;		// Running accumulation of error

/* Difference between the RTC_CNT register (counter) which counts Secf ticks and the
in-memory tick counter.  The difference is the accumulated adjustment of the time error
due to 1) freq offset of the 32 KHz osc, and 2) the tick adjustment due to effect of
temperature on the osc freq. */

int nTickDiff;


/* The following holds the next time to do the tick adjustment check.
This time is also used to set the alarm register for wakeup out of STANDBY. */
//unsigned int ullNextTickAdjTime;

/* Address this routine will go to upon completion */
void 	(*rtc_tickadjustdone_ptr)(void);// Address of function to call to go to for further handling under RTC interrupt

/* Flag = 0 when normal RTC_PRL is loaded.  Not zero when a time adjustment count is loaded.  This flag can be used
when going to STANDBY to assure that STANDBY is not executed with the RTC_PRL counter with a time adjustment count */
char cRTC_PRL_flag;

/******************************************************************************
 * void RTC_tickadjust(unsigned int uiAdcTherm);
 * @param	: Adc reading for thermistor.
 * @brief	: Adjust RTC tick counter for accumulation of errors
 ******************************************************************************/
void RTC_tickadjust(unsigned int uiAdcTherm)
{
	int nError;

	/* If on the previous RTC interrupt the RTC_PRL counter was changed the flag will be on */
	if (cRTC_PRL_flag != 0)
	{ // Here, we need to set the RTC_PRL register back to the normal count. (divide by 16)
		cRTC_PRL_flag = 0;		// Reset flag to show PRL register no longer has an adjustment count.
		p1_RTC_reg_load_prl(PRL_DIVIDER-1);// Load PRL register (15 causes countdown of 16)(see ../devices/32KHz.c)
	}
	else
	{ // Here, the situation is normal, i.e. no adjustment in progress
		/* Is it time to check if a tick adjustment needs to be made */
		if (strAlltime.SYS.ul[0] >= strAlltime.uiNextTickAdjTime)	// In memory RTC_CNT versus update time
		{ // Here, it is time to make a check
			strAlltime.uiNextTickAdjTime += UPDATEINCREMENT;	// Advance to next update time

			/* Get error due to temperature (table lookup.  See ../devices/adcppm_cal.c) */
			nError = adcppm_cal(uiAdcTherm,strDefaultCalib.adcppm );// Convert ADC reading to freq error (ppm * 100)

			/* Add in fixed error of osc */
			nError += strDefaultCalib.ppmoff;	// Note: this is signed.  Positive temp error is always in "down" direction.

			/* Sum to accumulator */
			// The errors in the tables are in ppm*100.  Scale them up to the scale used for time ticks.
			strAlltime.nTickErrAccum += nError * SCALEPPM;	// Running accumulation of estimated error and adjustments made

			/* See if error is large enough that a tick adjustment is needed */
			if (strAlltime.nTickErrAccum > TIMEERRORFORONETICK/2)
			{ // Add a tick (freq is low and we are getting behind...the usual case)
				sPreTickReset = 1; 		// Drop a tick: Start next divider counter from 1 instead of zero
				p1_RTC_reg_load_prl(PRL_DIVIDER-2);// Load PRL register (14 causes countdown of 15)(see ../devices/32KHz.c)
				strAlltime.nTickErrAccum -= TIMEERRORFORONETICK;// Adjust accumulator for time adjustment
			}
			else
			{ // Drop a tick (freq is too high and we are getting ahead...not the case for the first board built)
				sPreTickReset = -1; 		// Add a tick: Start next divider counter from -1 instead of zero
				p1_RTC_reg_load_prl(PRL_DIVIDER);	// Load PRL register (16 causes countdown of 17)(see ../devices/32KHz.c)
				strAlltime.nTickErrAccum += TIMEERRORFORONETICK;// Adjust accumulator for time adjustment
			}
		}
	}

	return;
}


/******************************************************************************
 * void RTC_tickadjust_entering_standby(void);
 * @brief	: Setup before going into STANDBY
 ******************************************************************************/
 
void RTC_tickadjust_entering_standby(void)
{
	unsigned int uiRtc_cnt;
	union LL_L_S strU;

	/* Synchronize with rtc interrupt ticks. */
	uiRtc_cnt = p1_RTC_tick_synchronize();	// Do the sync'ing loops
	
	/* At this point we have about 120 usec to accomplish the following work with the RTC_CNT */
	/* Assemble 32b words from the 16b backup registers.  Pay attention signed/unsigned. */

		/* Get the difference between our in-memory tick counter and the RTC_CNT.  This difference
           will accumulate due to time adjustments that account for offset and temp drift.  This
           difference is saved in the backup registers.  Updating the RTC_CNT register is
           complicated so that it is problematic that it can be synchronized without adding or 
	   dropping a tick */

	/* Get RTC_CNT and lop off low order ticks.  We can do this without loss since the 
           synchronization above is looking at the tick-counter which updates when the software
           divide by four reaches zero.  Even if*/
 	   
	/* Compute difference between tick-counter and RTC_CNT.  Shift tick-counter left by 2 to put at the 
           8192 rate, and or in the software divide by 4 count, thus putting it on the same basis as the
           RTC_CNT register.  

	   Note: this saves the low order ticks (sPreTick).  It could be (!) that sPreTick is always zero, i.e. it should
           remain in synch with the RTC_CNT register.  If it doesn't then something is wrong, so by
           including it in saved values during STANDBY it can be checked. */
	strU.ull = ( (strAlltime.SYS.ull << PRL_DIVIDE_PRE_ORDER) | (strAlltime.sPreTick & (PRL_DIVIDE_PRE-1)) );		

	/* Pack all the needed values up and store in the (limited) backup register space */

	/* RTC_CNT at the time of these saves */
	BKP_DR1 = (uiRtc_cnt << 16); 	// 0x1234 of 0x12345678
	BKP_DR2 = (uiRtc_cnt & 0xffff);	// 0x5678 of 0x12345678
	
	/* Save SYS counter time with imbedded sPreTick (48 bits) */
	BKP_DR3 = (strU.us[0]);
	BKP_DR4 = (strU.us[1]);
	BKP_DR5 = (strU.us[2]);

	/* Save tick difference between SYS counter and LNX time (48 bits) */
	BKP_DR6 = (strAlltime.DIF.us[0] & 0xffff);	// 
	BKP_DR7 = (strAlltime.DIF.us[1] & 0xffff); 	//
	BKP_DR8 = (strAlltime.DIF.us[2] & 0xffff);	//

	/* Save current time error accumulator in backup registers */
	BKP_DR9  = (strAlltime.nTickErrAccum << 16); 	// 0x1234 of 0x12345678
	BKP_DR10 = (strAlltime.nTickErrAccum & 0xffff);	// 0x5678 of 0x12345678

	/* Save next time to check for a time adjustment in */
	BKP_DR11  = (strAlltime.uiNextTickAdjTime << 16); 	// 0x1234 of 0x12345678
	BKP_DR12  = (strAlltime.uiNextTickAdjTime & 0xffff);	// 0x5678 of 0x12345678

	return;
}

/******************************************************************************
 * void RTC_tickadjust_init_exiting_standby(void);
 * @brief	: Setup after coming out of a STANDBY caused reset
 ******************************************************************************/
void RTC_tickadjust_init_exiting_standby(void)
{
	unsigned int uiRtc_cnt;
	unsigned int uitemp;

	/* Synchronize with rtc interrupt ticks.
           We sync to the low order two bits, since the software is dividing by 4 
           before incrementing the tick-counter. */
	
	/* Synchronize with rtc interrupt ticks. */
	uiRtc_cnt = p1_RTC_tick_synchronize();	// Do the sync'ing loops

	/* At this point we have about 120 usec to accomplish the following work with the RTC_CNT */
	/* Assemble 32b words from the 16b backup registers.  Pay attention signed/unsigned. */

	/* Restore tick counter (unsigned long long) */
	/* Restore difference between linux format time and tick counter */
	strAlltime.SYS.us[0] = BKP_DR3;
	strAlltime.SYS.us[1] = BKP_DR4;
	strAlltime.SYS.us[2] = BKP_DR5;
	strAlltime.SYS.us[3] = 0;

	/* Extract low order two bits, and adjust for 2048 rate */
	strAlltime.sPreTick = ( strAlltime.SYS.ul[0] & (PRL_DIVIDE_PRE-1) );	// Extract low order 2 bits
	strAlltime.SYS.ull  = ( strAlltime.SYS.ull >> (PRL_DIVIDE_PRE-1) );	// Shift right 2

	/* Update SYS tick counter for time between saving values and restoring values */
	uitemp = (BKP_DR2 & 0xffff) | (BKP_DR1 << 16);	// RTC_CNT when values were saved

	/* Bring SYS tick counter (which was saved before going to STANDBY) up-to-date */
	strAlltime.SYS.ull += (uiRtc_cnt - uitemp); 	// Amount of time transpired between "now" and "saved"
	
	/* Restore difference between linux format time and tick counter */
	strAlltime.DIF.us[0] = BKP_DR6;
	strAlltime.DIF.us[1] = BKP_DR7;
	strAlltime.DIF.sl[2] = BKP_DR8;	// This should sign extend to '.s[3]

	/* Setup Linux time */
	strAlltime.LNX.ull = strAlltime.SYS.ull + strAlltime.DIF.ull;

	/* Restore current time error accumulator in backup registers */
	strAlltime.nTickErrAccum = (BKP_DR10 & 0xffff) | (BKP_DR9 << 16);

	/* Restore next time to check for a time adjustment in */
	strAlltime.uiNextTickAdjTime = (BKP_DR12 & 0xffff) | (BKP_DR11 << 16); 	// 0x1234 of 0x12345678



	return;
}
/******************************************************************************
 * void rtc_tickadjust(void);
 * @brief	: Pick up ADC therm reading and call RTC_tickadjust
 ******************************************************************************/
void rtc_tickadjust(void)
{
//	RTC_tickadjust(   );
	/* If the address for the next function is not set, simply return. */
	if (rtc_tickadjustdone_ptr != 0)	// Having no address for the following is bad.
		(*rtc_tickadjustdone_ptr)();	// Go do something (e.g. poll the AD7799)
	return;
}
