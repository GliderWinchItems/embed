/************************ COPYRIGHT 2011 **************************************
* File Name          : tickadjust.h
* Hacker	     : deh
* Date First Issued  : 08/12/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Adc thermistor reading adjustment of tick time
*******************************************************************************/
/*
'RTC_tickadjust' is expected to be called from the RTC interrupt processing chain.

'RTC_tickadjust_init_exiting_standby' is called in the reset initialization when
the reset is one of coming out of STANDBY (and not a cold power up) reset.

'RTC_tickadjust_entering_standby' is called when setting up to enter STANDBY.
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TICKADJUST1
#define __TICKADJUST1

/* Includes ------------------------------------------------------------------*/
#include "libopenstm32/common.h"	// Has things like 'u16' defined
#include "libopenstm32/gpio.h"
#include "libopenstm32/spi.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/rtc.h"

#define TICKUPDATETIMESECS	10	// Number of seconds between updates

/* Tick count for time when RTC tick adjustment check is made */
#define UPDATEINCREMENT	(TICKUPDATETIMESECS * ALR_INCREMENT)	// (10 secs) * (2048 ticks/sec)


/* Jam the new reading if the number of ticks between the SYS and GPS times exceed this */
#define TICKJAMTOLERANCE	1024			// Number ticks (1/2 sec)

/* Temperature error table adjustment ratio */
#define	TEMPTABLEADJUST	1000	// Multiply by this, divide by 100

/******************************************************************************/
void RTC_tickadjust(unsigned int uiAdcTherm);
/* @param	: Adc reading for thermistor.
 * @brief	: Adjust RTC tick counter for accumulation of errors
 ******************************************************************************/
void RTC_tickadjust_entering_standby(void);
/* @brief	: Setup before going into STANDBY
 ******************************************************************************/
void RTC_tickadjust_init_exiting_standby(void);
/* @brief	: Setup after coming out of a STANDBY caused reset
 ******************************************************************************/
void rtc_tickadjust(void);
/* @brief	: Pick up ADC therm reading and call RTC_tickadjust
 ******************************************************************************/
int rtc_offset_computation(unsigned int uiTIM1, unsigned int uiTIM2);
/* @brief	: Compute osc offset, given gps TIM1, TIM2 tick data
 * @param	: uiTIM1: processor ticks between 512 interrupts of 32768Hz/64-> on MCO pin
 * @param	: uiTIM2: processor ticks between GPS 1 PPS interrupts 
 * @return	: PPM*100
 ******************************************************************************/
int rtc_tick_edit(unsigned int uiTIM1, unsigned int uiTIM2);
/* @brief	: Compute osc offset, given gps TIM1, TIM2 tick data
 * @param	: uiTIM1: processor ticks between 512 interrupts of 32768Hz/64-> on MCO pin
 * @param	: uiTIM2: processor ticks between GPS 1 PPS interrupts 
 * @return	: 0 = OK, 1 = fail;
 ******************************************************************************/
void	rtc_tick_adjust_ave_reset(void);
/* @brief	: Reset counts for averaging xtal calibration
 ******************************************************************************/
int rtc_tick_adjust_average(int uiOffset);
/* @brief	: Compute average of uiOffset
 * @param	: uiOffset: offset value (returned from 'rtc_offset_computation')
 * @return	: current average * 10
 ******************************************************************************/
struct TWO rtc_tick_adjust_filter(int nOffset);
/* @brief	: Compute cic filtered value of uiOffset (ppm*100)
 * @param	: uiOffset: offset value (returned from 'rtc_offset_computation')
 * @return	: {0,0} no new data; {1,value} new data (ppm*1000)
 ******************************************************************************/
struct TWO rtc_tick_filtered_offset(unsigned int uiDiffTim1, unsigned int uiDiffTim2);
/* @brief	: Compute cic filtered value of uiOffset (ppm*1000);
 * @param	: DiffTim1: bus ticks between 1 sec of 32 KHz interrupts (TIM1)
 * @param	: DiffTim2: bus ticks between 1 PPS interrupts (TIM2)
 * @return	: n1 = 0: no new filtered data, n2 = unfiltered offset;
 * @return	: n1 = 1; new filtered data; n2 = filtered offset
 ******************************************************************************/
void gps_tickadjust(void);
/* @brief	: Adjust tick time to GPS
 ******************************************************************************/
int RTC_tickadjust_polynomial(int nX);
/* @brief	: Compute estimated *total* freq error in parts per billion
 * @param	: Temperature in degrees C * 100.
 * @return	: ppb estmiated offset
 ******************************************************************************/
void RTC_tickadjust_poly_compute(void);
/* @brief	: Compute estimated *total* freq error in parts per billion
 * @param	: Temperature in degrees C * 100.
 * @return	: ppb estmiated offset
 ******************************************************************************/
int RTC_tickadjust_polynomial_n(const float p[], int nX);
/* @brief	: Compute estimated *total* freq error in parts per billion
 * @param	: Pointer to float array with coefficients
 * @param	: Temperature in degrees C * 100.
 * @return	: ppb estmiated offset
 ******************************************************************************/



/* Address rtc_tickadjust  will go to upon completion (@5) */
extern void 	(*rtc_tickadjustdone_ptr)(void);// Address of function to call to go to for further handling under 

/* Difference between our in-memory rtc tickcounter and the RTC_CNT counter */
extern int nTickDiff;

/* This shows the error rate estimate that is due to temperature */
extern int nAdcppm_temp_latest;	// Latest calibration table lookup (ppm*1000)

/* Used with '32KHz_p1.c' for adjusting time to GPS */
extern unsigned char DIFtmpctr;		// Increments each GPS storing into 'ullDIFtmp'
extern unsigned char DIFjamflag;		// Flag: 0 = do not jam, not zero = jam 
extern unsigned char TickAdjustflg;		// Increments each update of strAlltime.nTickAdjust
extern unsigned char TickAdjustOTO;		// One-Time-Only.  Set to 1 tells deepsleep_run that it is OK to shutdown


/* Debugging */
void Debug_bkp_save(void);
extern unsigned short Debug_bkp[12];
extern unsigned int Debug_dif;
extern int Debug_nErrorO;
extern int nTickErrAccum;		// Running accumulation of error
extern int Debug_nError;
extern int Debug_nErrorT;
extern int Debug_nErrorO;



#endif 

