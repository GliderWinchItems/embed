/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : gps_time_convert1.h
* Hackeroos          : deh
* Date First Issued  : 11/25/2011
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
unsigned int gps_time_stamp1(struct TIMESTAMPG *strTS, struct USARTLB strlb );
/* @brief	: Check for $GPRMC sentence then extract the date/time
 * @param	: strTS - receives extracted time
 * @param	: p - pointer to GPS line
 * @return	: 0 = record is valid; not zero = return error code--
 *  1 = '$' not found
 *  2 = 'GPRMC,' not found in theafter 6 chars following $
 *  3 = Checksums do not match
 *  4 = HHMMSS had some invalid chars
 *  5 = 3rd ',' not present in expected position
 *  6 = 'A' signifying valid fix is not present
 *  7 = 4th ',' not present in expected position
 *  8 = 5th ',' not present in expected position
 *  9 = 6th ',' not present in expected position
 * 10 = 7th ',' not present in expected position
 * 11 = 8th ',' not present in expected position
 * 12 = 9th ',' not present in expected position
 * 13 = 10th ',' not present in expected position
 * 14 = DDMMYY had some invalid chars
 * 15 = 11th ',' not present in expected position
 * 16 = String terinator ('0') not in expected position
 ******************************************************************************/
unsigned int gps_time_ng_fix1(struct USARTLB strlb );
/* @brief	: Check for $GPRMC sentence and see of fix is OK
 * @param	: p - pointer to GPS sentence
 * @return	: 0 = record is not valid; not zero = 'V' indicating bad fix
 ******************************************************************************/


/* gps_time_convert.c: Extraction of time from gps lines */
extern struct TIMESTAMPG strTS1;	// GPS time (@10)

#endif 

