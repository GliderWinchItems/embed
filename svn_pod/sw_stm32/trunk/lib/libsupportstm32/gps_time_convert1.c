/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : gps_time_convert1.c
* Hackerees          : deh
* Date First Issued  : 11/25/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Better version of: Convert NMEA record time to something else(?)
*******************************************************************************/
/* 
This version does checksums and separates fields by commas, whereas the early
version 'gps_time_convert.c' uses fixed positions for fields and ignores the
checksum.

This is a sample line --
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

static unsigned char hexbin (char c)
{
	if ( c <= '9') return ('0' - c);
	if ( c <= 'F') return ('A' - c + 9);
	if ( c <= 'f') return ('a' - c + 9);
	return -1;
}

/******************************************************************************
 * unsigned int gps_time_stamp1(struct TIMESTAMPG *strTS, struct USARTLB strlb );
 * @brief	: Check for $GPRMC sentence then extract the date/time
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
unsigned int gps_time_stamp1(struct TIMESTAMPG *strTS, struct USARTLB strlb )
{
	int i;
	unsigned char ucChk = 0;		// Checksum
	char *pp = "GPRMC,";
	char *p =  strlb.p;
	unsigned char ucS;


	/* Find leading '$'.  Spin down input line until $ found, or we run out of chars */
	i = 0;
	while ( (*p++ != '$') && (i++ < strlb.ct) );

	/* p now points to next char after '$' */
	if (*(p-1) != '$') return 1;	// Return if no '$' found	

	/* See if this is the sentence type matches "GPRMC," */
	for (i = 0; i < 6; i++)
	{
		if (*(p+i) != *pp++)	
			return 2;	// Return if not a match
	}
	
	/* Build checksum */
	while ( (*(p+i) != '*') && (i++ < strlb.ct) ) ucChk ^= *p++;

	/* The next 2 chars are the checksum in the sentence */		
	ucS = (hexbin(*(p+i)) << 8) | hexbin( *(p+i+1) );	// Convert to binary
	if (ucS != ucChk) return 3;	// Return if checksums do not match

	/* Sentence type is correct and checksum is correct, now fill struct */
	pp = &strTS->g[0];	// Point to struct to be filled

	/* The next 6 char should be HHMMSS */
	for (i = 0; i < 6 ; i++)
	{	
		if ( (*(p+i) >= '0') && (*(p+i) <= '9') )	// Valid chars?
		{ // Here, yes.  Copy into struct
			*pp++ = *strlb.p;	// Store and adv output char ptr
		}
		else
		{
			return 4;
		}
		p++;	// Step to next input char
	}
	if (*p++ != ',') return 5;	// This char should be a ','
	if (*p++ != 'A') return 6;	// 'A' == valid fix with time
	if (*p++ != ',') return 7;	// This char should be a ','
	
	/* Should be ',' in the following places */
	if (*(p+26-17) != ',') return 8; 
	if (*(p+28-17) != ',') return 9;
	if (*(p+39-17) != ',') return 10; 
	if (*(p+41-17) != ',') return 11; 
	if (*(p+47-17) != ',') return 12; 
	if (*(p+53-17) != ',') return 13; 

	p += (53-17+1);	

	/* The next 6 char should be DDMMYY */
	for (i = 0; i < 6; i++)
	{	
		if ( (*(p+i) >= '0') && (*(p+i) <= '9') )	// Valid chars?
		{ // Here, yes.  Copy into struct
			*pp++ = *(p+i);	// Store and adv output char ptr
		}
		else
		{
			return 14;
		}
		strlb.p++;	// Step to next input char
	}
	p += 6;

	if (*p++ != ',') return 15;	// This char should be a ','

	if (*(p+11) != 0) return 16;	// This char should be the string terminator

	/* Extract RTC_CNT register that was stored when EOL was received */
	strTS->cnt = *(unsigned int*)(p+12);
	return 0;		// Passed all the tests.
}
/******************************************************************************
 * unsigned int gps_time_ng_fix(struct USARTLB strlb );
 * @brief	: Check for $GPRMC sentence and see of fix is OK
 * @param	: p - pointer to GPS sentence
 * @return	: 0 = record is not valid; not zero = 'V' indicating bad fix
 ******************************************************************************/
unsigned int gps_time_ng_fix1(struct USARTLB strlb )
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

