/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : gps_time_convert.h
* Hackeroos          : deh
* Date First Issued  : 07/18/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Convert NMEA record time to something else(?)
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GPSTIMECONVT
#define __GPSTIMECONVT

#include <time.h>
#include "libusartstm32/usartallproto.h"
#define GPSARRAYSIZE	12	// Size of SSMMHHDDMMYY

struct TIMESTAMPG
{
	unsigned char	id;	// ID of packet
	char g[GPSARRAYSIZE];	// GPS time in ASCII array: SSMMHHDDMMYY(sec, min, hour, day, month, year)
	time_t	t;		// GPS time in Linux format time 
	unsigned int cnt;	// RTC_CNT register (2048 ticks per sec)
	unsigned short cnt_x;	// RTC_CNT extension(2048 ticks per sec) 
};

/******************************************************************************/
unsigned int gps_time_stamp(struct TIMESTAMPG *strTS, struct USARTLB strlb );
/* @brief	: Check for $GPRMC sentence then extract the date/time
 * @param	: strTS - receives extracted time
 * @param	: p - pointer to GPS line
 * @return	: 0 = record is not valid; not zero = the RTC_CNT following sentence EOL
 ******************************************************************************/
unsigned int gps_time_ng_fix(struct USARTLB strlb );
/* @brief	: Check for $GPRMC sentence and see of fix is OK
 * @param	: p - pointer to GPS sentence
 * @return	: 0 = record is not valid; not zero = 'V' indicating bad fix
 ******************************************************************************/


/* gps_time_convert.c: Extraction of time from gps lines */
extern struct TIMESTAMPG strTS1;	// GPS time (@10)

#endif 

