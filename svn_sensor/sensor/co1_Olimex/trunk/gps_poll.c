/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : gps_poll.c
* Hacker	     : deh
* Date First Issued  : 01/28/2013
* Board              : Olimex
* Description        : 1 Hz GPS time 
*******************************************************************************/
/*
Poll the GPS UART to see if there is line buffered.  If so, check it and extract
date|time from $GPRMC lines, then convert that to Linux time, shifted left 6 to 
accommodate 1/64th sec ticks.  

If the even seconds from the GPS differs from the system (SYS), set a flag so that
the Tim4_se.c will pick up the new time.  Once sync'd there should not be any need
for "jamming" a new time as the 1/64th ticks are synchronized to the GPS 1 PPS.

01/26/2014 Rev 361 Updating to add sending on CAN and logging of gps fix in binary.

*/

#include <string.h>
//#include <stdio.h>

#include "common.h"
#include "common_time.h"
#include "common_misc.h"
#include "common_can.h"
#include "p1_gps_time_convert.h"
#include "p1_gps_time_linux.h"
#include "libmiscstm32/printf.h"
#include "gps_1pps_se.h"
#include "can_log.h"

/* Routine protos */
void debug_datetime(void);
static void gpsfix_canmsg_add(u32 id, char* ppay, int count);

/* Periodically, set a packet for logging the GPS sentences.  */
#define GPSPACKETSKIP	15		// Number of GPS sentence packets skip between loggings
static char cGPSpacketctr = 0;		// Skip counter

/* Flags for folk */
volatile short gps_mn_pkt_ctr;		// Monitor data ready: 	increments each upon each time stamp
volatile short gps_monitor_flag;	// Monitor data:       	0 = ignore gps, + = collect data
volatile short gps_OK_to_shutdown;	// Shutdown flag: 	0 = Don't shutdown, 1 = OK to shutdown


/* GPS binary fix msgs for logging & placement on the CAN bus */
#define GPSCANRCVBUFSIZELOG  4	// Number of CAN type gps msgs buffered
static struct CANRCVSTAMPEDBUF gpscanbuflog[GPSCANRCVBUFSIZELOG];	// CAN message circular buffer
static int gpscanbuflogIDXm = 0;		// Index for mainline removing data from buffer
static int gpscanbuflogIDXi = 0;		// Index for interrupt routine adding data
#define GPSCANRCVBUFSIZECAN  4	// Number of CAN type gps msgs buffered
static struct CANRCVBUF gpscanbufcan[GPSCANRCVBUFSIZECAN];	// CAN message circular buffer
static int gpscanbufcanIDXm = 0;		// Index for mainline removing data from buffer
static int gpscanbufcanIDXi = 0;		// Index for interrupt routine adding data

static struct GPSFIX gpsfix;	// Lat, Long, Ht from GPS

static struct USARTLB strlb;	// Holds the return from 'getlineboth' of char count & pointer (@2)
unsigned int uiConsecutiveGoodGPSctr;	// Count of consecutive good GPS fixes

int nOffsetCalFlag;		// 0 = we have not updated the freq offset calibration


u8 	gps_poll_flag = 0;	// 0 = idle; 1 = update .SYS time
u8	GPStimegood = 0;	// Get started counter
u32 gps_poll_flag_ctr = 0;	// Running count of GPS v SYS time updates
u8	gps_sentence_flag = 0;	// 0 = don't display GPS sentences; not zero = display

struct TIMESTAMPGP1 pkt_gps_mn;		// GPS time versus rtc tickcounter packet for monitoring

/******************************************************************************
 * void gps_poll(void);
 * @brief	: Do all things nice and good for gps
 * @return	: 
 ******************************************************************************/
/* This routine is in the main polling loop */
void gps_poll(void)
{
	char *pgps;
	u8 ctemp;
	unsigned int gps_ret;
	time_t tLinuxtimecounter;	// Fancy name for int used by time routines
	int tmp;
	char vv[64];	// sprintf work

	strlb = USART1_rxint_getlineboth();	// Get both char count and pointer
	if (strlb.ct != 0)
	{ // Here it looks like we got a line of something.
	
		/* See if there is a '$' on this line, and return ptr to '$' */
		if ((pgps = gps_time_find_dollars(strlb)) == 0) return;	// Return: No '$' found

		if (gps_crc_check(strlb) != 0)
		{
			printf("GPS checksum error\n\r"); USART2_txint_send();
			return;
		}

		/* 'l' command sets flag for displaying gps sentences. "Ist the gps 'working'?" */
		if (gps_sentence_flag != 0) 
		{
			debug_datetime();
			printf("%s\n\r",strlb.p);USART2_txint_send();
		}
	
		/* Periodically, set a packet for logging the GPS sentences. */
		if (cGPSpacketctr++ >= GPSPACKETSKIP)
		{
			cGPSpacketctr = 0;			// Reset skip count
			ctemp = strlb.ct;			// nonCAN msg size limited to one byte count
			if (strlb.ct > 255) ctemp = 255;
			noncanlog_add((u8*)strlb.p, ctemp);	// Buffer packet
		}

		/* Check for a valid time/fix line.  */
		if ( (gps_ret = gps_time_stampGPRMC(&pkt_gps_mn, strlb, 0) ) == 99 )
		{ // Here we got a valid time/fix $GPRMC record and extracted the time fields */

//printf("OK: %s\n\r",strlb.p);USART2_txint_send();

			/* Start up--allow for a few fixes to "settle in". */
			if (GPStimegood < 3)	// Startup flag
				GPStimegood += 1;

			/* Convert gps time to linux format (seconds since year 1900) */
			tLinuxtimecounter =  gps_time_linux_init(&pkt_gps_mn);	// Convert ascii GPS time to 32b linux format
			strAlltime.GPS.ull  = tLinuxtimecounter; 	// Convert to signed long long
			strAlltime.GPS.ull -= (PODTIMEEPOCH);		// Move to more recent epoch to fit into 40 bits
			strAlltime.GPS.ull  = (strAlltime.GPS.ull << 6); // Scale linux time for 64 ticks per sec

//printf("gps_poll GPS %u SYS %u\n\r",(unsigned int)(strAlltime.GPS.ull >> 6),(unsigned int)(strAlltime.SYS.ull >> 6));USART2_txint_send();

			/* GPS and SYS (running time stamp count) should stay in step, once it gets started */
			if ( strAlltime.GPS.ull != (strAlltime.SYS.ull & ~0x3f) ) // Same at the 1 sec level?
			{ // Here, no. Set a flag for 'Tim4_pod_se.c' to pick up the time.
				gps_poll_flag = 1;
				gps_poll_flag_ctr += 1;	// Keep track of these "anomolies" for the hapless programmer.	
			}

			/* Setup gps fix CAN msg for logging and sending on the CAN bus. */
			tmp = gps_getfix_GPRMC(&gpsfix, strlb); // Convert the lat/long to binary, scaled, minutes * 1E5.
			if ( tmp == 0)
			{ // Success.  Send gps fix on CAN bus, and log it. 
				// Add to buffer and send to CAN: ID, pointer to payload, payload count
				gpsfix_canmsg_add( (CAN_UNITID_CO_OLI | (CAN_DATAID_LAT_LONG << CAN_DATAID_SHIFT) ), (char*)&gpsfix.lat, 8);
			}
			else
			{ // Here, some error in the extraction and conversion
				
				sprintf (vv,"$GPRMC FIX ERROR: %d %u\n\r",tmp, strlb.ct, strlb.p);
				USART2_txint_puts (vv);USART2_txint_send();
			}

		}
		else
		{

//struct USARTLB strlb;	// Holds the return from 'getlineboth' of char count & pointer
//char vv[64];	// sprintf work
//if (strlb.ct > 3)
//{
//USART2_txint_puts(strlb.p);	// Echo back the line just received
//USART2_txint_puts ("\n");	// Add line feed to make things look nice
//USART2_txint_send();		// Start the line buffer sending
//}
					/* List all other sentences, e.g. software version */
					if (gps_sentence_flag != 0)
					{
						printf ("NOT $GPRMC: %u %u %s",gps_ret,strlb.ct,pgps); USART2_txint_send();
					}
		}
	}
	return;
}
/* **************************************************************************************************
NOTE: The gps sets up a CAN msg with the "fix" in binary, i.e. lat/long, and another msg with height.
These are stored in two buffers.  One for logging and the other for setting up to be sent on the CAN
bus.  Two are used since the addition to the buffers is done under mainline polling, and the extraction
is done under different interrupt levels for logging and loading the CAN. */
/******************************************************************************
 * static void gpsfix_canmsg_add(u32 id, char* ppay, int count);
 * @brief	: Add a gps generated CAN type to the buffers for logging & sending on CAN
 * @param	: p = pointer to string with gps sentence (starting with '$')
 * @param	: id = CAN id
 * @param	: ppay = payload pointer
 * @param	: count = number of bytes in payload
 ******************************************************************************/
static void gpsfix_canmsg_add(u32 id, char* ppay, int count)
{
	int i;
	/* Set up id and our linux time stamp */
	gpscanbuflog[gpscanbuflogIDXi].id = id;	// Be sure we have it tagged with the ID
	gpscanbuflog[gpscanbuflogIDXi].U.ull = strAlltime.SYS.ull;	// Add extended linux format time

	/* Set payload count */
	if (count > 8) count = 8; gpscanbuflog[gpscanbuflogIDXi].dlc = count;
	
	/* Copy payload to buffer */
	gpscanbuflog[gpscanbuflogIDXi].cd.ull = 0;	// Zero out unused payload bytes
	for (i = 0; i < count; i++)
		gpscanbuflog[gpscanbuflogIDXi].cd.uc[i] = (u8)*ppay++;
	
	/* Save a copy in the can buffer (that will go on the CAN bus). */
	gpscanbufcan[gpscanbufcanIDXi].id  = gpscanbuflog[gpscanbuflogIDXi].id;
	gpscanbufcan[gpscanbufcanIDXi].dlc = gpscanbuflog[gpscanbuflogIDXi].dlc;
	gpscanbufcan[gpscanbufcanIDXi].cd  = gpscanbuflog[gpscanbuflogIDXi].cd;

	/* Advance indices for buffers with wrap-around. */
	gpscanbufcanIDXi += 1; if (gpscanbufcanIDXi >= GPSCANRCVBUFSIZECAN) gpscanbufcanIDXi = 0;
	gpscanbuflogIDXi += 1; if (gpscanbuflogIDXi >= GPSCANRCVBUFSIZELOG) gpscanbuflogIDXi = 0;

	return;
}

/******************************************************************************
 * struct CANRCVSTAMPEDBUF* gpsfix_canmsg_get_log(void);
 * @brief	: Get pointer & count to the buffer to be drained for logging
 * @return	: NULL (zero) for no new data, or pointer to buffer 
 ******************************************************************************/
struct CANRCVSTAMPEDBUF* gpsfix_canmsg_get_log(void)
{
	struct CANRCVSTAMPEDBUF* p;
	if (gpscanbuflogIDXm == gpscanbuflogIDXi) return NULL;	// Return showing no new data
	p = &gpscanbuflog[gpscanbuflogIDXm]; 	// Set pointer
	gpscanbuflogIDXm += 1;  if (gpscanbuflogIDXm >= GPSCANRCVBUFSIZELOG) gpscanbuflogIDXm = 0;
	return p;				// Return with pointer and count
}
/******************************************************************************
 * struct CANRCVBUF* gpsfix_canmsg_get_can(void);
 * @brief	: Get pointer & count to the buffer to be drained for sending on CAN bus
 * @return	: NULL (zero) for no new data, or pointer to buffer 
 ******************************************************************************/
struct CANRCVBUF* gpsfix_canmsg_get_can(void)
{
	struct CANRCVBUF* p;
	if (gpscanbufcanIDXm == gpscanbufcanIDXi) return NULL;	// Return showing no new data
	p = &gpscanbufcan[gpscanbufcanIDXm]; 	// Set pointer
	gpscanbufcanIDXm += 1;  if (gpscanbufcanIDXm >= GPSCANRCVBUFSIZECAN) gpscanbufcanIDXm = 0;
	return p;				// Return with pointer and count
}
/******************************************************************************
 * void debug_datetime(void);
 * @brief	: Format and print date time in readable form
 ******************************************************************************/
void debug_datetime(void)
{
	/* Linux format time.  Tick count shifted right 11 bits (2048) */
	unsigned int uitemp;

//	printf ("%u ",gps_poll_flag_ctr);

//	uitemp = (  (strAlltime.GPS.ull >> 6) );
	printf (" %010x %10x",  strAlltime.GPS.ui[1],strAlltime.GPS.ui[0] );

	uitemp = (  (strAlltime.SYS.ull >> 6) );
//	printf (" %010x",  uitemp );

	/* Get epoch out of our hokey scheme for saving a byte to the 'ctime' routine basis */
	uitemp += PODTIMEEPOCH;		// Adjust for shifted epoch

	/* Reconvert to ascii (should match the above ascii where appropriate) */
	printf ("  %s\n\r", ctime((const time_t*)&uitemp));		

	USART2_txint_send();

	return;
}


