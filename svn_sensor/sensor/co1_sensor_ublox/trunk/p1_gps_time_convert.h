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

#include "common_time.h"
#include "libusartstm32/usartallproto.h"

/* GPSFIX allows for the 5 digit fraction for minutes from the u-blox gps to just 
   fits within an int.  Lat/Long will fit in one CAN msg.  Height would be a separate
   msg if it is used.

   Sign convention-- Lat N/S -> +/-; Long W/E -> +/-
*/
struct GPSFIX
{
	int	lat;	// Latitude  (+/-  540000000 (minutes * 100,000) minus = S, plus = N)
	int	lon;	// Longitude (+/- 1080000000 (minutes * 100,000) minus = E, plus = W)
	int	ht;	// Meters (+/- meters * 10)
	unsigned char fix;	// Fix type 0 = NF, 1 = G1, 2 = G3, 3 = G3
	unsigned char nsats;	// Number of satellites
	int	code;	// Conversion resulting from parsing of the line
};

/******************************************************************************/
 unsigned int gps_time_stampGPRMC(struct TIMESTAMPGP1 *strTS, struct USARTLB strlb, u8 sw );
/* @brief	: Check for $GPRMC sentence then extract the date/time
 * @param	: strTS - receives extracted time
 * @param	: p - pointer to GPS line
 * @param	: sw - 0 = Garmin, 1 = u-blox
 * @return	: 0 = record is not valid; not zero = the RTC_CNT following sentence EOL
 ******************************************************************************/
char * gps_time_find_dollars(struct USARTLB strlb );
/* @brief	: Check for $GPRMC sentence then extract the date/time
 * @param	: p - pointer to GPS line
 * @return	: zero == no '$' found; non-zero = points to '$'
 ******************************************************************************/
char *gps_field(char *p, u8 n, s16 length );
/* @brief	: Position pointer to comma separated field
 * @param	: p = pointer to GPS line
 * @param	: n = field, 0 = $xxx, 1 = first field following $xxx
 * @param	: length = max number of chars to stop the search
 * @return	: pointer to char following ',' or not found return original 'p'
 ******************************************************************************/
char *gps_nextfield(char *p, u8 n);
/* @brief	: Position pointer to comma separated field
 * @param	: p = pointer to GPS line
 * @param	: n = max number of chars the next field could be
 * @return	: pointer to char following ',' or not found return original 'p''
 ******************************************************************************/
char *gps_field_copy(char *pp, char *p, s16 length );
/* @brief	: Copy field from current position 'p' to next ',' or length limit
 * @param	: pp = pointer to output line
 * @param	: p = pointer to GPS input line
 * @param	: length = max number of chars to stop the copy
 * @return	: pp = pointing to next char position to store in
 ******************************************************************************/
int gps_crc_check(struct USARTLB strlb );
/* @brief	: Check for $GPRMC sentence then extract the date/time
 * @param	: strlb.p - pointer to GPS line
 * @return	: zero == OK; non-zero = badness
 ******************************************************************************/
int gps_getfix_GPRMC(struct GPSFIX* p, struct USARTLB strlb);
/* @brief	: Get Lat/Long fix from an already verified correct gps sentence(s)
 * @param	: p = points to GPSFIX struct that receives results
 * @param	: strlb = pointer to struct  with GPS line and char count
 * @return	: 0 = OK, *p = Lat/Long/Height extracted and converted
 *		: -1 = '$' not found on gps line
 *		: -2 = '.' not found in expected position of lat
 *		: -3 = ',' not found in expected position at end of lat field
 *		: -4 = not 'N' or 'S' for lat.
 *		: -5 = ',' not found in expected position at end of lat field
 *		: -6 = '.' not found in expected position in long field
 *		: -7 = not 'W' or 'E' for long
 ******************************************************************************/
int isGps_PUBX00(struct USARTLB strlb);
/* @brief	: Check if line is PUBX,00 gps message
 * @param	: p = points to ascii input line
 * @return	: 0 = OK, negative = not PUBX,00 message
 ******************************************************************************/
int gps_getfix_PUBX00(struct GPSFIX* p, struct USARTLB strlb);
/* @brief	: Get Lat/Long fix from an already verified correct gps sentence(s)
 * @param	: p = points to GPSFIX struct that receives results
 * @param	: strlb = pointer to struct  with GPS line and char count
 * @return	: 0 = OK, *p = Lat/Long/Height extracted and converted
 *		: -1 = '$' not found on gps line
 *		: -2 = '.' not found in expected position of lat
 *		: -3 = ',' not found in expected position at end of lat field
 *		: -4 = not 'N' or 'S' for lat.
 *		: -5 = ',' not found in expected position at end of lat field
 *		: -6 = '.' not found in expected position in long field
 *		: -7 = not 'W' or 'E' for long
 *		: -8 = '.' in height field not found
 * 		: -9 = not a PUBX,00 msg
 *		:-10 = line too short
 *		:-11 = '.' in height field not found
 *		:-12 = No fix: (1st char of fix = 'N')
 ******************************************************************************/

/* Status of fix */
extern char cFixstatus; // 0 = unknown, 'A' good fix, 'V' no fix, maybe time, 'T' V & time looks legitimate

#endif 

