/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : can_log.c
* Hackerees          : deh
* Date First Issued  : 01/24/2013
* Board              : STM32F103VxT6_pod_mm
* Description        : Post CAN interrupt stuff
*******************************************************************************/
/*
07-19-2013: rev 212 Start changes to buffering ('canbuf')
*/
/* 
Since the main program loop may be slow, 'can_log_can' (which contains a sd card write) is
called via triggering an interrupt with a low level.  If 'can_log_can' were called via polling
from the main program loop, the sd card write might get behind.

*/

#include "canwinch_pod.h"
#include "can_log.h"
#include "sdlog.h"		// Logging routines
#include "common_time.h"
#include "common_can.h"
#include "gps_poll.h"
#include "gps_1pps_se.h"
#include "p1_PC_handler.h"
#include "p1_PC_monitor_can.h"
#include "libusartstm32/nvicdirect.h" 
#include "gps_packetize.h"

/* Pointer to functions to be executed under a low priority interrupt */
void 	(*logpriority_ptr)(void) = 0;	// Address of function to call forced interrupt

/* Running count of CAN msgs written to SD card */
volatile u32 canlogct = 0;
volatile u32 canlogSDwritectr = 0;

/* Logging buffer [to allow for delay in SD card writing]' */
#define LOGBUFSIZE	256	// SD card write might get far behind
static struct CANRCVSTAMPEDBUF canbuf[LOGBUFSIZE];
static u16 canbufidxi = 0;	// Index into 'canbuf' interrupt (adding to)
static u16 canbufidxm = 0;	// Index into 'canbuf' main (removing from)

/* Buffer for monitor output of CAN *msgs* for only *one CAN id* for 'm' command. */
#define CANBUFSIZEM	16	// Number of buffers for CAN msgs that go to PC ('m' command)
static struct CANRCVSTAMPEDBUF monitor_m_canbuf[CANBUFSIZEM];	// Single buffer for passing CAN msgs to PC that match id
static u16 monitor_m_canbufidxi = 0;
static u16 monitor_m_canbufidxm = 0;

/* Buffers for non-CAN messages that will be logged */
// Since sdlog_write is done under interrupt, circular buffering is required
/* Indices into buffers */
#define NONCAN_BUFSIZE	6	// Number of non-CAN items to be buffered (for logging)
static u16 noncanidxi = 0;	// Index into 'noncan' buf (removing under interrupt)
static u16 noncanidxm = 0;	// Index into 'noncan' buf (adding under mainline)
static struct GPSPACKETHDR noncan[NONCAN_BUFSIZE];

/******************************************************************************
 * void noncanlog_add(u8 *p, u8 ct);
 * @brief 	: Add a non-CAN item to be buffered for logging (call from mainline only)
 * @param	: p = pointer to data to be buffered
 * @param	: ct = number of bytes in data to be buffered
*******************************************************************************/
void noncanlog_add(u8 *p, u8 ct)
{
	int i;

	/* Setup the buffer as time|id|data */
	noncan[noncanidxm].U.ull = strAlltime.SYS.ull;	// Add time.  Make header just like CAN messages
	noncan[noncanidxm].id  = CAN_UNITID_GPS;		// Add fake CAN id
	if (ct > (GPSSAVESIZE-1)) ct = (GPSSAVESIZE-1);	// Don't overrun buffer
	noncan[noncanidxm].c[0] = ct;			// 1st byte of array is the byte count
	for (i = 1; i <= ct; i++)			// Copy data (1st byte is byte count)
		noncan[noncanidxm].c[i] = *p++;

	/* Advance the (mainline) index */
	noncanidxm += 1; if (noncanidxm >= NONCAN_BUFSIZE) noncanidxm = 0;

	/* Trigger a pending interrupt */
	NVICISPR(NVIC_I2C2_ER_IRQ);	// Set pending (low priority) interrupt for executing 'can_log_can()'

	return;
}
/*----------------------------------------------------------------------------- 
 *  static struct GPSPACKETHDR* noncanlog_get(void);
 * @brief 	: Get pointer to buffered noncan packets to be logged
-------------------------------------------------------------------------------*/
static struct GPSPACKETHDR* noncanlog_get(void)
{
	struct GPSPACKETHDR* p;
	if (noncanidxi == noncanidxm) return 0;	// Return NULL if nothing buffered.
	p = &noncan[noncanidxi];	// Get pointer to buffer
	/* Advance index */
	noncanidxi += 1; if (noncanidxi >= NONCAN_BUFSIZE) noncanidxi = 0;	
	return p;	// Return pointer to buffer
}
/******************************************************************************
 * void can_log_trigger_logging(void);
 * @brief 	: Set the low level interrupt to execute sd card write
*******************************************************************************/
void can_log_trigger_logging(void)
{
	/* Trigger a pending interrupt, which will cause a chain of related routines to execute */
	NVICISPR(NVIC_I2C2_ER_IRQ);	// Set pending (low priority) interrupt for executing 'can_log_can()'
	return;
}
/*#######################################################################################
void can_log_puttime(struct CANRCVBUF *p, unsigned long long ull);	
 * brief Set up CAN time sync msg for logging. Buffer once per sec msg with time
 * Enter here under a high priority interrupt from Tim4_pod_se.c
 * param	: p = pointer to time sync msg (64/sec)
 * param	: ull = system linux time (<<6)
 *####################################################################################### */
static struct CANRCVSTAMPEDBUF canbuftimesync;	// Once per second log the time sync msg
static int canbuftimesync_flag = 0;		// 0 = no new data; 1 = new msg buffered

void can_log_puttime(struct CANRCVBUF *p, unsigned long long ull)	// Set up CAN time sync msg for logging
{
	if ((p->cd.ui[0] & 0x3f) == 0)
	{
		canbuftimesync.U.ull = ull;	// Time stamp
		canbuftimesync.id = p->id; canbuftimesync.dlc = p->dlc; canbuftimesync.cd.ull = p->cd.ull;
		canbuftimesync_flag = 1;
	}
	return;
}

/*#######################################################################################
 * Enter here under a low priority interrupt that is forced by the CAN FIFO 0 or FIFO 1 interrupts
 * Load logable incoming msgs into a buffer for writing to the SD card
 *####################################################################################### */
void can_log_buff (void)
{
	struct CANRCVSTAMPEDBUF* p;
	struct CANRCVTIMBUF* pt;

	while ( (p = canrcv_get()) != 0)
	{ // Log ALL FIFO 0 msgs.

		/* Copy msg from CAN buffer into logging buffer */
		canbuf[canbufidxi] = *p;	// Copy CAN interrupt buff

		/* Advance index */
		canbufidxi += 1; if (canbufidxi >= LOGBUFSIZE) canbufidxi = 0;	

		/* Running count of msgs buffered (mostly for monitoring/debugging purposes) */
		canlogct += 1;					

		/* Buffer any msg id that matches what the PC sent, if 'm' command is in effect. */
		if ((p->id == m_cmd_id) && (monitor_can_state != 0))
		{
			monitor_m_canbuf[monitor_m_canbufidxi] = *p;	// Copy CAN interrupt buff

			/* Advance index */
			monitor_m_canbufidxi += 1; if (monitor_m_canbufidxi >= CANBUFSIZEM) monitor_m_canbufidxi = 0;	
		}

		/* Buffer all msg id's only, if 'n' command is in effect */
		if (monitor_can_state_n != 0)
		{
	 		command_n_count(p->id);	// Build a table of ID|data_types and count messages ('p1_PC_monitor_can.c')
		}
	}

	/* Move all FIFO 1 msgs into the can logging buffer. */
	while ( (pt = canrcvtim_get()) != 0)
	{ // Log ALL FIFO 1 msgs.
		/* Copy msg from received CAN buffer into logging buffer */
	/* (Rather than go through the whole mess and use the same struct, the following does a copy.) */
		canbuf[canbufidxi].U   = pt->U;	
		canbuf[canbufidxi].id  = pt->R.id;	
		canbuf[canbufidxi].dlc = pt->R.dlc;	
		canbuf[canbufidxi].cd  = pt->R.cd;	

		/* Running count of msgs buffered (mostly for monitoring/debugging purposes) */
		canlogct += 1;					

		/* Buffer any msg id that matches what the PC sent, if 'm' command is in effect. */
		if ((canbuf[canbufidxi].id == m_cmd_id) && (monitor_can_state != 0))
		{
			monitor_m_canbuf[monitor_m_canbufidxi] = canbuf[canbufidxi];
			monitor_m_canbufidxi += 1; if (monitor_m_canbufidxi >= CANBUFSIZEM) monitor_m_canbufidxi = 0;	
		}

		/* Buffer all msg id's only, if 'n' command is in effect */
		if (monitor_can_state_n != 0)
		{
	 		command_n_count(canbuf[canbufidxi].id);	// Build a table of ID|data_types and count messages ('p1_PC_monitor_can.c')
		}

		/* Advance index */
		canbufidxi += 1; if (canbufidxi >= LOGBUFSIZE) canbufidxi = 0;	
	}

	return;
}
/******************************************************************************
 * struct CANRCVSTAMPEDBUF* monitor_m_canbuf_get(void);
 * @brief 	: See if buffer with CAN msg is ready for sending to PC
 * @return	: pointer = 0 for data not ready, otherwise points buffer
*******************************************************************************/
struct CANRCVSTAMPEDBUF* monitor_m_canbuf_get(void)
{
	struct CANRCVSTAMPEDBUF* p;
	if (monitor_m_canbufidxi == monitor_m_canbufidxm) return 0;

	p = &monitor_m_canbuf[monitor_m_canbufidxm];
	/* Advance index */
	monitor_m_canbufidxm += 1; if (monitor_m_canbufidxm >= CANBUFSIZEM) monitor_m_canbufidxm = 0;	
	return p;
}
/******************************************************************************
 * void can_log_init(void);
 * @brief 	: A bit of initialization
*******************************************************************************/
void can_log_init(void)
{
	// Handles non-high priority msgs, e.g. sensor readings-- logging priority
	NVICIPR (NVIC_I2C2_ER_IRQ, NVIC_I2C2_ER_IRQ_PRIORITY_CO );	// Set interrupt priority ('../lib/libusartstm32/nvicdirect.h')
	NVICISER(NVIC_I2C2_ER_IRQ);				// Enable interrupt controller ('../lib/libusartstm32/nvicdirect.h')

	/* Once this is set, the 'canwinch_pod.c' FIFO 0 interrupt will trigger a low level interrupt
	   which will go to 'can_log_buff' for storing a msg. */
	lowpriority_ptr = can_log_buff;		// Set low priority FIFO can interrupt entry ('canwinch_pod.c')

	return;
}
/*============================================================================
 * 'sdlog log' loops waiting for operation to complete.  This can take many milliseconds, e.g.
 *  we have seen 254 ms worst-case with a Samsung 8 GB class 4 card.  Normally writes can take 
 *  15 or less ms, and average around 6 ms.  To provide for the rare big delay the message buffer
 *  is set to be large.
 *
 * int can_log_can(void);
 * @brief 	: Polling to write any available buffers to SD card
 * @return	: zero = OK.  negative = a non-recoverable error.
==============================================================================*/
int can_log_can(void)
{
	struct GPSPACKETHDR* pnon;
	struct CANRCVSTAMPEDBUF* p;
	int error = 0;

	/* Catch SD writing with incoming buffering. */
	while (canbufidxi != canbufidxm)
	{ // Here, We have a buffer to log

		if (tim4_readyforlogging == 0x3) // Is gps time and syncing ready for logging?
		{ // Here, yes, don't skip
			error = sdlog_write( &canbuf[canbufidxm], sizeof(struct CANRCVSTAMPEDBUF) );	// Write packet
			if (error != 0) return error;
			canlogSDwritectr += 1;	// Running count of packets written (monitoring and debugging)
		}
		/* Advance 'm' index */
		canbufidxm += 1; if (canbufidxm >= LOGBUFSIZE) canbufidxm = 0;	
	}

	if (canbuftimesync_flag != 0)
	{ // Here, there is a new time sync msg to be logged 
			canbuftimesync_flag = 0;	// Reset buffer flag
			error = sdlog_write( &canbuftimesync, sizeof(struct CANRCVSTAMPEDBUF) );	// Write packet
			if (error != 0) return error;
			canlogSDwritectr += 1;	// Running count of packets written (monitoring and debugging)	
	}

	while ( (p = gpsfix_canmsg_get_log()) != 0)
	{
			error = sdlog_write( p, sizeof(struct CANRCVSTAMPEDBUF) );	// Write packet
			if (error != 0) return error;
			canlogSDwritectr += 1;	// Running count of packets written (monitoring and debugging)	
	}

	/* Log all buffered, non-CAN messages */
	while ( (pnon = noncanlog_get()) != 0)	// NULL pointer return signals that nothing remains buffered
	{ // Here, something to log.  The size is variable--Fixed length header, plus variable data following.
//$$$		error = sdlog_write( (char*)pnon, (pnon->c[0] + sizeof(struct GPSPACKETHDR) - GPSSAVESIZE) );	// Write packet
		if (error != 0) return error;
	}

	return error;
}
/*#######################################################################################
 * ISR routine for handling logging (sd card writes)
 *####################################################################################### */
int sd_error;	// Pass last SD card write error to mainline

void I2C2_ER_IRQHandler(void)
{
//while(1==1);
	sd_error = can_log_can();	// Log to sd card if packets are ready (likely if we got here.)

	/* Call other routines if an address is set up */
	if (logpriority_ptr != 0)	// Having no address for the following is bad.
		(*logpriority_ptr)();	// Go do something

	return;
}


