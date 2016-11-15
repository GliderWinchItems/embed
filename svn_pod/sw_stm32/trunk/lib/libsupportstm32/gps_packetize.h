/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : gps_packetize.h
* Hacker	     : deh
* Date First Issued  : 09/05/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Buffering/unbuffering GPS time versus RTC tick counter
*******************************************************************************/
/*
@1 = svn_pod/sw_stm32/trunk/lib/libsupportstm32/gps_time_convert.h

*/
#ifndef __GPS_PACKETIZE
#define __GPS_PACKETIZE



#define GPSLIMITCT	90	// Limit time to get a good fix/time (secs)
#define GPSTIMEOUTCT	90	// Limit time to get a good fix/time (secs)
#define GPSDISCARDCT	3	// Number of initial good readings to discard
#define GPSSDPACKETSSKIP 10	// Number of GPS packets skip between writes to SD Card
#define PKT_GPS_ID	ID_GPS	// ID for GPS packets

#include "../../../../sw_pod/trunk/pod_v1/p1_common.h"
#include "../../../../sw_pod/trunk/pod_v1/p1_gps_time_convert.h"


/******************************************************************************/
struct PKT_PTR gps_packetize_poll(void);
/* @brief	: Do all things nice and good for gps
 * @return	: Pointer & count--zero = not ready.
 ******************************************************************************/
void gps_packetize_restart(void);
/* @brief	: Look for good gps data and store a packet
 ******************************************************************************/

/* These are in 'gps_packetize.c' */
extern volatile short gps_sd_pkt_ready;		// SD packet ready:    	0 = not ready, + = ready. ?
extern volatile short gps_sd_pkt_ctr;		// SD packet ready:    	0 = not ready, + = ready. ?
extern volatile short gps_mn_pkt_ctr;		// Monitor data ready: 	increments each upon each time stamp
extern volatile short gps_monitor_flag;		// Monitor data:       	0 = ignore gps, + = collect data ?
extern volatile short gps_OK_to_shutdown;	// Shutdown flag: 	0 = Don't shutdown, 1 = OK to shutdown

/* Packet of GPS time versus RTC tick counter (@1) */
extern struct TIMESTAMPG pkt_gps_sd;		// GPS time versus rtc tickcounter packet writing to SD card
extern struct TIMESTAMPG pkt_gps_mn;		// GPS time versus rtc tickcounter packet for monitoring

extern char cGPS_flag;


#endif

