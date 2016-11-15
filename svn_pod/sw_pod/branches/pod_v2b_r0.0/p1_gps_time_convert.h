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

#include "p1_common.h"


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

/* gps_time_convert.c: Extraction of time from gps lines */
//extern struct TIMESTAMPGP1 strTS1;	// GPS time (@10)

#endif 

