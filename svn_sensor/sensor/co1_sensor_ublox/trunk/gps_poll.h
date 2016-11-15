/******************************************************************************
* File Name          : gps_poll.h
* Date First Issued  : 08/09/2016
* Board              : Sensor
* Description        : GPS polling
*******************************************************************************/

/*
@1 = svn_pod/sw_stm32/trunk/lib/libsupportstm32/gps_time_convert.h

*/
#ifndef __GPS_POLL
#define __GPS_POLL

#include "p1_gps_time_convert.h"
#include "can_hub.h"
#include "gps_idx_v_struct.h"
#include "p1_gps_time_convert.h"

#define GPSLIMITCT	90	// Limit time to get a good fix/time (secs)
#define GPSTIMEOUTCT	90	// Limit time to get a good fix/time (secs)
#define GPSDISCARDCT	4	// Number of initial good readings to discard

/* (This saves looking it up.)
struct GPSFIX
{
	int	lat;	// Latitude  (+/-  540000000 (minutes * 100,000) minus = S, plus = N)
	int	lon;	// Longitude (+/- 1080000000 (minutes * 100,000) minus = E, plus = W)
	int	ht;	// Meters (+/- meters * 10)
	unsigned char fix;	// Fix type 0 = NF, 1 = G1, 2 = G3, 3 = G3
	unsigned char nsats;	// Number of satellites
	int	code;	// Conversion resulting from parsing of the line
};
*/
/* msg format
pcan.uc[0]
 0:3 fix type (.fix)
   0 = 0 = NF, 1 = G1, 2 = G3, 3 = G3
 4:7 conversion code 
   0 = conversion good
   -12 = conversion good, but NF (No Fix)
   other negative numbers = line has some error and conversion aborted
pcan.uc[1] 
 0:2 which reading
   0 = latitude
   1 = longitude
   2 = height
 3:7 Number of satellites (.nsats)

pcan.uc[2:5] = .lat or .lon
pcan.uc[2:3] = .ht
*/

struct GPSFUNCTION
{
	struct	GPSLC gps_s;
	struct CANHUB* phub_gps;	// Pointer: CAN hub buffer
	void* pparamflash;		// Pointer to flash area with flat array of parameters
	uint32_t* pcanid_cmd_gps;	// Pointer into high flash for command can id
	uint32_t hb_tim_t;		// tim9 tick counter for next heart-beat CAN msg--time
	uint32_t hb_llh_t;		// tim9 tick counter for next heart-beat CAN msg--fix
	uint32_t hbct_tim_ticks;	// gps_s.hbct_tim (ms) converted to timer ticks--time	
	uint32_t hbct_llh_ticks;	// gps_s.hbct_llh (ms) converted to timer ticks--fix
	uint32_t hbct_llh_delay_ticks;	// gps_s.hbct_llh_delay (ms) converted to timer ticks between gps lat/lon/ht msgs burst
	uint8_t	status_byte_tim;	// unix time msg status byte
	uint8_t	status_byte_llh;	// lat/long/ht msg(s) status byte
	uint8_t llhmsgidx;		// index for sequencing lat/long/ht msg sending
	uint32_t llh[3];		// lat/lon/height 
	time_t tLinuxtimecounter;	// Fancy name for int used by time routines
	struct GPSFIX gpsfix[2];	// latest fix data buffer
	uint32_t gpsfixidx;		// gpsfix buffer index for main


};

/******************************************************************************/
int gps_poll_can (struct CANRCVBUF* pcan, struct GPSFUNCTION* p);
/* @brief 	: Handles all things associated with the GPS module and CAN
 * @param	: pcan = pointer to struct with icoming CAN msg
 * @param	: p = pointer to logger function struct with params and more
 * @return	: 0 = No outgoing msgs sent; 1 = one or more msgs were sent
 ******************************************************************************/
void gps_poll(struct GPSFUNCTION* p);
/* @brief 	: Handles all things associated with the GPS module and CAN
 * @param	: p = pointer to logger function struct with params and more
 * @return	: 0 = No outgoing msgs sent; 1 = one or more msgs were sent
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
int gps_poll_init(void);
/* @brief	: Setup the gps_poll function
 * @return	: 0 = success
 *		: -997 can-hub addition failed
 *		: -998 did not find function code in table
 *		: -999 unreasonable table size
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

/* Holds parameters and associated computed values and readings. */
extern struct GPSFUNCTION gps_f;

#endif

