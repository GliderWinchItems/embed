/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : gps_poll.h
* Hacker	     : deh
* Date First Issued  : 01/28/2013
* Board              : Olimex
* Description        : 1 Hz GPS time 
*******************************************************************************/

/*
@1 = svn_pod/sw_stm32/trunk/lib/libsupportstm32/gps_time_convert.h

*/
#ifndef __GPS_POLL
#define __GPS_POLL

#include "p1_gps_time_convert.h"

#define GPSLIMITCT	90	// Limit time to get a good fix/time (secs)
#define GPSTIMEOUTCT	90	// Limit time to get a good fix/time (secs)
#define GPSDISCARDCT	4	// Number of initial good readings to discard

/******************************************************************************/
void gps_poll(void);
/* @brief	: Do all things nice and good for gps
 * @return	: 
 ******************************************************************************/
void debug_datetime(void);
/* @brief	: Format and print date time in readable form
 ******************************************************************************/
struct CANRCVSTAMPEDBUF* gpsfix_canmsg_get_log(void);
/* @brief	: Get pointer & count to the buffer to be drained for logging
 * @return	: NULL (zero) for no new data, or pointer to buffer 
 ******************************************************************************/
struct CANRCVBUF* gpsfix_canmsg_get_can(void);
/* @brief	: Get pointer & count to the buffer to be drained for sending on CAN bus
 * @return	: NULL (zero) for no new data, or pointer to buffer 
 ******************************************************************************/


/* These are in 'gps_packetize.c' */
extern volatile unsigned int gps_sd_pkt_ready;		// SD packet ready:    	0 = not ready, + = ready. ?
extern volatile short gps_sd_pkt_ctr;		// SD packet ready:    	0 = not ready, + = ready. ?
extern volatile short gps_mn_pkt_ctr;		// Monitor data ready: 	increments each upon each time stamp
extern volatile short gps_monitor_flag;		// Monitor data:       	0 = ignore gps, + = collect data ?
extern volatile short gps_OK_to_shutdown;	// Shutdown flag: 	0 = Don't shutdown, 1 = OK to shutdown
extern unsigned short usGGAsavectr;	// GGA save flag/ctr for PC monitoring

/* Packet of GPS time versus RTC tick counter (@1) */
extern struct TIMESTAMPGP1 pkt_gps_sd;		// GPS time versus rtc tickcounter packet writing to SD card
extern struct TIMESTAMPGP1 pkt_gps_mn;		// GPS time versus rtc tickcounter packet for monitoring



/* Used for setting offset calibration */
extern unsigned int uiConsecutiveGoodGPSctr;	// Count of consecutive good GPS fixes
extern int nOffsetCalFlag;			// 0 = we have not updated the freq offset calibration


extern char	cGPS_flag;
extern u8	GPStimegood;	// Get started counter
extern u8	gps_sentence_flag;	// 0 = don't display GPS sentences; not zero = display

#endif

