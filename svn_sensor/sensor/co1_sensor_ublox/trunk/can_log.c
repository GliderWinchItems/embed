/******************************************************************************
* File Name          : can_log.c
* Date First Issued  : 08/09/2016
* Board              : Sensor
* Description        : Logging function
*******************************************************************************/
/*
07-19-2013: rev 212 Start changes to buffering ('canbuf')
08-08-2016: Begin complete revision to database/function approach
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
#include "SD_socket.h"
#include "Tim9.h"
#include "can_hub.h"
#include "../../../../svn_common/trunk/common_highflash.h"
#include "db/gen_db.h"

/* CAN1 control block pointer. (co1.c') */ 
extern struct CAN_CTLBLOCK* pctl0;

/* Holds parameters and associated computed values and readings. */
struct LOGGERFUNCTION logger_f;

/* Pointer to functions to be executed under a low priority interrupt */
void 	(*logpriority_ptr)(void) = 0;	// Address of function to call forced interrupt

/* Running count of CAN msgs written to SD card */
 u32 canlogct = 0;

/* When SD card is not in the socket count msgs by-passed. */
 u32 logbypassctr = 0;		// Count of packets not written/bypassed

/* Highflash command CAN id table lookup mapping. */
static const uint32_t myfunctype = FUNCTION_TYPE_LOGGER;


 u32 canlogSDwritectr = 0;	// Count of SD writes
 u32 logoverrunctr = 0;		// Count of SD card buffer overruns
 int logbuffdepth = 0;		// Max depth into canbuf[];
 u32 logerrct = 0;		// Count log msg overrun errors
 u32 log_can_tim = 0;		// Max time for log_can_log to execute

int sdlog_error = 0;	// Signal mainline unrecoverable SD write

/* Logging buffer [to allow for delay in SD card writing]' */
#define LOGBUFSIZE	512	// SD card write might get far behind
static struct CANRCVBUF canbuf[LOGBUFSIZE];
static u16 canbufidxi = 0;	// Index into 'canbuf' interrupt (adding to)
static u16 canbufidxm = 0;	// Index into 'canbuf' main (removing from)

/* **************************************************************************************
 * static void send_can_msg(uint32_t canid, uint8_t status, uint32_t* pv, struct LOGGERFUNCTION* p); 
 * @brief	: Setup CAN msg with reading
 * @param	: canid = CAN ID
 * @param	: status = status of reading
 * @param	: pv = pointer to a 4 byte value (little Endian) to be sent
 * @param	: p = pointer to a bunch of things for this function instance
 * ************************************************************************************** */
static void send_can_msg(uint32_t canid, uint8_t status, uint32_t* pv, struct LOGGERFUNCTION* p)
{
	struct CANRCVBUF can;
	can.id = canid;
	can.dlc = 5;			// Set return msg payload count
	can.cd.uc[0] = status;
	can.cd.uc[1] = (*pv >>  0);	// Add 4 byte value to payload
	can.cd.uc[2] = (*pv >>  8);
	can.cd.uc[3] = (*pv >> 16);
	can.cd.uc[4] = (*pv >> 24);
	can_hub_send(&can, p->phub_logger);	// Send CAN msg to 'can_hub'
	p->hb_t = tim9_tick_ctr + p->hbct_ticks;// Reset heart-beat time duration each time msg sent
	return;
}
/******************************************************************************
 * static int adv_index(int idx, int size)
 * @brief	: Advance buffer index
 * @param	: idx = incoming index
 * @param	: size = number of items in FIFO
 * return	: index advanced by one
 ******************************************************************************/
static int adv_index(int idx, int size)
{
	idx += 1; if (idx >= size) idx = 0;
	return idx;
}
/******************************************************************************
 * void can_log_trigger_logging(void);
 * @brief 	: Set the low level interrupt to execute sd card write
*******************************************************************************/
void can_log_trigger_logging(void)
{
	/* Trigger a pending interrupt, which will cause a chain of related routines to execute */
	NVICISPR(NVIC_I2C2_ER_IRQ);	// Set pending (low priority 0xe0) interrupt for executing 'can_log_can()'
	return;
}
/******************************************************************************
 * int can_log_init(void);
 * @brief 	: A bit of initialization
 * @return	: 0 = success
 *		: -997 can-hub addition failed
 *		: -998 did not find function code in table
 *		: -999 unreasonable table size
*******************************************************************************/
int can_log_init(void)
{
	int ret;
	unsigned int i;

	/* Set pointer to table in highflash.  Base address provided by .ld file */
// TODO routine to find latest updated table in this flash section

	/* Pointer to 1st parameter area in high flash. */
	extern uint32_t* __paramflash1;	// supplied by .ld file
	logger_f.pparamflash = (uint32_t*)&__paramflash1;

	/* Copy flat high-flash table entries to struct in sram. */
	ret = logger_idx_v_struct_copy(&logger_f.logger_s, logger_f.pparamflash); // return = PARAM_LIST_CT_LOGGER

	/* First heartbeat time */
	// Convert heartbeat time (ms) to timer ticks (needs to recompute for online update)
	logger_f.hbct_ticks = (logger_f.logger_s.hbct * tim9_tick_rate/1000);
	// Compute tim9 tick counter for first heartbeat
	logger_f.hb_t = tim9_tick_ctr + logger_f.hbct_ticks;	

	/* Add this function to the "hub-server" msg distribution. */
	logger_f.phub_logger = can_hub_add_func();	// Set up port/connection to can_hub
	if (logger_f.phub_logger == NULL) return -997;	// Failed

	// Handles non-high priority msgs, e.g. sensor readings-- logging priority
	NVICIPR (NVIC_I2C2_ER_IRQ, NVIC_I2C2_ER_IRQ_PRIORITY_CO );	// Set interrupt priority
	NVICISER(NVIC_I2C2_ER_IRQ);				// Enable interrupt controller

	/* Find command CAN id for this function in table. (__paramflash0a supplied by .ld file) */
	extern uint32_t* __paramflash0a;	// supplied by .ld file
	struct FLASHH2* p0a = (struct FLASHH2*)&__paramflash0a;

	/* Check for reasonable size value in table */
	if ((p0a->size == 0) || (p0a->size > NUMCANIDS2)) return -999;

// TODO get the CAN ID for the ldr from low flash and compare to the loader
// CAN id in this table.

	/* Check if function type code in the table matches our function */
	for (i = 0; i < p0a->size; i++)
	{ 
		if (p0a->slot[i].func == myfunctype)
		{
			logger_f.pcanid_cmd_logger = &p0a->slot[i].canid; // Save pointer
			return ret; // Success!
		}
	}
	return -998;	// Argh! Table size reasonable, but didn't find it.
}
/******************************************************************************
 * int can_log_poll (struct CANRCVBUF* pcan, struct LOGGERFUNCTION* p);
 * @brief 	: Buffer msgs for SD logging and trigger lower level SD write
 * @param	: pcan = pointer to struct with CAN msg
 * @param	: p = pointer to logger function struct with params and more
 * @return	: 0 = No outgoing msgs sent; 1 = one or more msgs were sent
*******************************************************************************/
/*
Enter this routine from CAN_poll_loop
*/
int can_log_poll (struct CANRCVBUF* pcan, struct LOGGERFUNCTION* p)
{
	int ret = 0;
	if (pcan != NULL)
	{ // Here, add CAN msg to buffer for SD writing.
		canbuf[canbufidxi] = *pcan;	// Copy msg to buffer 
		canbufidxi += 1; if (canbufidxi >= LOGBUFSIZE) canbufidxi = 0;// Adv index
		/* Setup time tick msg */
		can_log_trigger_logging();
	}
	/* Check for need to send heart-beat. */
	if ( ( (int)tim9_tick_ctr - (int)p->hb_t) > 0  )	// Time to send heart-beat?
	{ // Here, yes.		
		/* Send heartbeat and compute next hearbeat time count. */
		//      Args:  (CAN id, status of reading, reading pointer, instance pointer)
		send_can_msg(p->logger_s.cid_loghb_ctr, p->status_byte, &canlogct, p); 
		ret = 1;
	}
	/* Check for logger function command msg. */
	if (pcan->id == *p->pcanid_cmd_logger)
	{ // Here, CAN id is command for this function instance 
		// TODO handle command code, set return code if sending msg
		return 1;
	}

	return ret;
}			
/*=========from I2C2_ER_IRQHandler===================================================================
 * 'sdlog log' loops waiting for operation to complete.  This can take many milliseconds, e.g.
 *  we have seen 254 ms worst-case with a Samsung 8 GB class 4 card.  Normally writes can take 
 *  15 or less ms, and average around 6 ms.  To provide for the rare big delay the message buffer
 *  is set to be large.
 *
 * void can_log_can(void);
 * @brief 	: Lowest level interrupt to write any available buffers to SD card
=========from I2C2_ER_IRQHandler=====================================================================*/
static void can_log_can(void)
{
	unsigned int diff;
	unsigned int dwt0;

	/* When the SD card is not in the socket, empty the write buffer. */
	if (SD_socket_sw_status(1) != 0)
	{
		while (canbufidxi != canbufidxm) // Empty buffer
		{
			canbufidxm = adv_index(canbufidxm, LOGBUFSIZE); // Advance queue index
			logbypassctr += 1;	// Count these for error monitoring
			
		}
		log_can_tim = 0;	// Reset max time for SD write
		return; // Don't start any writes if SD inserted switch is off.
	}

	/* Write buffered msgs to SD card. */
	while (canbufidxi != canbufidxm)
	{ // Here, We have a msg to log
		if (SD_socket_sw_status(1) != 0) return; // Don't start any writes if SD inserted switch is off.

		dwt0 = (*(volatile unsigned int *)0xE0001004); 	// Get time of SD write
		sdlog_error = sdlog_write( &canbuf[canbufidxm], sizeof(struct CANRCVBUF) );	// Write packet
		diff = (int)((*(volatile unsigned int *)0xE0001004) - dwt0);	// Compute sys ticks for write
		if (diff > log_can_tim) log_can_tim = diff;	// Save longest time encountered

		if (sdlog_error != 0) return;   // Return: unrecoverable error. 'co1_ublox.c' (main) will see this and reboot.
		canlogSDwritectr += 1;		// Running count of packets written
		canlogct += 1;	// Running ct also
		canbufidxm = adv_index(canbufidxm, LOGBUFSIZE); // Advance queue index
	}
	return;
}
/*#######################################################################################
 * ISR routine for handling logging (sd card writes)
 *####################################################################################### */
void I2C2_ER_IRQHandler(void)
{
	can_log_can();	// Log to sd card if packets are ready (likely if we got here.)

	/* Call other routines if an address is set up */
	if (logpriority_ptr != 0)	// Having no address for the following is bad.
		(*logpriority_ptr)();	// Go do something
	return;
}

