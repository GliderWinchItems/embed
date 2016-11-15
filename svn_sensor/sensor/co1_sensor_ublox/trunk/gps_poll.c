/******************************************************************************
* File Name          : gps_poll.c
* Date First Issued  : 08/09/2016
* Board              : Sensor
* Description        : GPS polling
*******************************************************************************/
/*

Poll the GPS UART to see if there is line buffered.  If so, check it and extract
date|time from $GPRMC lines, then convert that to Linux time, shifted left 6 to 
accommodate 1/64th sec ticks.  

If the even seconds from the GPS differs from the system (SYS), set a flag so that
the Tim4_se.c will pick up the new time.  Once sync'd there should not be any need
for "jamming" a new time as the 1/64th ticks are synchronized to the GPS 1 PPS.

01/26/2014 Rev 361 Updating to add sending on CAN and logging of gps fix in binary.
08/04/2016 begin revision for parameters

*/

#include <string.h>
#include <stdio.h>
//#include <stdio.h>
#include "gps_poll.h"
#include "common.h"
#include "common_time.h"
#include "common_misc.h"
#include "common_can.h"
#include "p1_gps_time_convert.h"
#include "p1_gps_time_linux.h"
//#include "libmiscstm32/printf.h"
//#include "gps_1pps_se.h"
#include "can_log.h"
#include "Tim9.h"
#include "gps_1pps_se.h"
#include "can_hub.h"
#include "CAN_poll_loop.h"
#include "../../../../svn_common/trunk/common_highflash.h"
#include "db/gen_db.h"

/* CAN1 control block pointer. (co1.c') */ 
extern struct CAN_CTLBLOCK* pctl0;

/* Highflash command CAN id table lookup mapping. */
static const uint32_t myfunctype = FUNCTION_TYPE_GPS;

/* Holds parameters and associated computed values and readings. */
struct GPSFUNCTION gps_f;

/* Routine protos */
void debug_datetime(void);
//static void gpsfix_canmsg_add(u32 id, char* ppay, int count);

/* Periodically, set a packet for logging the GPS sentences.  */
#define GPSPACKETSKIP	31		// Number of GPS sentence packets skip between loggings
//static char cGPSpacketctr = 0;		// Skip counter

/* Flags for folk */
volatile short gps_mn_pkt_ctr;		// Monitor data ready: 	increments each upon each time stamp
volatile short gps_monitor_flag;	// Monitor data:       	0 = ignore gps, + = collect data
volatile short gps_OK_to_shutdown;	// Shutdown flag: 	0 = Don't shutdown, 1 = OK to shutdown

/* State for sequencing out lat/lon/ht msgs */
static int gps_llh_state = 0;

/* GPS binary fix msgs for logging & placement on the CAN bus */
#define GPSCANRCVBUFSIZELOG  8	// Number of CAN type gps msgs buffered
static struct CANRCVSTAMPEDBUF gpscanbuflog[GPSCANRCVBUFSIZELOG];	// CAN message circular buffer
static int gpscanbuflogIDXm = 0;		// Index for mainline removing data from buffer
static int gpscanbuflogIDXi = 0;		// Index for interrupt routine adding data
#define GPSCANRCVBUFSIZECAN  8			// Number of CAN type gps msgs buffered
static struct CANRCVBUF gpscanbufcan[GPSCANRCVBUFSIZECAN];	// CAN message circular buffer
static int gpscanbufcanIDXm = 0;		// Index for mainline removing data from buffer
static int gpscanbufcanIDXi = 0;		// Index for interrupt routine adding data

static struct USARTLB strlb;	// Holds the return from 'getlineboth' of char count & pointer (@2)
unsigned int uiConsecutiveGoodGPSctr;	// Count of consecutive good GPS fixes

int nOffsetCalFlag;		// 0 = we have not updated the freq offset calibration


u8 	gps_poll_flag = 0;	// 0 = idle; 1 = update .SYS time
u8	GPStimegood = 0;	// Get started counter
u32 gps_poll_flag_ctr = 0;	// Running count of GPS v SYS time updates
u8	gps_sentence_flag = 0;	// 0 = don't display GPS sentences; not zero = display

struct TIMESTAMPGP1 pkt_gps_mn;		// GPS time versus rtc tickcounter packet for monitoring

/******************************************************************************
 * static int adv_index(int idx, int size)
 * @brief	: Format and print date time in readable form
 * @param	: idx = incoming index
 * @param	: size = number of items in FIFO
 * return	: index advanced by one
 ******************************************************************************/
static int adv_index(int idx, int size)
{
	int localidx = idx;
	localidx += 1; if (localidx >= size) localidx = 0;
	return localidx;
}

/* **************************************************************************************
 * static void send_can_msg(uint32_t canid, uint8_t status, uint32_t* pv, struct GPSFUNCTION* p); 
 * @brief	: Setup CAN msg with reading
 * @param	: canid = CAN ID
 * @param	: status = status of reading
 * @param	: pv = pointer to a 4 byte value (little Endian) to be sent
 * @param	: p = pointer to a bunch of things for this function instance
 * ************************************************************************************** */
static void send_can_msg(uint32_t canid, uint8_t status, uint32_t* pv, struct GPSFUNCTION* p)
{
	struct CANRCVBUF can;
	can.id = canid;
	can.dlc = 5;			// Set return msg payload count
	can.cd.uc[0] = status;
	can.cd.uc[1] = (*pv >>  0);	// Add 4 byte value to payload
	can.cd.uc[2] = (*pv >>  8);
	can.cd.uc[3] = (*pv >> 16);
	can.cd.uc[4] = (*pv >> 24);
	can_hub_send(&can, p->phub_gps);	// Send CAN msg to 'can_hub'
	return;
}
/******************************************************************************
 * int gps_poll_init(void);
 * @brief	: Setup the gps_poll function
 * @return	: 0 = success
 *		: -997 can-hub addition failed
 *		: -998 did not find function code in table
 *		: -999 unreasonable table size
 ******************************************************************************/
int gps_poll_init(void)
{
	int ret;
	unsigned int i;

	/* Set pointer to table in highflash.  Base address provided by .ld file */
// TODO routine to find latest updated table in this flash section

	gps_llh_state = 0;	// jic

	/* Pointer to 1st parameter area in high flash. */
	extern uint32_t* __paramflash2;	// supplied by .ld file
	gps_f.pparamflash = (uint32_t*)&__paramflash2;

	/* Copy flat high-flash table entries to struct in sram. */
	ret = gps_idx_v_struct_copy(&gps_f.gps_s, gps_f.pparamflash); // return = PARAM_LIST_CT_LOGGER

	/* First heartbeat time */
	// Convert heartbeat time (ms) to timer ticks (recompute for online update)
	// Time heartbeat
	gps_f.hbct_tim_ticks = (gps_f.gps_s.hbct_tim * tim9_tick_rate)/1000;
	gps_f.hb_tim_t = tim9_tick_ctr + gps_f.hbct_tim_ticks;
	// lat/lon/ht heartbeat
	gps_f.hbct_llh_ticks = (gps_f.gps_s.hbct_llh * tim9_tick_rate)/1000;
	gps_f.hb_llh_t = tim9_tick_ctr + gps_f.hbct_llh_ticks;	
	// Delay between sequencing the three fix msgs
	gps_f.hbct_llh_delay_ticks = (gps_f.gps_s.hbct_llh_delay * tim9_tick_rate)/1000;

	/* Buffer for between interrupt level for fixes. */
	gps_f.gpsfixidx	= 0; // Index for main->CAN_poll interrupt buffer of struct GPSFIX

	/* Add this function to the "hub-server" msg distribution. */
	gps_f.phub_gps = can_hub_add_func();	// Set up port/connection to can_hub
	if (gps_f.phub_gps == NULL) return -997;	// Failed

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
			gps_f.pcanid_cmd_gps = &p0a->slot[i].canid; // Save pointer
			return ret; // Success!
		}
	}
	return -998;	// Argh! Table size reasonable, but didn't find it.
}/******************************************************************************
 * static void gpsfix_msg(struct GPSFUNCTION* p, int n);
 * @brief 	: Setup and send FIX msgs
 * @param	: p = pointer to logger function struct with params and more
 * @param	: n: 0 = lat, 1, long, 2 = height
 ******************************************************************************/
static void gpsfix_msg(struct GPSFUNCTION* p, int n)
{
	int* ppay;
	struct CANRCVBUF can;	// Temporary CAN msg
	/* Index to last buffer populated by gps_poll (below) at main interrupt level */
	uint32_t idx = p->gpsfixidx ^ 0x1;
	can.id  = p->gps_s.cid_hb_llh;	// CANID: Heartbeat fix (3 msgs) lat/lon/ht
	can.dlc = 6;
// can.dlc = 7; // For debugging
	can.cd.uc[0]  = (p->gpsfix[idx].code << 2); // Upper bits for conversion code
	can.cd.uc[0] |= (p->gpsfix[idx].fix & 0x3); // Fix type in lower two bits

	can.cd.uc[1]  = (p->gpsfix[idx].nsats << 3); // Upper bits for number of satellites
	can.cd.uc[1] |= (n & 0x7);	// Msg index (lat/lon/ht)

	if (n == 2) 	{ppay = &p->gpsfix[idx].ht;}
	else if (n == 1){ppay = &p->gpsfix[idx].lon;}
	else 		{ppay = &p->gpsfix[idx].lat;}
	can.cd.uc[2]  =  (*ppay >> 0);
	can.cd.uc[3]  =  (*ppay >> 8);
	can.cd.uc[4]  =  (*ppay >> 16);
	can.cd.uc[5]  =  (*ppay >> 24);
//can.cd.uc[6] = n; // Added debugging byte
	can_hub_send(&can, p->phub_gps);	// Send CAN msg to 'can_hub' 
	return;
}
/******************************************************************************
 * int gps_poll_can (struct CANRCVBUF* pcan, struct GPSFUNCTION* p);
 * @brief 	: Handles all things associated with the GPS module and CAN
 * @param	: pcan = pointer to struct with icoming CAN msg
 * @param	: p = pointer to logger function struct with params and more
 * @return	: 0 = No outgoing msgs sent; 1 = one or more msgs were sent
 ******************************************************************************/
/* This routine is called from the CAN_poll_loop.c */
int gps_poll_can (struct CANRCVBUF* pcan, struct GPSFUNCTION* p)
{
	int ret = 0;
	
	/* This is non-standard so that it gets the time sync msg ahead of any others
            that might follow. */
	struct CANRCVBUF* pcansync = Tim4_pod_se_sync_msg_get();
	if (pcansync != NULL)
	{
		// Put the time sync msg in the CAN driver queue
		can_hub_send(pcansync, p->phub_gps);	// Send CAN msg to 'can_hub'
		ret = 1;
	}

	while (pcansync != NULL) pcansync = Tim4_pod_se_sync_msg_get();

	/* This flag takes care of the case where the time sync msg is setup and we are further down in
           the loop and should make another pass so the foregoing code sends it now. */
	tim_sync_flag = 0;	

	/* Check for need to send TIME heartbeat. */
	if ( ((int)tim9_tick_ctr - (int)p->hb_tim_t) > 0  )	// Time to send heart-beat?
	{ // Here, yes.		
		/* Send heartbeat and compute next hearbeat time count. */
		send_can_msg(p->gps_s.cid_hb_tim, p->gpsfix[(p->gpsfixidx ^ 0x1)].code,(uint32_t*)&p->tLinuxtimecounter, p);
		p->hb_tim_t = tim9_tick_ctr + p->hbct_tim_ticks; // tick ct for next heartbeat
		ret = 1;	// Show CAN can_hub sending one or more msgs
	}

	/* Check and sequence lat/lon/ht heartbeat msgs. */
	// Rather than dump all three onto the CAN bus, they are sequenced out with a time delay between
	switch (gps_llh_state)
	{
	case 0: // Waiting for to time to start next fix heartbeat three msg burst
		if ( ( (int)tim9_tick_ctr - (int)p->hb_llh_t) > 0  )	// Time to send lat heart-beat?
		{ // Here, yes.		
			p->hb_llh_t = tim9_tick_ctr + p->hbct_llh_delay_ticks; // tick ct for next msg	
			/* Send heartbeat and compute next hearbeat time count. */
			//      Args:  (CAN id, status of reading, reading pointer, instance pointer)
			gpsfix_msg(p, 0);			// Construct longitude msg
			ret = 1;	// Show CAN can_hub sending one or more msgs
			gps_llh_state = 1;	// 
		}
		break;
	case 1: // Waiting to send 2nd msg of three
		if ( ( (int)tim9_tick_ctr - (int)p->hb_llh_t) > 0  )	// Time to send lon heart-beat?
		{ // Here, yes.	
			gpsfix_msg(p, 1);			// Construct longitude msg
			p->hb_llh_t = tim9_tick_ctr + p->hbct_llh_delay_ticks; // tick ct for last msg	
			ret = 1;	// Show CAN can_hub sending one or more msgs
			gps_llh_state = 2;	// 
		}
		break;
	case 2: // Waiting to send 3rd msg of three
		if ( ( (int)tim9_tick_ctr - (int)p->hb_llh_t) > 0  )	// Time to send height heart-beat?
		{ // Here, yes.	
			gpsfix_msg(p, 2);			// Construct longitude msg
			p->hb_llh_t = tim9_tick_ctr + p->hbct_llh_ticks; //tick ct for starting fix burst
			ret = 1;	// Show CAN can_hub sending one or more msgs
			gps_llh_state = 0;	// 	

		}
		break;
	default:
			gps_llh_state = 0;	// 
		break;
				
	}

	/* Check for gps function command msg. */
	if ((pcan != NULL) && (pcan->id == *p->pcanid_cmd_gps))
	{ // Here, CAN id is command for this function instance 
		// TODO handle command code, set return code if sending msg
		ret = 1;	// Show CAN can_hub sending one or more msgs
	}

	return ret;
}
/******************************************************************************
 * void gps_poll(struct GPSFUNCTION* p);
 * @brief 	: Handles all things associated with the GPS module and CAN
 * @param	: p = pointer to logger function struct with params and more
 * @return	: 0 = No outgoing msgs sent; 1 = one or more msgs were sent
 ******************************************************************************/
void gps_poll(struct GPSFUNCTION* p)
{
	char *pgps;
//	u8 ctemp;
	unsigned int gps_ret;
//	time_t tLinuxtimecounter;	// Fancy name for int used by time routines
	int tmp;
//	char vv[64];	// sprintf work
	strlb = USART3_rxint_getlineboth();	// Get both char count and pointer
	if (strlb.ct != 0)
	{ // Here it looks like we got a line of something.
// printf("OK: %s\n\r",strlb.p);USART1_txint_send();
//TODO-error counter

		/* See if there is a '$' on this line, and return ptr to '$' */
		if ((pgps = gps_time_find_dollars(strlb)) == 0) return;	// Null returns: No '$' found

		if (gps_crc_check(strlb) != 0)
		{
			printf("GPS checksum error\n\r"); USART1_txint_send();
//TODO-error counter
			return;
		}

		/* 'l' command sets flag for displaying gps sentences. "Is the gps 'working'?" */
		if (gps_sentence_flag != 0) 
		{
			debug_datetime();
			printf("%s\n\r",strlb.p);USART1_txint_send();
		}
	
		/* Check for a valid time/fix line.  */
		if ( (gps_ret = gps_time_stampGPRMC(&pkt_gps_mn, strlb, 0) ) == 99 )
		{ // Here we got a valid time/fix $GPRMC record and extracted the time fields */

//printf("gps_ret: %d %s\n\r",gps_ret,strlb.p);USART1_txint_send();
//TODO-error counter

			/* Start up--allow for a few fixes to "settle in". */
			if (GPStimegood < 3)	// Startup flag
				GPStimegood += 1;

			/* Convert gps time to linux format (seconds since year 1900) */
			p->tLinuxtimecounter =  gps_time_linux_init(&pkt_gps_mn);	// Convert ascii GPS time to 32b linux format
			strAlltime.GPS.ull  = p->tLinuxtimecounter; 	// Convert to signed long long
			strAlltime.GPS.ull -= (PODTIMEEPOCH);		// Move to more recent epoch to fit into 40 bits
			strAlltime.GPS.ull  = (strAlltime.GPS.ull << 6); // Scale linux time for 64 ticks per sec

//printf("gps_poll GPS %u SYS %u\n\r",(unsigned int)(strAlltime.GPS.ull >> 6),(unsigned int)(strAlltime.SYS.ull >> 6));USART1_txint_send();

			/* GPS and SYS (running time stamp count) should stay in step, once it gets started */
			if ( strAlltime.GPS.ull != (strAlltime.SYS.ull & ~0x3f) ) // Same at the 1 sec level?
			{ // Here, no. Set a flag for 'Tim4_pod_se.c' to pick up the time.
				gps_poll_flag = 1;
				gps_poll_flag_ctr += 1;	// Keep track of these "anomolies" for the hapless programmer.	
			}
			// Add to buffer and send to CAN: ID, pointer to payload, payload count
//?			if ((strAlltime.GPS.ull & (0x7 << 6)) == (0x4 << 6))	// Throttle output.
//? error counter "readings"				gpsfix_canmsg_add( (CAN_UNITID_CO_OLI | CAN_DATAID29_3_GPS_FLAGCTR ), (char*)&gps_poll_flag_ctr, 4);


			/* Setup gps fix from CAN msg $GPRMC for logging and sending on the CAN bus. */
//			tmp = gps_getfix_GPRMC(&gpsfix, strlb); // Convert the lat/long to binary, scaled, minutes * 1E5.
//			if ( tmp == 0)
//			{ // Success.  Send gps fix on CAN bus, and log it. 
//				// Add to buffer and send to CAN: ID, pointer to payload, payload count
//				if ((p->tLinuxtimecounter & 0x7) == 0x5)	// Throttle output of lat|lon CAN msgs
//					gpsfix_canmsg_add( (CAN_UNITID_CO_OLI | (CAN_DATAID_LAT_LONG << CAN_DATAID_SHIFT) ), (char*)&gpsfix.lat, 8);
//				if ((p->tLinuxtimecounter & 0x7) == 0x6)	// Throttle output of height CAN msgs		
//					gpsfix_canmsg_add( (CAN_UNITID_CO_OLI | (CAN_DATAID_HEIGHT   << CAN_DATAID_SHIFT) ), (char*)&gpsfix.ht, 4);
//printf("%9d %9d %9d\n\r",gpsfix.lat, gpsfix.lon, gpsfix.ht); USART1_txint_send();
//			}
//			else
//			{ // Here, some error in the extraction and conversion				
//				sprintf (vv,"$GPRMC FIX ERROR: %d %u\n\r",tmp, strlb.ct, strlb.p);
//				USART1_txint_puts (vv);USART1_txint_send();
//			}

		}
		else
		{

//struct USARTLB strlb;	// Holds the return from 'getlineboth' of char count & pointer
//char vv[64];	// sprintf work
//if (strlb.ct > 3)
//{
//printf("gps_ret: %d %s\n\r",gps_ret,strlb.p);USART1_txint_send();			
//}
			if (isGps_PUBX00(strlb) == 0) // Is this a PUBX00 msg?
			{ // Here yes.
//printf("gps_ret: %d %s\n\r",gps_ret,strlb.p);USART1_txint_send();
				tmp = gps_getfix_PUBX00(&p->gpsfix[p->gpsfixidx], strlb); // Convert the lat/long to binary, scaled, minutes * 1E5, height to meters
				if ( tmp == 0) // Was conversion successful?
				{ // Yes.  Set new index in buffer
					p->gpsfixidx ^= 0x1;	// 0|1 for index
				}
				else
				{
					p->gpsfix[p->gpsfixidx & 1].code = tmp;
					if (tmp == -12)	// Show all fix errors
					{
//TODO This will be a "readings"				gpsfix_canmsg_add( (CAN_UNITID_CO_OLI | CAN_DATAID29_2_GPS_NO_FIX ), 0, 0);
					printf("Conversion unsuccessful: error code=%d (NO FIX) %s\n\r",tmp,strlb.p);USART1_txint_send();
					}
				}
			}
			/* List all other sentences, e.g. software version */
			if (gps_sentence_flag != 0)
			{
				printf ("%u %u %s\n\r",gps_ret,strlb.ct,pgps); USART1_txint_send();
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
void gpsfix_canmsg_add(u32 id, char* ppay, int count)
{
	int i;
	/* Set up id and our linux time stamp */
	gpscanbuflog[gpscanbuflogIDXi].id = id;	// Be sure we have it tagged with the ID
	gpscanbuflog[gpscanbuflogIDXi].U.ull = strAlltime.SYS.ull;	// Add extended linux format time

	/* Set payload count */
	if (count > 8) count = 8; gpscanbuflog[gpscanbuflogIDXi].dlc = count;
	
	/* Copy payload to buffer */
	gpscanbuflog[gpscanbuflogIDXi].cd.ull = 0;	// Zero out unused payload bytes
	if (ppay != 0) // JIC we got a null pointer with a non-zero count
	{
		for (i = 0; i < count; i++)
			gpscanbuflog[gpscanbuflogIDXi].cd.uc[i] = (u8)*ppay++;
	}
	
	/* Save a copy in the can buffer (that will go on the CAN bus). */
	gpscanbufcan[gpscanbufcanIDXi].id  = gpscanbuflog[gpscanbuflogIDXi].id;
	gpscanbufcan[gpscanbufcanIDXi].dlc = gpscanbuflog[gpscanbuflogIDXi].dlc;
	gpscanbufcan[gpscanbufcanIDXi].cd  = gpscanbuflog[gpscanbuflogIDXi].cd;

	/* Advance indices for buffers with wrap-around. */
	gpscanbufcanIDXi = adv_index(gpscanbufcanIDXi, GPSCANRCVBUFSIZECAN);
	gpscanbuflogIDXi = adv_index(gpscanbuflogIDXi, GPSCANRCVBUFSIZELOG);
	
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
	gpscanbuflogIDXm = adv_index(gpscanbuflogIDXm, GPSCANRCVBUFSIZELOG);
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
	gpscanbufcanIDXm = adv_index(gpscanbufcanIDXm, GPSCANRCVBUFSIZECAN);
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

	USART1_txint_send();

	return;
}

