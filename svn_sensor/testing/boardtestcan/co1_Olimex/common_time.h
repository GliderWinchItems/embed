/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : common_time.h
* Hackerees          : deh
* Date First Issued  : 01/26/2013
* Board              : STM32F103RxT6_pod_mm
* Description        : Olimex CO: time related "stuff"
*******************************************************************************/

#ifndef __COMMON_TIME_OLI_CO
#define __COMMON_TIME_OLI_CO

#include "common_misc.h"
#include <time.h>

/* GPS codes */
// 0 = no gps, 1 = Garmin_18X_5_Hz, 2 = u-blox_NEO-6M, 3 = Garmin_18X_1_Hz
enum gpstype
{
	NO_GPS		= 0,
	GARMIN_18X_5_HZ	= 1,	// if cGPStype == GARMIN_18X_5_HZ)
	UBLOX_NEO_6M	= 2,	// if cGPStype == UBLOX_NEO_6M)
	GARMIN_18X_1_HZ	= 3	// if cGPStype == GARMIN_18X_1_HZ)
};
extern char cGPStype;

/* This definition shifts the date/time of the Linux format time */
#define PODTIMEEPOCH	1318478400	// Offset from Linux epoch to save bits

/* This defines the number of bytes needed in the time */
#define	PODTIMEEPOCHBYTECT	5	// 40 bits will get use to year 2028


/* Time counts used by all */
struct ALLTIMECOUNTS
{
	union LL_L_S		SYS;		// Time stamp: Shifted epoch linux time at 64 ticks per sec.
	union LL_L_S		GPS;		// GPS: linux time divined from the ascii of last GPS fix
};


/* The following construct is left over from the POD stuff */
#define GPSARRAYSIZE	14	// Size of time|date
struct TIMESTAMPGP1
{
	char g[GPSARRAYSIZE];		// GPS time in ASCII array: HHMMSSMMDDYY(hour, min, sec, day, month, year)
	struct ALLTIMECOUNTS alltime;	// All the time counters and differences
};


extern volatile struct ALLTIMECOUNTS	strAlltime;	// All the time stuff lumped into this

extern u8  gps_poll_flag;	// 0 = idle; 1 = update .SYS time
extern u32 gps_poll_flag_ctr;	// Running count of GPS v SYS time updates

#endif 


