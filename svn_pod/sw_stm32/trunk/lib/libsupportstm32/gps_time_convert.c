/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : gps_time_convert.c
* Hackerees          : deh
* Date First Issued  : 08/07/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Convert NMEA record time to something else(?)
*******************************************************************************/
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
#include "gps_time_convert.h"

/******************************************************************************
 * unsigned int gps_time_stamp(struct TIMESTAMPG *strTS, struct USARTLB strlb );
 * @brief	: Check for $GPRMC sentence then extract the date/time
 * @param	: strTS - receives extracted time
 * @param	: p - pointer to GPS line
 * @return	: 0 = record is not valid; not zero = the RTC_CNT following sentence EOL
 ******************************************************************************/
unsigned int gps_time_stamp(struct TIMESTAMPG *strTS, struct USARTLB strlb )
{
	int i;
	unsigned int j = 0;
	char *pp = "GPRMC,";

//	if (*strlb.p != '$')
//		strlb.p++;		// Step over 0x0a at beginning of line

	while ((*strlb.p++ != '$') && (j++ < strlb.ct));

//	if ((strlb.ct != 71)||(strlb.ct != 72))	return 0; // Line length for time record

	/* The first 7 chars should match "$GPRMC," */
	for (i = 0; i < 6; i++)
	{
		if (*strlb.p++ != *pp++)	
			return 0;
	}
	pp = &strTS->g[0];	// Point to struct to be filled
	/* The next 6 char should be HHMMSS */
	for (i = 0; i < 6; i++)
	{	
		if ( (*strlb.p >= '0') && (*strlb.p <= '9') )	// Valid chars?
		{ // Here, yes.  Copy into struct
			*pp++ = *strlb.p;	// Store and adv output char
		}
		else
		{
			return 0;
		}
		strlb.p++;	// Step to next input char
	}
	if (*strlb.p++ != ',') return 0;	// This char should be a ','
	if (*strlb.p++ != 'A') return 0;	// 'A' == valid fix with time
	if (*strlb.p++ != ',') return 0;	// This char should be a ','
	
	/* Should be ',' in the following places */
	if (*(strlb.p+26-17) != ',') return 0; 
	if (*(strlb.p+28-17) != ',') return 0;
	if (*(strlb.p+39-17) != ',') return 0; 
	if (*(strlb.p+41-17) != ',') return 0; 
	if (*(strlb.p+47-17) != ',') return 0; 
	if (*(strlb.p+53-17) != ',') return 0; 

	strlb.p += (53-17+1);	

	/* The next 6 char should be DDMMYY */
	for (i = 0; i < 6; i++)
	{	
		if ( (*strlb.p >= '0') && (*strlb.p <= '9') )	// Valid chars?
		{ // Here, yes.  Copy into struct
			*pp++ = *strlb.p;	// Store and adv output char
		}
		else
		{
			return 0;
		}
		strlb.p++;	// Step to next input char
	}
	if (*strlb.p++ != ',') return 0;	// This char should be a ','

	if (*(strlb.p+11) != 0) return 0;	// This char should be the string terminator

	/* Extract RTC_CNT register that was stored when EOL was received */
	strTS->cnt = *(unsigned int*)(strlb.p+12);
	return strTS->cnt;	
}
/******************************************************************************
 * unsigned int gps_time_ng_fix(struct USARTLB strlb );
 * @brief	: Check for $GPRMC sentence and see of fix is OK
 * @param	: p - pointer to GPS sentence
 * @return	: 0 = record is not valid; not zero = 'V' indicating bad fix
 ******************************************************************************/
unsigned int gps_time_ng_fix(struct USARTLB strlb )
{
	int i;
	char *pp = "$GPRMC,";
	if (*strlb.p != '$')
		strlb.p++;		// Step over 0x0a at beginning of line

	if (strlb.ct < 60)	return 0; // Line length for time record
	if (strlb.ct > 62)	return 0; // Line length for time record

	/* The first 7 chars should match "$GPRMC," */
	for (i = 0; i < 7; i++)
	{
		if (*strlb.p++ != *pp++)	
			return 0;
	}
	if (*(strlb.p+8) == 'V') return 1;
	return 0;
}

