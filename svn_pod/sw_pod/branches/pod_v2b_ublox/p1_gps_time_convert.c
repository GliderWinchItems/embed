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
03-08-2013 Changes for u-blox GPS
06-11-2013 Added GPS checksum check
*/

/* Three possibilties from u-blox
$GPRMC,,V,,,,,,,,,,N*53
$GPRMC,150609.60,V,,,,,,,190313,,,N*79
$GPRMC,150849.60,A,2808.94368,N,08228.60014,W,0.307,,190313,,,A*69
*/

#include "libusartstm32/usartallproto.h"
#include "libmiscstm32/printf.h"
#include "p1_gps_time_convert.h"
#include <string.h>

/* Status of fix */
char cFixstatus = 0;	// 0 = unknown, 'A' good fix, 'V' no fix, maybe time, 'T' V & time looks legitimate
/******************************************************************************
 * char *gps_field_copy(char *pp, char *p, s16 length );
 * @brief	: Copy field from current position 'p' to next ',' or length limit
 * @param	: pp = pointer to output line
 * @param	: p = pointer to GPS input line
 * @param	: length = max number of chars to stop the copy
 * @return	: pp = pointing to next char position to store in
 ******************************************************************************/
char *gps_field_copy(char *pp, char *p, s16 length )
{
	/* Stop--at end of line, or run out of fields, too many chars searched, or ',' (next field) */
	while ( (*p != 0xa) && (*p != 0xd) && (length-- > 0) && (*p != ',') )
	{ // Here we are good to go.
		*pp++ = *p++;	// Copy input to output
	}
	return pp;
}
char *gps_field_copy_p(char *pp, char *p, s16 length )
{
	/* Stop--at end of line, or run out of fields, too many chars searched, or ',' (next field) */
	while ( (*p != 0xa) && (*p != 0xd) && (length-- > 0) && (*p != ',') )
	{ // Here we are good to go.
		*pp++ = *p++;	// Copy input to output
	}
	return p;
}
/******************************************************************************
 * char *gps_field(char *p, u8 n, s16 length );
 * @brief	: Position pointer to comma separated field
 * @param	: p = pointer to GPS line
 * @param	: n = field, 0 = $xxx, 1 = first field following $xxx
 * @param	: length = max number of chars to stop the search
 * @return	: pointer to char following ',' or not found return original 'p'
 ******************************************************************************/
char *gps_field(char *p, u8 n, s16 length )
{
	u8 r = 0;	// Field counter
	char *psv = p;

	/* Stop--at end of line, or run out of fields, or too many chars searched */
	while ( (*p != 0xa) && (*p != 0xd) && (r < 23) && (length-- > 0))
	{ // Here we are good to go.
		if ( *p++ == ',')
		{
			r += 1;
			if (r == n) return p;  // Return point to char after ','
		}
	}
	return psv;	// We didn't find the field number specified
}
/******************************************************************************
 * char *gps_nextfield(char *p, u8 n);
 * @brief	: Position pointer to comma separated field
 * @param	: p = pointer to GPS line
 * @param	: n = max number of chars the next field could be
 * @return	: 0 = not found (too many), not zero = pointer to char following ','
 ******************************************************************************/
char *gps_nextfield(char *p, u8 n)
{
	return gps_field(p, 1, n);	// Return pointer
}
/******************************************************************************
 * char *gps_time_find_dollars(struct USARTLB strlb );
 * @brief	: Check for $GPRMC sentence then extract the date/time
 * @param	: strlb.p - pointer to GPS line
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
 * int gps_crc_check(struct USARTLB strlb );
 * @brief	: Check for $GPRMC sentence then extract the date/time
 * @param	: strlb.p - pointer to GPS line
 * @return	: zero == OK; non-zero = badness
 ******************************************************************************/
static unsigned char asctobin(char x)
{
	if (x <= '9') return (x - '0');
	return (x - 'A' + 10);
}
int gps_crc_check(struct USARTLB strlb )
{
	char *p = gps_time_find_dollars(strlb);
	char *pend = strlb.p + strlb.ct;
	unsigned char sum = 0;
	unsigned char z;
	
	p++;	// Point to 1st char following the '$'

	while ( (*p != '*') && (p < pend) )
		sum ^= *p++;

	/* Convert GPS ASCII checksum */
	z = asctobin(*(pend-3));
	z = z*16 + asctobin(*(pend-2));

	return (z - sum);
}
/******************************************************************************
 * unsigned int gps_time_stampGPRMC(struct TIMESTAMPGP1 *strTS, struct USARTLB strlb, u8 sw );
 * @brief	: Check for $GPRMC sentence then extract the date/time
 * @param	: strTS - receives extracted time
 * @param	: p - pointer to GPS line
 * @param	: sw - 0 = Garmin, 1 = u-blox
 * @return	: 0 = record is not valid; not zero = the RTC_CNT following sentence EOL
 ******************************************************************************/
unsigned int gps_time_stampGPRMC(struct TIMESTAMPGP1 *strTS, struct USARTLB strlb, u8 sw)
{
	int i;
	char *pp = "$GPRMC,";	// Sentence header to match
	char *p;
	char ttmp[16];		// Temp time (hhmmss.ss or hhmmss.s)
	char dtmp[16];		// Temp date (ddmmyy)
	unsigned int t;		
	unsigned int tsize;	// Number of chars extracted for time
	unsigned int dsize;	// Number of chars extracted for date

	/* Search forward to the '$'.  If not present something bogus. */
	if ( (p=gps_time_find_dollars(strlb)) == 0) return 0; // return invalid

	/* The first 7 chars should match "$GPRMC," */
	for (i = 0; i < 7; i++)
	{
		if (*p++ != *pp++)	
			return 1;	// Code showing this is line starting with '$'
	}

	/* Extract time field */
	if (*p == ',')	// Are there any time chars?
		return 6;	// No, nothing we can do.
	if (cGPStype == GARMIN_18X_1_HZ)
	{ // Here 1 Hz Garmin GPS
		pp = gps_field_copy_p(&ttmp[0], p, 6);	// Copy time field
		tsize = (pp - p);		// Save size of HHMMSS field
		if (tsize < 6) 	return 8;	// Too few 'hhmmss.s'		
	}
	else
	{ // Here, assuming a u-blox GPS
		pp = gps_field_copy_p(&ttmp[0], p, 9);	// Copy time field
		tsize = (pp - p);		// Save size of HHMMSS.SS field
		if (tsize < 8) 	return 8;	// Too few 'hhmmss.s'
	}

	/* Save fix status */
	pp++; if (*pp == ',')	return 12;	// No fix status char
	cFixstatus = *pp++;			// Save and step to ','
	pp++;

	/* Extract date field */
	pp = gps_field(pp, 6, 60); 		// Spin forward to DDMMYY
	p = gps_field_copy_p(&dtmp[0], pp, 6);	// Copy to temp location
	dsize = (p - pp);			// Save size of DDMMYY field

	/* Check validity */
	if (dsize != 6)  return 9;		// Wrong number from 'ddmmyy'
	t = ( (dtmp[4] - '0') * 10 + (dtmp[5] - '0') );	// Convert 'yy' to decimal
	if ((t < 13) || (t > 23)) return 11; 	// Year out of range

	if (cGPStype != GARMIN_18X_1_HZ)
	{ // Here, not a 1 Hz GPS
		/* Only use on-the-second times (only used on 5 Hz GPSs) */
		if (ttmp[7] != '0') 	return 32;	// Good time, but not on-the-second time
		if (tsize == 8)
		{ // Here, u-blox: hhmmss.ss
			if (sw == 1)		
			{
				if (ttmp[8] != 0)	return 32;	// Good time, but not on-the-second time
			}
			else
				return 41;		// Conflict of type of gps
		}
	}

	/* Save time for use by others */
	strncpy(&strTS->g[0], &ttmp[0],6);
	strncpy(&strTS->g[6], &dtmp[0],6);

	/* If date/time is from a 'V' fix status code, change it show time/date OK */
	if (cFixstatus == 'V') 
	{
		cFixstatus = 'T';
		/* Extract system tick counter that was stored when EOL was received.  1 PPS does not start
		   until fix is good, so use the time stored at EOL as a substitute. */
		while (*p++ != 0);	// Spin forward to input string terminator
		strTS->alltime.SYS.ul[0] = *(unsigned int*)(p);	// Next four bytes are the counter
	}
	
	return 99;	// Sucess!

}

