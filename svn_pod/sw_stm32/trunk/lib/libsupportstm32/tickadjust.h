/******************** (C) COPYRIGHT 2011 **************************************
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

/* Address rtc_tickadjust  will go to upon completion (@5) */
extern void 	(*rtc_tickadjustdone_ptr)(void);// Address of function to call to go to for further handling under 

/* Difference between our in-memory rtc tickcounter and the RTC_CNT counter */
extern int nTickDiff;


#endif 
