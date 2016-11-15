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
@5 = svn_pod/sw_pod/trunk/lib/libsupportstm32/p1_gps_time_convert.c
@6 = svn_pod/sw_stm32/trunk/devices/Tim2.c
@7 = svn_pod/sw_pod/trunk/pod_v1/32KHz_p1.h
@8 = svn_pod/sw_pod/trunk/pod_v1/p1_PC_handler.c
@9 = svn_pod/sw_pod/trunk/pod_v1/p1_gps_time_linux.ch
@10 = svn_pod/sw_pod/trunk/pod_v1/tickadjust.c
@11 = svn_pod/sw_pod/trunk/pod_v1/p1_Tim2_pod.c




*/
#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/rtc.h"


#include "gps_packetize.h"
#include "p1_common.h"

/* Packet of GPS time versus RTC tick counter (@5) */
struct TIMESTAMPGP1 pkt_gps_sd;		// GPS time versus rtc tickcounter packet writing to SD card
struct TIMESTAMPGP1 pkt_gps_mn;		// GPS time versus rtc tickcounter packet for monitoring
volatile unsigned int gps_sd_pkt_ready;	// SD packet ready: increments each time a packet is ready

/* The following three are for synchronizing the usage of the 'GPS' with the RTC interrupt */

/* Flags for folk */
volatile short gps_mn_pkt_ctr;		// Monitor data ready: 	increments each upon each time stamp
volatile short gps_monitor_flag;		// Monitor data:       	0 = ignore gps, + = collect data
volatile short gps_OK_to_shutdown;	// Shutdown flag: 	0 = Don't shutdown, 1 = OK to shutdown


/* Just for us variables */
static short state_GPS;		// Honorable state machine
static struct USARTLB strlb;	// Holds the return from 'getlineboth' of char count & pointer (@2)
static short gps_limit_ctr;	// Discard the first few fixes
static unsigned int uiGPSrtc;	// RTC tickcount saved at EOL of GPS time
static unsigned short skipct = GPSSDPACKETSSKIP;//Number of secs to skip between SD writes
unsigned int uiConsecutiveGoodGPSctr;	// Count of consecutive good GPS fixes
static unsigned int uiConsecutiveGBadGPSflag;	// Set flag when we have gone one sec without a good fix

/* For Xtal & time calibration */
static unsigned int uiTim2Prev;	// Used to compute difference
static unsigned int styflgPrev;	// Previous flag count for testing for new TIM2 input capture data
int nOffsetCalFlag;		// 0 = we have not updated the freq offset calibration

char cGPS_flag;

 	int	nOffsetave;		// Average, or filtered ppm

/******************************************************************************
 * struct PKT_PTR gps_packetize_poll(void);
 * @brief	: Do all things nice and good for gps
 * @return	: Pointer & count--zero = not ready.
 ******************************************************************************/
/* This routine is in the main polling loop */
struct PKT_PTR gps_packetize_poll(void)
{
	struct PKT_PTR pp = {0,0};	// Default return values (@4)
	time_t tLinuxtimecounter;	// Fancy name for int used by time routines
	struct TIMCAPTRET32 stY;	// TIM2 returns counts in this 
	struct TWO two;			// Two value return from 'rtc_tick_adjust_filter'
	

	switch (state_GPS)
	{
	case 0:	// Initialization state.

		pkt_gps_mn.id = PKT_GPS_ID;	// Set ID in the gps packets

		/* Limit time that we look for a good GPS fix, i.e. only shutdown after a good fix or timeout. */
		nTimercounter[2] = RTC_TICK_FREQ * GPSTIMEOUTCT; // Set time (tick cts) to give up waiting for good fix (@1)

		/* Discard the first reading or two */
		gps_limit_ctr = 0;

		state_GPS = 1;	// Next case 

	case 1:	// Look for a good response

		/* The following gets a pointer and count to a buffer and steps the pointer in the usart routine to the next buffer
		   if the line in the buffer is complete. */
		strlb = UART4_rxint_getlineboth();	// Get both char count and pointer (@2), (@3)


// Debugging: check that raw input lines are coming in
//if (strlb.p > (char*)0)	// Check if we have a completed line (@1)
//{
//USART1_txint_puts(strlb.p);	// Echo back the line just received
//USART1_txint_puts ("\n");	// Add line feed to make things look nice
//USART1_txint_send();		// Start the line buffer sending
//}

		/* Polling is coming through here fast--looking for an input line */		
		if (strlb.ct == 0)	break;	// Nothing to do

		/* We have an input line.  Is it a GPS time fix line? (The above 'if' is redundant) */
//		if (strlb.ct != 72)	break;	// No, either a different GPS sentence or something bogus

		/* Check for a valid time/fix line and extract time and RTC counter at EOL.  */
		if ( (uiGPSrtc = gps_time_stamp(&pkt_gps_mn, strlb) ) > 0 )	// (@5)
		{ // Here we got a valid time/fix $GPRMC record and extracted the time fields */

			/* Reset the RS-232 timeout time while we are receiving good GPS lines */
			RS232_ctl_reset_timer();

			/* Reset timeout timer that puts the unit into deepsleep (if no big tension) */
/* NOTE:--The following statement will keep the unit in 'normal run' mode as long as the GPS is active */
			p1_shutdown_timer_reset();

			/* Reset timeout timer (used for timing out to shutdown the RS-232 converter) */
			nTimercounter[2] = RTC_TICK_FREQ * GPSTIMEOUTCT; // Set time (tick cts) to give up waiting for good fix (@1)

			if (gps_limit_ctr <= GPSDISCARDCT) 
			{ // Here, not through discarding the initial fixes
				/* Blip the external LED on the BOX to amuse the hapless op */
				LED_ctl_turnon(10,100,2);	// Pulse ON width, space width, number of pulses

				gps_limit_ctr += 1;	// Discard counter
				break;	// Return until enough readings discarded 
			}
			else
			{ // Here, any initial, dubious readings should be long gone.  Time to use it for sync'ing time.

				/* Show we have one or more consecutive good GPS readings */
				uiConsecutiveGoodGPSctr += 1;
				
				/* Reset consecutive bad GPS readings count */
				uiConsecutiveGBadGPSflag = 0;

				/* NOTE: refer to '32KHz_p1.h' for description about the synchronization to GPS (@7) */

				/* Convert gps time to linux format (seconds since year 1900) */
				tLinuxtimecounter =  gps_time_linux_init(&pkt_gps_mn);	// Convert ascii GPS time to 32b linux format (@9)


				strAlltime.GPS.ull  = tLinuxtimecounter; 	// Convert to signed long long
				strAlltime.GPS.ull -= (PODTIMEEPOCH);		// Move to more recent epoch to fit into 40 bits (@9)
				strAlltime.GPS.ull  = (strAlltime.GPS.ull << ALR_INC_ORDER);	// Scale linux time for 2048 ticks per sec
				
				/* Compute difference between GPS time and and the SYS tick counter stored by the 1PPS interrupt */
				strAlltime.DIF.ll = strAlltime.GPS.ull - strAlltime.TIC.ull;	// NOTE: signed result

				/* GPS 1_PPS drives TIM1 input capture.  This will give processor clock calibration */
				// 'p1_gps_1pps.c' saves the difference between successive readings in 'uiTim1onesec'

				/* Get latest GPS input capture time plus flag */
				stY = p1_Tim2_inputcapture_ui();		// Get latest GPS IC time & flag counter
				if (stY.flg != styflgPrev)
				{
					styflgPrev = stY.flg;
					strAlltime.uiTim2diff = stY.ic - uiTim2Prev;	// Processor ticks between 1_PPS interrupts
					uiTim2Prev = stY.ic;				// Save for next pass
				}

				/* Sometimes when the GPS is plugged in, the 1st 'strAlltime.uiTim2diff' can be large */
				if ( (rtc_tick_edit(uiTim1onesec, strAlltime.uiTim2diff)) == 0)
				{ // From the two timer times compute a (32 KHz osc) filtered offset 
					two = rtc_tick_filtered_offset(uiTim1onesec, strAlltime.uiTim2diff); // (@10)
					if (two.n1 == 1) // Check new data flag (see @4 for struct TWO)
					{ // Here we have a new data output.  Save it for others to use
						strAlltime.nOscOffFilter = two.n2;	// Save data (see @4 for struct TWO)
						// When we are sure we have enough good readings in the filtering it is safe to update the offset
						// The time adjust flag is to prevent updating for the 'c' command where we are testing and the gps is running
						if ( (uiConsecutiveGoodGPSctr > 37) && (gps_timeadjustflag == 0) )
						{ // Here, enough readings, so update offset used to adjust time (@10)
							strAlltime.nOscOffset32 = strAlltime.nOscOffFilter - nAdcppm_temp_latest;	// Update offset
							strAlltime.nOscOffset8 =  strAlltime.uiTim2diff    - strAlltime.nPolyOffset8 - 48000000; // Update offset
							strDefaultCalib.xtal_o8 -= strAlltime.nOscOffset8;	// Save in calibration 
							cCalChangeFlag = 1; 	// Cause calibration in SD card to update when 'shutdown'
							nOffsetCalFlag = 1;	// Set flag to show we have updated the offset (@11)
						}
						else
						{
							nOffsetCalFlag = 0;	// Set flag to show we are not updating offset (@11)
						}
					}
				}
				

				/* 'c' command stops the time adjusting with the gps (@8) */
				if (gps_timeadjustflag == 0)	// When this flag == 1, then don't adjust the time
				{ // Here, someone wants us to adjust the time
					/* Adjust rtc tick time to GPS */
					gps_tickadjust();
				}

// Monitoring crap for debugging
//printf (" %6d %6d %6d",two.n1,two.n2,strAlltime.nOscOffFilter);

//static int secctr; secctr += 1;
//char vv[256];

//sprintf(vv,"DIF %10d %10d %4u",strAlltime.DIF.ul[1],strAlltime.DIF.ul[0],strAlltime.sPreTickTIC);
//USART1_txint_puts(vv);

//sprintf (vv," %5u",secctr);
//USART1_txint_puts(vv);

//sprintf (vv," %5u %2u",DIFtmpctr,DIFjamflag);
//USART1_txint_puts(vv);

//sprintf (vv," %6d %9d",strAlltime.uiNextTickAdjTime,strAlltime.nTickErrAccum);
//USART1_txint_puts(vv);

//sprintf (vv," %6d %6d %6d",Debug_nError,nAdcppm_temp_latest,Debug_nErrorO);
//USART1_txint_puts(vv);

//printf (" %9d %9d", uiTim1onesec,strAlltime.uiTim2diff );
//USART1_txint_puts(vv);

//sprintf (vv," %6u",strAlltime.nOscOffset);
//USART1_txint_puts(vv);

//sprintf (vv," %8d",strAlltime.nOscOffFilter);
//sprintf (vv," %6d %8d",Debug_TIM1,nTim1Debug0);
//USART1_txint_puts(vv);

//USART1_txint_puts("\n\r");
//USART1_txint_send();


				/* Save the whole "mess" of time counts, etc in the 'ALLTIME' struct (@6) */
				pkt_gps_mn.alltime =  strAlltime;	// Add the times & difference to the packet

				/* Log GPS time stamp packets every GPSSDPACKETSSKIP seconds (e.g. 10 secs)  */
				if (++skipct >= GPSSDPACKETSSKIP )	// Time log?
				{ // Here, yes.
					skipct = 0;				// Reset skip counter
					gps_send_time_packet();			// Setup for sending time packet
					cGPS_flag = 1;				// RS232_ctl uses this for RS232 shutdown
					gps_OK_to_shutdown = 1;			// Show it is OK to shutdown.		

					/* Change the default return values: pointer and byte count */
					pp.ptr = (char *)&pkt_gps_sd.id;	// Pointer to packet
					pp.ct  = sizeof (struct TIMESTAMPGP1);	// Byte count
				}
	
				/* This lets 'p1_PC_gps_monitor_gps.c' know there is something to "printf" */
				gps_mn_pkt_ctr += 1;		// Increment this to signal 'p1_PC handler,c' a new packet

				break;
			}
		}
		else
		{ // Here, UART4 input line did not have a good GPS fix

			/* Skip over software version such as this--
				$PGRMT,GPS 18x-LVC software ver. 3.70,,,,,,,,*6B
			*/
			if (strlb.ct == 50)	break;
		
			/* See if this is a good record, showing a bad fix--
				$GPRMC,020040,V,2808.9348,N,08228.5868,W,,,171111,005.0,W*63
			*/

// Debugging: check that raw input lines are coming in
if (strlb.p > (char*)0)	// Check if we have a completed line (@1)
{
printf ("%4u:",strlb.ct);
USART1_txint_puts(strlb.p);	// Echo back the line just received
USART1_txint_puts ("\n");	// Add line feed to make things look nice
USART1_txint_send();		// Start the line buffer sending
}


			/* Reset consecutive good GPS readings counter */
			uiConsecutiveGoodGPSctr = 0;

			/* Count consecutive bad GPS readings */
			uiConsecutiveGBadGPSflag += 1;

			/*  Check if time has run out */
			if (nTimercounter[2] == 0) 
			{ // Here, no good fixes, but time hasn't run out
				state_GPS = 0;			// Start over
				rtc_tick_adjust_ave_reset();	// Reset counts for averaging xtal calibration
				break;	
			}
			else
			{ // Here, the time ran out before we got any good fixes
				gps_OK_to_shutdown = 1;		// Show it is OK to shutdown, i.e. don't hold up shutdown due to GPS
			}
		}
		break;

	} // End of 'switch (state_GPS)'
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
/******************************************************************************
 * struct PKT_PTR gps_packetize_get(void);
 * @brief	: Pointer to packet if ready
 * @return	: Pointer & count--zero = not ready.
 ******************************************************************************/
static unsigned int gps_sd_pkt_readyPrev;	// Used to see if new data is ready

struct PKT_PTR gps_packetize_get(void)
{
	struct PKT_PTR pp = {0,0};	// Default return values (@4)

	if (gps_sd_pkt_ready != gps_sd_pkt_readyPrev)
	{
		gps_sd_pkt_readyPrev = gps_sd_pkt_ready;
		pp.ptr = (char *)&pkt_gps_sd; 	// Set pointer
		pp.ct  = sizeof (struct TIMESTAMPGP1);	// Set count
	}

	return pp;
}
/******************************************************************************
 * void gps_send_time_packet(void);
 * @brief	: Setup packet for sending
 ******************************************************************************/
void gps_send_time_packet(void)
{
	pkt_gps_sd = pkt_gps_mn;		// Copy packet for monitoring to 'sd buffer for SD Carding storing
	gps_sd_pkt_ready += 1;			// Show we have the packet loaded and ready.
	return;
}
