/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : calendar_arith.h
* Author             : deh
* Date First Issued  : 09/17/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Calendar & RTC tick count
*******************************************************************************/
/*
*/
/*
Subroutine call references shown as "@n"--
@1 = svn_pod/sw_stm32/trunk/lib/libsupportstm32/gps_time_convert.h



*/

// The following is the Linux struct for time--
//struct tm {
//    int tm_sec;         /* seconds */
//    int tm_min;         /* minutes */
//    int tm_hour;        /* hours */
//    int tm_mday;        /* day of the month */
//    int tm_mon;         /* month */
//    int tm_year;        /* year */
//    int tm_wday;        /* day of the week */
//    int tm_yday;        /* day in the year */
//    int tm_isdst;       /* daylight saving time */
//};
/*
The following is loaded from the GPS record editing (@1)--
struct TIMESTAMPG
{
	unsigned char	id;	// ID when used as a packet
	char g[12];		// GPS ASCII: YYMMDDHHMMSS (year, month, day, hour, min, sec)
	unsigned int cnt;	// RTC_CNT register (2048 Hz ticks)
};
*/

#include <time.h>
#include "p1_gps_time_convert.h"

short asctoint2d(char * p);

struct tm strTM;	

/******************************************************************************
 * struct tm *calendar_pkt_to_tm(struct TIMESTAMPGP1 * strTS);
 * @brief	: Convert GPS extracted time to Linux struct tm
 * @param	: Pointer to GPS timestamp
 * @return	: Pointer to static with Linux tm struct
*******************************************************************************/
struct tm *calendar_pkt_to_tm(struct TIMESTAMPGP1 * strTS)
{
		/* GPS time as MM/DD/YY HH:MM:SS */
	strTM.tm_mon  =	asctoint2d(&strTS->g[ 8]); // Month
	strTM.tm_mday =	asctoint2d(&strTS->g[10]); // Day
	strTM.tm_year =	asctoint2d(&strTS->g[ 6]); // Year
	strTM.tm_min  =	asctoint2d(&strTS->g[ 0]); // Minute
	strTM.tm_sec  =	asctoint2d(&strTS->g[ 4]); // Second
	return &strTM;
}
/******************************************************************************
 * short asctoint2d(char * p);
 * @brief	: Convert decimal digits in ascii to short
 * @param	: pointer to high order of two ascii digits
*******************************************************************************/
short asctoint2d(char * p)
{
	return ( (*p -'0')*10 ) + ( *(p+1) - '0' );
	
}
