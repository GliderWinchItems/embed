/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : p1_gps_time_convert.c
* Hackerees          : deh
* Date First Issued  : 08/07/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Convert NMEA record time to something else(?)
*******************************************************************************/
/*
10-14-2011 Update for pod_v1
06-24-2012 Changes for on-board GPS logging
*/
/* This is a sample line --
$GPRMC,191814,A,2808.9417,N,08228.6094,W,000.0,000.0,070811,005.0,W*7A
...
$GPRMC,001635,A,2808.9491,N,08228.6040,W,000.0,000.0,080811,005.0,W*77

$GPGGA,001945,2808.9485,N,08228.6037,W,2,10,0.8,24.7,M,-30.0,M,,*4C
ER: 1 72

$GPRMC,001946,A,2808.9485,N,08228.6037,W,000.0,000.0,080811,005.0,W*79
 24025666         0  24025666 
ER: 0 69

*/

#include "libusartstm32/usartallproto.h"
#include "p1_gps_time_convert.h"
/******************************************************************************
 * char * gps_time_find_dollars(struct USARTLB strlb );
 * @brief	: Check for $GPRMC sentence then extract the date/time
 * @param	: p - pointer to GPS line
 * @return	: zero == no '$' found; non-zero = points to '$'
 ******************************************************************************/
char * gps_time_find_dollars(struct USARTLB strlb )
{
	/* Search forward for a '$' */
	while ((*strlb.p != '$') && (*strlb.p != 0)) strlb.p++;
	
	if (*strlb.p == 0) return 0;	// Return = not a valid sentence (no '$')

	return strlb.p;
}
/******************************************************************************
 * unsigned int gps_time_stamp(struct TIMESTAMPGP1 *strTS, struct USARTLB strlb );
 * @brief	: Check for $GPRMC sentence then extract the date/time
 * @param	: strTS - receives extracted time
 * @param	: p - pointer to GPS line
 * @return	: 0 = record is not valid; not zero = the RTC_CNT following sentence EOL
 ******************************************************************************/
unsigned int gps_time_stamp(struct TIMESTAMPGP1 *strTS, struct USARTLB strlb )
{
	int i;
	char *pp = "$GPRMC,";
	char * p;

	/* Search forward to the '$'.  If not present something bogus. */
	if ( (p=gps_time_find_dollars(strlb)) == 0) return 0; // return invalid

	/* The first 7 chars should match "$GPRMC," */
	for (i = 0; i < 7; i++)
	{
		if (*p++ != *pp++)	
			return 1;
	}
	pp = &strTS->g[0];	// Point to struct to be filled
	/* The next 6 char should be HHMMSS */
	for (i = 0; i < 6; i++)
	{	
		if ( (*p >= '0') && (*p <= '9') )	// Valid number?
		{ // Here, yes.  Copy into struct
			*pp++ = *p;	// Store and adv output char
		}
		else
		{
			return 2;
		}
		p++;	// Step to next input char
	}

	/* 5 Hz version has time extended to tenths sec, e.g. 163417.2 */
	if (*p++ != '.') return 31;	// This char should be a '.'	
	if (*p++ != '0') return 32;

	if (*p++ != ',') return 3;	// This char should be a ','
	if (*p++ != 'A') return 4;	// 'A' == valid fix with time
	if (*p++ != ',') return 5;	// This char should be a ','
	
	/* Should be ',' in the following places */
	if (*(p+29-19) != ',') return 6; 
	if (*(p+31-19) != ',') return 7;
	if (*(p+43-19) != ',') return 8; 
	if (*(p+45-19) != ',') return 9; 
	if (*(p+58-19) != ',') return 10; 
	if (*(p+65-19) != ',') return 11; 
	if (*(p+71-19) != ',') return 110; 
//	if (*(p+73-19) != ',') return 111; // Re: NEMA v 2.30

	/* A '.' should be in the following places */
	if (*(p+23-19) != '.') return 112;
	if (*(p+37-19) != '.') return 113;
	if (*(p+56-19) != '.') return 114;
	if (*(p+69-19) != '.') return 115;


	/* The next 6 char should be DDMMYY */
	for (i = 0+(59-19); i < 6+(59-19); i++)
	{	
		if ( (*(p+i) >= '0') && (*(p+i) <= '9') )	// Valid numbers?
		{ // Here, yes.  Copy into struct
			*pp++ = *(p+i);	// Store and adv output char
		}
		else
		{
			return 12;
		}
	}
	*pp = 0;	// String terminator

return 99;
	/* Extract system tick counter that was stored when EOL was received */
//	strTS->alltime.SYS.ul[0] = *(unsigned int*)(p+12);
//	return strTS->alltime.SYS.ul[0];	
}

