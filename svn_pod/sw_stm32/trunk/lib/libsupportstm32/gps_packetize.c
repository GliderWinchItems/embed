/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : gps_packetize.c
* Hacker	     : deh
* Date First Issued  : 09/05/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Buffering/unbuffering GPS time versus RTC tick counter
*******************************************************************************/
/*
Subroutine call references shown as "@n"--
@1 = svn_pod/sw_stm32/trunk/lib/libsupportstm32/rtctimers.c
@2 = svn_pod/sw_stm32/trunk/lib/libusartstm32/usartallproto.c
@3 = svn_pod/sw_stm32/trunk/lib/libusartstm32/UART4_rxint_getlineboth.c
@4 = svn_pod/sw_pod/trunk/pod_v1/p1_common.h
@5 = svn_pod/sw_stm32/trunk/lib/libsupportstm32/gps_time_convert.c
@6 = svn_pod/sw_stm32/trunk/devices/Tim2.c
@7 = svn_pod/sw_pod/trunk/pod_v1/32KHz_p1.h



*/
#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/rtc.h"


#include "gps_packetize.h"
#include "p1_common.h"

/* Packet of GPS time versus RTC tick counter (@5) */
struct TIMESTAMPG pkt_gps_sd;		// GPS time versus rtc tickcounter packet writing to SD card
struct TIMESTAMPG pkt_gps_mn;		// GPS time versus rtc tickcounter packet for monitoring
volatile short gps_sd_pkt_ready;	// SD packet ready:    	0 = not ready, + = ready. ?

/* Flags for folk */
volatile short gps_sd_pkt_ctr;		// SD packet ready:    	increments each upon each time stamp
volatile short gps_mn_pkt_ctr;		// Monitor data ready: 	increments each upon each time stamp
volatile short gps_monitor_flag;		// Monitor data:       	0 = ignore gps, + = collect data
volatile short gps_OK_to_shutdown;	// Shutdown flag: 	0 = Don't shutdown, 1 = OK to shutdown


/* Just for us variables */
static short state_GPS;		// Honorable state machine
static struct USARTLB strlb;	// Holds the return from 'getlineboth' of char count & pointer (@2)
static short gps_limit_ctr;	// Discard the first few fixes
static unsigned int uiGPSrtc;	// RTC tickcount saved at EOL of GPS time
static unsigned short skipct = GPSSDPACKETSSKIP;//Number of secs to skip between SD writes


char cGPS_flag;


/******************************************************************************
 * struct PKT_PTR gps_packetize_poll(void);
 * @brief	: Do all things nice and good for gps
 * @return	: Pointer & count--zero = not ready.
 ******************************************************************************/
struct PKT_PTR gps_packetize_poll(void)
{
	struct PKT_PTR pp = {0,0};	// Default return values (@4)
	time_t tLinuxtimecounter;

	switch (state_GPS)
	{
	case 0:	// Initialization state.

		pkt_gps_mn.id = PKT_GPS_ID;	// ID the gps packets

		/* Limit time that we look for a good GPS fix.  Don't shutdown before a good fix or timeout. */
		nTimercounter[2] = RTC_TICK_FREQ * GPSTIMEOUTCT; // Set time (tick cts) to give up waiting for good fix (@1)

		/* Discard the first few fixes */
		gps_limit_ctr = GPSDISCARDCT;
		
		state_GPS = 1;	// Next case 

	case 1:	// Look for a good response

		/* The following gets a pointer and count to a buffer and steps the pointer in the routine to the next buffer
		   if the line in the buffer is complete. */
		strlb = UART4_rxint_getlineboth();	// Get both char count and pointer (@2) = (@3)

// Debugging: check that raw input lines are coming in
//if (strlb.p > (char*)0)			// Check if we have a completed line (@1)
//{
//USART1_txint_puts(strlb.p);	// Echo back the line just received
//USART1_txint_puts ("\n");	// Add line feed to make things look nice
//USART1_txint_send();		// Start the line buffer sending
//}

		/* Check for a valid time/fix line and extract time and RTC counter at EOL.  */
		if ( (uiGPSrtc = gps_time_stamp(&pkt_gps_mn, strlb) ) > 0 )	// (@5)
		{ // Here we got a valid time/fix $GPRMC record and extracted the time fields */
			if (gps_limit_ctr < GPSDISCARDCT) 
			{ // Here, not through discarding the initial fixes
				gps_limit_ctr += 1;
				break;	// Return until discarded enough readings
			}
			else
			{ // Here, any initial, dubious readings should be long gone.  Time to use it for sync'ing time.

				/* NOTE: refer to '32KHz_p1.h' for description about the synchronization to GPS (@7) */

				/* Convert gps time to linux format (seconds since year 1900) */
				tLinuxtimecounter =  gps_time_linux_init(&pkt_gps_mn);	// Convert ascii GPS time to 32b linux format

				/* Scale linux time upwards to match the RTC_CNT */
				strAlltime.LNX.ull = tLinuxtimecounter << ALR_INC_ORDER;// Scale linux time for 2048 ticks per sec
				
				/* Since a RTC tick could occur during the udpate, disable interrupts */
				NVICICER(NVIC_RTC_IRQ);			// Disable interrupt controller for RTC				

				/* Compute and store difference between the current system tick time, and linux time */
				strAlltime.DIF.ull = strAlltime.LNX.ull - t64RTCsystemcounterTim2IC.ull;

				NVICISER(NVIC_RTC_IRQ);			// Re-enable interrupt controller for RTC
	
				/* Use RTC counter saved at gps time record EOL (@5) */
//				pkt_gps_mn.cnt = uiGPSrtc;	// 

				/* Use RTC counter saved by TIM2 1_PPS interrupt (@6) */
//				pkt_gps_mn.cnt = uiRTCsystemcounterTim2IC;	// 
				pkt_gps_mn.alltime =  strAlltime;	// Add the times & difference to the packet

				/* Log GPS time stamp packets every GPSSDPACKETSSKIP seconds (e.g. 10 secs)  */
				if (++skipct >= GPSSDPACKETSSKIP )	// Time log?
				{ // Here, yes.
					skipct = 0;				// Reset skip counter
					pkt_gps_sd = pkt_gps_mn;		// Copy packet for monitoring to 'sd buffer for SD Carding storing
					gps_sd_pkt_ready += 1;			// Show we have the packet loaded and ready.
					gps_sd_pkt_ctr += 1;			// This looks redundant; maybe it is.
					cGPS_flag = 1;				// RS232_ctl uses this for RS232 shutdown
					gps_OK_to_shutdown = 1;			// Show it is OK to shutdown.		

					/* Change the default return values: pointer and byte count */
					pp.ptr = (char *)&pkt_gps_sd.id;	// Pointer to packet
					pp.ct  = sizeof (struct TIMESTAMPG);	// Byte count
				}
	
				/* This lets 'p1_PC_gps_monitor_gps.c' know there is something to "printf" */
				gps_mn_pkt_ctr += 1;		// Increment this to signal 'p1_PC handler,c' a new packet

				break;
			}
		}
		/* No valid gps reading.  Check if time has run out */
		if (nTimercounter[2] > 0) break;	// Return: No good fixes, but time hasn't run out
		{ // Here, the time ran out before we got any good fixes
			gps_OK_to_shutdown = 1;		// Show it is OK to shutdown, i.e. don't hold up shutdown due to GPS
		}
		break;

	}
	return pp;
}
/******************************************************************************
 * void gps_packetize_restart(void);
 * @brief	: Look for good gps data and store a packet
 ******************************************************************************/
void gps_packetize_restart(void)
{
	state_GPS = 0;
	return;

}

