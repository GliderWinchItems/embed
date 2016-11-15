/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : p1_gps_time_linux.h
* Hackeroos          : deh
* Date First Issued  : 10/08/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Convert gps|rtc time ticks to linux time format
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __P1_GPSTIMELINUX
#define __P1_GPSTIMELINUX

#include <time.h>

/* This definition shifts the date/time of the Linux format time */
#define PODTIMEEPOCH	1318478400	// Offset from Linux epoch to save bits
//#define PODTIMEEPOCH	0		// *NO* offfset from Linux epoch for debugging
//#define PODTIMEEPOCH	(60	)	// One min ffset from Linux epoch for debugging

/* This defines the number of bytes needed in the time */
#define	PODTIMEEPOCHBYTECT	5	// 40 bits will get use to year 2028

/******************************************************************************/
time_t gps_time_linux_init(struct TIMESTAMPGP1 * pts);
/* @brief	:
 * @param	: strTS - receives extracted time
 * @param	: p - pointer to GPS line
 * @return	: 0 = record is not valid; not zero = Extracted RTC_CNT following EOL
 ******************************************************************************/

#endif 

