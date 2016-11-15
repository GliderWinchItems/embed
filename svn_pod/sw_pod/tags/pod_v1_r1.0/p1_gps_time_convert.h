/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : p1_gps_time_convert.h
* Hackeroos          : deh
* Date First Issued  : 07/18/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Convert NMEA record time to something else(?)
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __P1_GPSTIMECONVT
#define __P1_GPSTIMECONVT

#include "p1_common.h"


/******************************************************************************/
unsigned int gps_time_stamp(struct TIMESTAMPGP1 *strTS, struct USARTLB strlb );
/* @brief	: Check for $GPRMC sentence then extract the date/time
 * @param	: strTS - receives extracted time
 * @param	: p - pointer to GPS line
 * @return	: 0 = record is not valid; not zero = the RTC_CNT following sentence EOL
 ******************************************************************************/

/* gps_time_convert.c: Extraction of time from gps lines */
//extern struct TIMESTAMPGP1 strTS1;	// GPS time (@10)

#endif 

