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

void debug_datetime(void);

/* Periodically, set a packet for logging the GPS sentences.  */
#define GPSPACKETSKIP	15	// Number of seconds between GPS sentence packets
static char cGPSpacketctr = 0;
char cGPSpacketflag = 0;
static struct GPSPACKETHDR gpspacket; // Hold header for GPS logging


/* The following three are for synchronizing the usage of the 'GPS' with the RTC interrupt */

/* Flags for folk */
volatile short gps_mn_pkt_ctr;		// Monitor data ready: 	increments each upon each time stamp
volatile short gps_monitor_flag;	// Monitor data:       	0 = ignore gps, + = collect data
volatile short gps_OK_to_shutdown;	// Shutdown flag: 	0 = Don't shutdown, 1 = OK to shutdown


/* Just for us variables */
//static short state_GPS;		// Honorable state machine
static struct USARTLB strlb;	// Holds the return from 'getlineboth' of char count & pointer (@2)
//static short gps_limit_ctr;	// Discard fixes until at least 4 secs of consecutive good fixes
//static unsigned int uiGPSrtc;	// RTC tickcount saved at EOL of GPS time
//static unsigned short skipct = GPSSDPACKETSSKIP;//Number of secs to skip between SD writes
unsigned int uiConsecutiveGoodGPSctr;	// Count of consecutive good GPS fixes

/* For Xtal & time calibration */
//static unsigned int uiTim2Prev;	// Used to compute difference
//static unsigned int styflgPrev;	// Previous flag count for testing for new TIM2 input capture data
int nOffsetCalFlag;		// 0 = we have not updated the freq offset calibration

/* These are for flashing the onboard and external LEDs (not BOX LED) */
//static char cGPSng;		// 0 = GPS giving not good data, 1 = good data
//static unsigned int nDelayCt;	// Count polls to determine if gps

//char cGPS_flag;			// Receiving good GPS fixes

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
	char *p, *pp, *pgps;
	unsigned int gps_ret;
	time_t tLinuxtimecounter;	// Fancy name for int used by time routines
	int i;


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

		if (gps_sentence_flag != 0)
		{
			debug_datetime();
			printf("%s\n\r",strlb.p);USART2_txint_send();
		}
	

		/* Periodically, set a packet for logging the GPS sentences. */
		if (cGPSpacketctr++ >= GPSPACKETSKIP)
		{
			if (cGPSpacketflag == 0)	
			{ // Here, log.c has logged packet.  log.c runs at a low priority interrupt level
				cGPSpacketflag = 1;		// Show logger we are updating		
				cGPSpacketctr = 0;		// Reset skip count
				gpspacket.U.ull = strAlltime.SYS.ull; // Add time.  Make header just like CAN messages
				if (strlb.ct >= GPSSAVESIZE) strlb.ct = GPSSAVESIZE; // Prevent buffer overrun
				gpspacket.dlc = strlb.ct;	// Time & byte count
				/* Copy GPS line to packet buffer */
				p = pgps; pp = &gpspacket.c[0];
				for (i = 0; i < strlb.ct; i++) *pp++ = *p++;
				cGPSpacketflag = 2;		// Show logger we are done updating
			}
		}

		/* Check for a valid time/fix line and extract time and RTC counter at EOL.  */
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
}
/******************************************************************************
 * struct PP get_packet_GPS(void);
 * @brief	: Format and print date time in readable form
 ******************************************************************************/
struct PP get_packet_GPS(void)
{
/* Entry from log.c is done under a low level interrupt. */
	struct PP pp = {0,0};
	
	if (cGPSpacketflag < 2) return pp;	// Return nothing ready, or caught in update

	cGPSpacketflag = 0;	// Reset 
	pp.ct = sizeof(struct GPSPACKETHDR) + gpspacket.dlc; // Total byte count for logging
	pp.p = (char *)&gpspacket;	//  Pointer to it
	return pp;
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


