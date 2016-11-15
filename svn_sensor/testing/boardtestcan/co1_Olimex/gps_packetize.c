/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : gps_packetize.c
* Hacker	     : deh
* Date First Issued  : 09/05/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Buffering/unbuffering GPS time versus RTC tick counter
*******************************************************************************/
/*
06-24-2012 Modifications for on-board GPS.
*/

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

//#define GPS_SWITCH_PRESENT	// check GPS ON/OFF switch

#include <string.h>


#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/rtc.h"

#include "32KHz_p1.h"
#include "gps_packetize.h"
#include "p1_common.h"

/* Subroutines */
static void gpsfix_time_add(char * p);
static int gpsfix_time_GGAsave(char * p);
//static int gps_power_updn(void);

/* GPS FIX packets */
#define PKTGPSFIXBUFFSIZE  3	// Number of gps fix packets buffered
struct PKT_GPSFIX pkt_gpsfix_buf[PKTGPSFIXBUFFSIZE];	// GPS fix packet buffers
unsigned short usGGAsavectr;	// GGA save flag/ctr for PC monitoring

/* Indices for filling packets under poll */
static unsigned short usPollIdx;		// Index of entry within a packet

/* Indices for retrieving buffered packets for logging */
static unsigned short usMainIdx;	// Index for packet being emptied (main's write to sd)


/* Packet of GPS time versus RTC tick counter (@5) */
struct TIMESTAMPGP1 pkt_gps_sd;		// GPS time versus rtc tickcounter packet writing to SD card
struct TIMESTAMPGP1 pkt_gps_mn;		// GPS time versus rtc tickcounter packet for monitoring
volatile unsigned int gps_sd_pkt_ready;	// SD packet ready: increments each time a packet is ready

/* The following three are for synchronizing the usage of the 'GPS' with the RTC interrupt */

/* Flags for folk */
volatile short gps_mn_pkt_ctr;		// Monitor data ready: 	increments each upon each time stamp
volatile short gps_monitor_flag;	// Monitor data:       	0 = ignore gps, + = collect data
volatile short gps_OK_to_shutdown;	// Shutdown flag: 	0 = Don't shutdown, 1 = OK to shutdown


/* Just for us variables */
static short state_GPS;		// Honorable state machine
static struct USARTLB strlb;	// Holds the return from 'getlineboth' of char count & pointer (@2)
static short gps_limit_ctr;	// Discard fixes until at least 4 secs of consecutive good fixes
static unsigned int uiGPSrtc;	// RTC tickcount saved at EOL of GPS time
//static unsigned short skipct = GPSSDPACKETSSKIP;//Number of secs to skip between SD writes
unsigned int uiConsecutiveGoodGPSctr;	// Count of consecutive good GPS fixes

/* For Xtal & time calibration */
static unsigned int uiTim2Prev;	// Used to compute difference
static unsigned int styflgPrev;	// Previous flag count for testing for new TIM2 input capture data
int nOffsetCalFlag;		// 0 = we have not updated the freq offset calibration

/* These are for flashing the onboard and external LEDs (not BOX LED) */
static char cGPSng;		// 0 = GPS giving not good data, 1 = good data
static unsigned int nDelayCt;	// Count polls to determine if gps

char cGPS_flag;			// Receiving good GPS fixes
char cGPS_ready;		// Enough GPS fixes for it to have settled in

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
//	struct TWO two;			// Two value return from 'rtc_tick_adjust_filter'
	char * pgps;			// Pointer to '$' if found in string

//char vv[80]; // Char array for debugging sprintf
	

	switch (state_GPS)
	{
	case 0:	// Initialization state.
		pkt_gps_mn.id = PKT_GPS_ID;	// Set ID in the gps packets

		/* Discard readings until gps settles down */
		gps_limit_ctr = 0;

		state_GPS = 1;	// Next case 

	case 1:	// Look for a good response

		/* The following gets a pointer and count to a buffer and steps the pointer in the usart routine to the next buffer
		   if the line in the buffer is complete. */
		strlb = UART4_rxint_getlineboth();	// Get both char count and pointer (@2), (@3)

		/* Check for load-cell activity and turn on the GPS power if warranted */
		gps_shutdown_poll();

		/* Check if GPS power switch & supply is ON */
//		if (gps_power_updn() == 0) 
//		{ // Here, it is off, so nothing more we can do with the GPS
//			skipct = 0;	// Reset skip counter
//			/* Flash onboard LED so that we know POD is still running */
//			LEDonboard_ctl_turnon_ct(25, 1000,1);	// ON time, OFF time (ms), one flash

//			break;
//		}
		/* Flash onboard LED so that we know this mode is running */
		if ((cGPS_ready != 0) && ((GPIO_IDR(GPIOD) & (1<<14) ) != 0))	// GPS ready AND GPS power is on
//		if ((GPIO_IDR(GPIOD) & (1<<14) ) != 0) // Double flash when GPS power is on & 
		{
			LEDonboard_ctl_turnon_ct(10, 1000,2);	// ON time, OFF time (ms), two flashes
		}
		else
		{
			LEDonboard_ctl_turnon_ct(25, 1000,1);	// ON time, OFF time (ms), one flash	
		}




// Debugging: check that raw input lines are coming
//char vv[120];
//if (strlb.p > (char*)0)	// Check if we have a completed line (@1)
//{
//sprintf (vv," %d %d", strlb.ct,strlen(strlb.p)); USART1_txint_puts(vv);
//USART1_txint_puts(strlb.p);	// Echo back the line just received
//USART1_txint_puts ("\n");	// Add line feed to make things look nice
//USART1_txint_send();		// Start the line buffer sending
//}

		/* Polling is coming through here looking for an input line */		
#define NOGPSDELAYCT	50000	// Number of pollings before switching to single flash of the LED
		if (strlb.ct == 0)	// Input line ready?
		{ // Here no.
			nDelayCt += 1;
			if (nDelayCt >= NOGPSDELAYCT)
			{
				nDelayCt = NOGPSDELAYCT;
				cGPSng = 0;	// Show gps no good
			}
			
			break;	// Nothing to do
		}
		/* Here, we got some sort of input line, so see if it useful */
		nDelayCt = 0;	// Reset no-gps delay count
z
USART1_txint_puts("# ");
USART1_txint_puts(pgps);	// Echo back the line just received
USART1_txint_puts ("\n");	// Add line feed to make things look nice
USART1_txint_send();		// Start the line buffer sending

		/* See if this looks like a gps sentence */
		if ((pgps = gps_time_find_dollars(strlb)) == 0) break;  // Not a '$' string



		/* Check for a valid time/fix line and extract time and RTC counter at EOL.  */
		if ( (uiGPSrtc = gps_time_stamp(&pkt_gps_mn, strlb), 0 ) == 99 )	// (@5) // 5 Hz onboard version
//		if ( (uiGPSrtc = gps_time_stamp(&pkt_gps_mn, strlb) ) == 1 )	// (@5) // 1 Hz test version
		{ // Here we got a valid time/fix $GPRMC record and extracted the time fields */
//USART1_txint_puts(strlb.p);	// Echo back the line just received
//USART1_txint_puts ("\n");	// Add line feed to make things look nice
//USART1_txint_send();		// Start the line buffer sending
//char vv[96];
//sprintf (vv,"$GPRMC ER: %u %s\n\r",uiGPSrtc,pkt_gps_mn.g);
//USART1_txint_puts (vv);USART1_txint_send();		// Start the line buffer sending

			/* Flag to controlling 1 versus 2 flashes of LED */
			cGPSng = 1;	// Show gps good;

			/* Reset the RS-232 timeout time while we are receiving good GPS lines */
			RS232_ctl_reset_timer();

			/* Reset timeout timer that puts the unit into deepsleep (if no big tension) */
/* NOTE:--The following statement will keep the unit in 'normal run' mode as long as the GPS is active */
			p1_shutdown_timer_reset();

			if (gps_limit_ctr <= GPSDISCARDCT) 
			{ // Here, not through discarding the initial fixes
				/* Blip the external LED on the BOX to amuse the hapless op */
//				LED_ctl_turnon(10,100,2);	// Pulse ON width, space width, two flashes

				gps_limit_ctr += 1;	// Discard counter
				break;	// Return until enough readings discarded 
			}
			else
			{ // Here, any initial, dubious readings should be long gone.  Time to use it for sync'ing time.

				/* Show we have one or more consecutive good GPS readings */
				uiConsecutiveGoodGPSctr += 1;
				
				/* NOTE: refer to '32KHz_p1.h' for description about the synchronization to GPS (@7) */

				/* Convert gps time to linux format (seconds since year 1900) */
				tLinuxtimecounter =  gps_time_linux_init(&pkt_gps_mn);	// Convert ascii GPS time to 32b linux format (@9)


				strAlltime.GPS.ull  = tLinuxtimecounter; 	// Convert to signed long long
				strAlltime.GPS.ull -= (PODTIMEEPOCH);		// Move to more recent epoch to fit into 40 bits (@9)
				strAlltime.GPS.ull  = (strAlltime.GPS.ull << ALR_INC_ORDER);	// Scale linux time for 2048 ticks per sec
				
				/* Compute difference between GPS time and and the SYS tick counter stored by the 1PPS interrupt */
				/* The GPS interrupt (TIM2) stores the current strAlltime.SYS.ull count (maintained by TIM3 divided to simulate
				   the 32 KHz 8192 interrupts) in  strAlltime.TIC.ull.  Later, when we come through this code the difference
				   between the 'TIC and 'GPS times is the offset, or error, in the 'SYS counter.  This difference is then used in
				   p1_tim3_OC_pod.c isr to take out the difference (in small steps, or one big jam-in). */
				strAlltime.DIF.ll = strAlltime.GPS.ull - strAlltime.TIC.ull;	// NOTE: signed result

				/* Update ticks per second used by TIM3 to produce 8192 interrupts per second */
				if ( rtc_tick_edit(strAlltime.uiTim2diff) == 0)	// Is uiTim2diff out-of-range? (tickadjust.c)
				{ // Here, no.
					tim3_tickspersec = strAlltime.uiTim2diff;
				}

				/* GPS 1_PPS drives TIM1 input capture.  This will give processor clock calibration */
				// 'p1_gps_1pps.c' saves the difference between successive readings in 'uiTim1onesec'

				/* Get latest GPS input capture time plus flag */
				/* The following gets the number of timer ticks between 1 pps GPS pulses (strAlltime.uiTim2diff) */
				stY = p1_Tim2_inputcapture_ui();		// Get latest GPS IC time & flag counter
				if (stY.flg != styflgPrev)
				{ // Here, new data.
					styflgPrev = stY.flg;
					strAlltime.uiTim2diff = stY.ic - uiTim2Prev;	// Processor ticks between 1_PPS interrupts
					uiTim2Prev = stY.ic;				// Save for next pass
//sprintf(vv,"%8u\n\r",strAlltime.uiTim2diff);
//USART1_txint_puts(vv);
//USART1_txint_send();
				}

				/* When the GPS starts, the 1st 'strAlltime.uiTim2diff' can be large */
				if ( (rtc_tick_edit(strAlltime.uiTim2diff)) == 0) // Is 'uiTim2diff' out-of-range?  Precludes a double second count
				{ // Here, no.  From the two timer times compute a (32 KHz osc) filtered offset 
					if (uiConsecutiveGoodGPSctr > 4) 
						 cGPS_ready = 1;	// Set flag so that logging can begin

//					two = rtc_tick_filtered_offset(uiTim1onesec, strAlltime.uiTim2diff); // (@10)
//					if (two.n1 == 1) // Check new data flag (see @4 for struct TWO)
//					{ // Here we have a new data output.  Save it for others to use
//						strAlltime.nOscOffFilter = two.n2;	// Save data (see @4 for struct TWO)
						// When we are sure we have enough good readings in the filtering it is safe to update the offset
						// The time adjust flag is to prevent updating for the 'c' command where we are testing and the gps is running
//						if ( (uiConsecutiveGoodGPSctr > 37) && (gps_timeadjustflag == 0) )
//						{ // Here, enough readings, so update offset used to adjust time (@10)
//							strAlltime.nOscOffset32 = strAlltime.nOscOffFilter - nAdcppm_temp_latest;	// Update offset
//							strAlltime.nOscOffset8 =  strAlltime.uiTim2diff    - strAlltime.nPolyOffset8 - 48000000; // Update offset
//							strDefaultCalib.xtal_o8 -= strAlltime.nOscOffset8;	// Save in calibration 
//							cCalChangeFlag = 1; 	// Cause calibration in SD card to update when 'shutdown'
//							nOffsetCalFlag = 1;	// Set flag to show we have updated the offset (@11)
//						}
//						else
//						{
//							nOffsetCalFlag = 0;	// Set flag to show we are not updating offset (@11)
//						}
//					}
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
//				if (++skipct >= GPSSDPACKETSSKIP )	// Time log?
//				{ // Here, yes.
//					skipct = 0;				// Reset skip counter
//					gps_send_time_packet();			// Setup for sending time packet (copy pkt_gps_mn to pkt_gps_sd)
//					cGPS_flag = 1;				// RS232_ctl uses this for RS232 shutdown
//					gps_OK_to_shutdown = 1;			// Show it is OK to shutdown.		
//
//					/* Change the default return values: pointer and byte count */
//					pp.ptr = (char *)&pkt_gps_sd.id;	// Pointer to packet
//					pp.ct  = sizeof (struct TIMESTAMPGP1);	// Byte count
//				}
	
				/* This lets 'p1_PC_gps_monitor_gps.c' know there is something to "printf" */
				gps_mn_pkt_ctr += 1;		// Increment this to signal 'p1_PC handler.c' a new packet

				break;
			}
		}
		else // From: "if ( (uiGPSrtc = gps_time_stamp(&pkt_gps_mn, strlb) ) == 99 )"
		{ // Here, UART4 input line had a '$' but either not a $GPRMC line or the $GPRMC line didn't check out

// Debugging
//USART1_txint_puts(strlb.p);	// Echo back the line just received
//USART1_txint_puts ("\n");	// Add line feed to make things look nice
//USART1_txint_send();		// Start the line buffer sending
//char vv[96];
//sprintf (vv,"$ gps: %u %s\n\r",uiGPSrtc,pkt_gps_mn.g);
//USART1_txint_puts (vv);USART1_txint_send();		// Start the line buffer sending
//break;

			if (uiGPSrtc == 1)
			{
				/* Skip all the sentences until we have good fixes for at least 4 seconds */
				if (gps_limit_ctr >= GPSDISCARDCT) 
				{ // Here, the fixes will have settled down.		
					/* Add this sentence to the gpsfix packet buffer for logging */
					gpsfix_time_add(pgps);
					/* Save the GPGGA sentence for PC display */
					if (gpsfix_time_GGAsave(pgps) == 0)
						usGGAsavectr += 1;
				}
			}
			else
			{
				/* Good $GPRMC readings are ignored except for 1/10th sec equal zero */
				if (uiGPSrtc != 32) // 32 = $GPRMC that has a time that seconds does not end in .0
				{
					/* Discard readings until gps settles down */
					gps_limit_ctr = 0;
	
					/* Reset consecutive good GPS readings counter */
					uiConsecutiveGoodGPSctr = 0;
				}
//				else
//					cGPSng = 0;	// Show gps no good

			}

//				gps_OK_to_shutdown = 1;		// Show it is OK to shutdown, i.e. don't hold up shutdown due to GPS
		}
		break;

	} // End of 'switch (state_GPS)'
	return pp;
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
/******************************************************************************
 * struct PKT_PTR gpsfix_packetize_get(void);
 * @brief	: Get pointer & count to the buffer to be drained.
 * @return	: struct with pointer to buffer & sizeof, ptr & ct = zero if no new data
 ******************************************************************************/
struct PKT_PTR gpsfix_packetize_get(void)
{
	struct PKT_PTR pp = {0,0};		// This holds the return values

	if (usMainIdx == usPollIdx) return pp;	// Return showing no new data
	pp.ptr = (char *)&pkt_gpsfix_buf[usMainIdx]; 	// Set pointer
	pp.ct  = sizeof (struct PKT_GPSFIX);	// Set count
	usMainIdx += 1;				// Advance index for main
	if (usMainIdx >= PKTGPSFIXBUFFSIZE) 	// Wrap-around?
	{ // Here, yes.
		usMainIdx = 0;			// Reset index to beginning.
	}	
	return pp;				// Return with pointer and count
}
/******************************************************************************
 * static void gpsfix_time_add(char * p);
 * @brief	: Add a gps fix sentence to the buffer
 * @param	: p = pointer to string with gps sentence (starting with '$')
 ******************************************************************************/
static void gpsfix_time_add(char * p)
{
//USART1_txint_puts(p); USART1_txint_send();

	/* Set up id and our linux time stamp */
	pkt_gpsfix_buf[usPollIdx].id = ID_GPSFIX;	// Be sure we have it tagged with the ID
	pkt_gpsfix_buf[usPollIdx].U.ull = strAlltime.SYS.ull;	// Add extended linux format time

	/* Copy sentence into buffer and be sure not to over run the buffer */
	strncpy(&pkt_gpsfix_buf[usPollIdx].c[0], p,(PKT_GPSFIX_SIZE-9));
	
	/* Be sure we have a string terminator in case the input line was all cobbled up */
	pkt_gpsfix_buf[usPollIdx].c[(PKT_GPSFIX_SIZE-1)] = 0;

	/* Advance pointer with wrap-around */
	usPollIdx++;		// Advance index of entries within a packet
	if (usPollIdx >= PKTGPSFIXBUFFSIZE) 	// Wrap-around?
	{ // Here, yes.
		usPollIdx = 0;			// Reset index to beginning.
	}	
	return;
}
/******************************************************************************
 * static int gpsfix_time_GGAsave(char * p);
 * @brief	: Extract and save GPGGA sentence
 * @param	: p = pointer to string with gps sentence (starting with '$')
 * @return	: 0 = OK, 1 = not GPGGA
 ******************************************************************************/
#define GGASAVESIZE	64
static char GGAsave[GGASAVESIZE];	// Save values for PC monitoring with 'h' command

static int gpsfix_time_GGAsave(char * p)
{
	char * p0 = p;
	char * pp = "$GPGGA,";
	char * pout;
	int i;

	/* The first 7 chars should match "$GPGGA," */
	for (i = 0; i < 7; i++)
	{
		if (*p++ != *pp++)	
			return 1;
	}	

//USART1_txint_puts(p); USART1_txint_puts("\n\r");USART1_txint_send();


	/* Save time of fix: Hr, min, sec as 'hh:mm:ss.s ' */
	pout = &GGAsave[0];
	*pout++ = *p++; *pout++ = *p++; *pout++ = ':';	// Hour
	*pout++ = *p++; *pout++ = *p++; *pout++ = ':';	// Minute
	*pout++ = *p++; *pout++ = *p++;			// Second
	*pout++ = *p++; *pout++ = *p++; *pout++ = ' ';	// '.s' for 1/10 Seconds


	/* Save latitude: input = 'ddmm.mmmmm' output */
	p++; // Skip ','
	*pout++ = *p++; *pout++ = *p++;	*pout++ = '-';// degrees
	for (i = 0; i < 8; i++)	*pout++ = *p++;
	p++; // Skip ','
	*pout++ = *p++; // Add 'N' or 'S'
	*pout++ = ' ';

	/* Save longitude: input = 'dddmm.mmmmm' */
	p++; // Skip ','
	*pout++ = *p++;	*pout++ = *p++; *pout++ = *p++; *pout++ = '-';	// degrees
	for (i = 0; i < 8; i++)	*pout++ = *p++;
	p++; // Skip ','
	*pout++ = *p++; // Add 'W' or 'E'
	*pout++ = ' ';

	/* Save GPS quality digit */
	p++; 	*pout++ = *p++; *pout++ = ' ';

	/* Save Number of satellites */
	p++; *pout++ = *p++;  *pout++ = *p++; *pout++ = ' ';

	/* Save height */
	p++;
	/* Skip Dilution value and be sure not to hang looking for a '',' */
	while ( (*p++ != ',')  && ((p - p0) < 128) ); 

	/* Copy height to output and be sure not to overrun buffer if ',' is missing */
	while ( (*p != ',') && ( pout < &GGAsave[(GGASAVESIZE - 5)] ) )
			 *pout++ = *p++;

	// Add plenty of terminations to make all beings happy */
	*pout++ = 0x0a; *pout++ = 0x0d; *pout = 0;
	
// USART1_txint_puts(GGAsave); USART1_txint_send();

	return 0;	
}
/******************************************************************************
 * char * gpsfix_get_GGAsave(void);
 * @brief	: Get pointer to zero terminated string with data extracted from $GPGGA sentence
 * @return	: Point to string.
 ******************************************************************************/
char * gpsfix_get_GGAsave(void)
{
	return &GGAsave[0];
}
/******************************************************************************
 * static int gps_power_updn(void);
 * @brief	: Check PAD33 (ON/OFF switch), if ON, power up
 * @return	: 1 = switch is in ON position (short to gnd); 0 = switch OFF (open), pull-up on pin
 ******************************************************************************/
/*
The ON/OFF switch grounds the pin when it is closed.  When the switch is open the pin pull-up
makes the sensed bit a one.  This polarity allows--
1) If the wire to the switch breaks the GPS turns off and doesn't drain the battery.
2) Any wire shorts are more likely to be to ground than plus, and switching to plus instead
   of ground would have raised the risk of shorting the plus supply.
*/

//static int gps_power_updn(void)
//{
//#ifdef GPS_SWITCH_PRESENT
//	/* Check if ON/OFF switch is in the ON position  */
//	if ( (GPSONOFF_IDR & (1 << GPSONOFF_BIT)) != 0) // Check input bit 
//	{ // Here, bit is ON, so switch is in OFF positoin;  the pull-up on the pin 
//		ENCODERGPSPWR_off;	// Diable 5v power regulator
//		return 0;
//	}
//#endif
//	/* Enable +5 supply to gps (if not already on) */
//	ENCODERGPSPWR_on;
//	return 1;
//}


