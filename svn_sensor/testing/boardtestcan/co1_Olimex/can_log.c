/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : can_log.c
* Hackerees          : deh
* Date First Issued  : 01/24/2013
* Board              : STM32F103VxT6_pod_mm
* Description        : Post CAN interrupt stuff
*******************************************************************************/
/* 
There are buffers ('CANBUFSIZE' = 3) for each unit ID and msg type that is loggable.
E.g. 20 units, 8 id's -> 160 buffers that are 3 deep.

When a msg is received, if it is loggable, it placed in the buffer by the unit ID and
msg type index.  This is inefficient in storage, but avoids building a table and searching
it each time a msg comes in.

Since the main program loop may be slow, 'can_log_can' (which contains a sd card write) is
called via triggering an interrupt with a low level.  If 'can_log_can' were called via polling
from the main program loop, the sd card write might get behind.

*/

#include "canwinch_pod.h"
#include "can_log.h"
#include "sdlog.h"		// Logging routines
#include "common_time.h"
#include "gps_poll.h"
#include "gps_1pps_se.h"
#include "p1_PC_handler.h"
#include "p1_PC_monitor_can.h"
#include "libusartstm32/nvicdirect.h" 


/* #### ALERT!  Buffer size might exceed space available #### */
#define CANBUFSIZE	2	// Number of buffers for each (unit ID * data log ID)
#define CANMONBUFSIZE	8	// Number of buffers for CAN msgs that go to PC ('m' command)
#define CANMONNBUFSIZE	4	// Number of CAN id buffers that go to PC ('n' command)

/* Pointer to functions to be executed under a low priority interrupt */
void 	(*logpriority_ptr)(void) = 0;	// Address of function to call forced interrupt

/* Running count of CAN msgs written to SD card */
volatile u32 canlogct = 0;
volatile u32 canlogSDwritectr = 0;

/* Logging buffers: 'canbuf[one for each possible unit ID][depth to allow for delay in SD card writing]' */
static struct CANRCVSTAMPEDBUF canbuf[CAN_MSG_TOTALLOGGED][CANBUFSIZE];

/* Buffer for holding CAN msgs for one CAN id for 'm' command */
static struct CANRCVSTAMPEDBUF monitor_canbuf[CANMONBUFSIZE];	// Single buffer for passing CAN msgs to PC that match id
static u8 monitor_canbufidxi = 0;
static u8 monitor_canbufidxm = 0;

/* Buffer for holding just CAN id's for 'n' command */
static u32 monitor_n_canbuf[CANMONNBUFSIZE];
static u8 monitor_n_canbufidxi = 0;
static u8 monitor_n_canbufidxm = 0;


/* Indices into buffers */
static u8 canbufidxi[CAN_MSG_TOTALLOGGED];	// Index into 'canbuf' interrupt (adding to)
static u8 canbufidxm[CAN_MSG_TOTALLOGGED];	// Index into 'canbuf' main (removing from)

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
 * Enter here under a low priority interrupt that is forced by the CAN FIFO 0 interrupt
 * Load logable incoming msgs into a buffer for writing to the SD card
 *####################################################################################### */
void can_log_buff (void)
{
	struct CANRCVSTAMPEDBUF* p;
	u32 unitid;
//while(1==1);
	/* Buffer only log'able msgs */
	while ( (p = canrcv_get()) != 0)
	{
		unitid = ((p->id & CAN_UNITID_MASK) >> CAN_UNITID_SHIFT);	// Isolate just the UNITID
		if ( (unitid >= CAN_MSG_BS) && (unitid <= CAN_MSG_BE)  )	// Fall within the loggable range?
		{ // Here, yes.  The UNITID is one that is to be logged
			if ( ( p->id & CAN_EXTRID_SHIFT ) == 0 )		// Is the data type to be logged?
			{ // Here, yes.  UNITID and no extra data field bits
				/* Copy msg from CAN buffer into logging buffer */
				canbuf[unitid][canbufidxi[unitid]] = *p;	// Copy CAN interrupt buff

				/* Advance index */
				canbufidxi[unitid] += 1; if (canbufidxi[unitid] >= CANBUFSIZE) canbufidxi[unitid] = 0;	

				/* Running count of msgs buffered (mostly for monitoring/debugging purposes) */
				canlogct += 1;					
			}
		}
		/* Buffer any msg id that matches what the PC sent, if 'm' command is in effect. */
		if ((p->id == m_cmd_id) && (monitor_can_state != 0))
		{
			monitor_canbuf[monitor_canbufidxi] = *p;	// Copy CAN interrupt buff

			/* Advance index */
			monitor_canbufidxi += 1; if (monitor_canbufidxi >= CANMONBUFSIZE) monitor_canbufidxi = 0;	
		}
		/* Buffer all msg id's only, if 'n' command is in effect */
		if (monitor_can_state_n != 0)
		{
			monitor_n_canbuf[monitor_n_canbufidxi] = p->id;	// Copy just CAN id

			/* Advance index */
			monitor_n_canbufidxi += 1; if (monitor_n_canbufidxi >= CANMONNBUFSIZE) monitor_n_canbufidxi = 0;	
		}
 

	}
	/* Trigger a pending interrupt, which will cause a chain of related routines to execute */
	NVICISPR(NVIC_I2C2_ER_IRQ);	// Set pending (low priority) interrupt for executing 'can_log_can()'
	return;
}
/******************************************************************************
 * struct CANRCVSTAMPEDBUF* monitor_canbuf_get(void);
 * @brief 	: See if buffer with CAN msg is ready for sending to PC
 * @return	: pointer = 0 for data not ready, otherwise points buffer
*******************************************************************************/
struct CANRCVSTAMPEDBUF* monitor_canbuf_get(void)
{
	struct CANRCVSTAMPEDBUF* p;
	if (monitor_canbufidxi == monitor_canbufidxm) return 0;

	p = &monitor_canbuf[monitor_canbufidxm];
	/* Advance index */
	monitor_canbufidxm += 1; if (monitor_canbufidxm >= CANMONBUFSIZE) monitor_canbufidxm = 0;	
	return p;
}
/******************************************************************************
 * u32 monitor_n_canbuf_get(void);
 * @brief 	: See if buffer with CAN id is ready for sending to PC
 * @return	: 0 = none, otherwise holds ID (ID == 0 not assigned)
*******************************************************************************/
u32 monitor_n_canbuf_get(void)
{
	if (monitor_n_canbufidxi == monitor_n_canbufidxm) return 0;

	/* Advance index */
	monitor_n_canbufidxm += 1; if (monitor_n_canbufidxm >= CANMONNBUFSIZE) monitor_n_canbufidxm = 0;	
	return monitor_n_canbuf[monitor_n_canbufidxm];
}
/******************************************************************************
 * void can_log_init(void);
 * @brief 	: A bit of initialization
*******************************************************************************/
void can_log_init(void)
{
	u32 i;

	for (i = 0; i < CAN_MSG_TOTALLOGGED; i++)
	{ // Very important that these be initially zero...(can you trust static variables to be initially zero?...maybe)
		canbufidxi[i] = 0; canbufidxm[i] = 0;
	}

	// Handles non-high priority msgs, e.g. sensor readings-- logging priority
	NVICIPR (NVIC_I2C2_ER_IRQ, NVIC_I2C2_ER_IRQ_PRIORITY );	// Set interrupt priority ('../lib/libusartstm32/nvicdirect.h')
	NVICISER(NVIC_I2C2_ER_IRQ);				// Enable interrupt controller ('../lib/libusartstm32/nvicdirect.h')



	/* Once this is set, the 'canwinch_pod.c' FIFO 0 interrupt will trigger a low level interrupt
	   which will go to 'can_log_buff' for storing a msg. */
	lowpriority_ptr = can_log_buff;		// Set low priority FIFO can interrupt entry ('canwinch_pod.c')


	return;
}
/******************************************************************************
 * void can_log_can(void);
 * @brief 	: Polling to write any available buffers to SD card
 * @return	: none
*******************************************************************************/
void can_log_can(void)
{
	u32 i;
	struct PP pp = get_packet_GPS();

	/* Check if there is a GPS sentence to be logged */
	if (pp.p != 0)	// If pointer is zero, there is sentence ready
	{
		sdlog_write( pp.p, pp.ct);	// Write packet
		cGPSpacketflag = 0;		// Reset flag
	}
	/* Check all possible (loggable) buffers */
	for (i = 0; i < CAN_MSG_TOTALLOGGED; i++)
	{
		if (canbufidxi[i] != canbufidxm[i])	// New data?
		{ // Here, yes.  We have a buffer to log

			if (tim4_readyforlogging == 0x3) // Is gps time and syncing ready for logging?
			{ // Here, yes.
				sdlog_write( &canbuf[i][canbufidxm[i]], sizeof(struct CANRCVSTAMPEDBUF) );	// Write packet
				canlogSDwritectr += 1;
			}

			/* Advance 'm' index */
			canbufidxm[i] += 1; if (canbufidxm[i] >= CANBUFSIZE) canbufidxm[i] = 0;	
		}
	}
	return;
}
/*#######################################################################################
 * ISR routine for handling logging (sd card writes)
 *####################################################################################### */
void I2C2_ER_IRQHandler(void)
{
//while(1==1);
	can_log_can();	// Log to sd card if packets are ready (likely if we got here.)

	/* Call other routines if an address is set up */
	if (logpriority_ptr != 0)	// Having no address for the following is bad.
		(*logpriority_ptr)();	// Go do something
	return;
}

