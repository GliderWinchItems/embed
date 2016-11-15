/******************************************************************************
* File Name          : canwinch_pod_common_systick2048.h
* Date First Issued  : 06/30/2013
* Board              : RxT6
* Description        : CAN routines for winch instrumentation--sensor, 2048/sec sync'd
*******************************************************************************/
/* 
See p610 of Ref Manual 
*/

#ifndef __CANWINCH_COMMON_2048
#define __CANWINCH_COMMON_2048

#include "../../../../../svn_discoveryf4/sw_discoveryf4/trunk/lib/libopencm3/cm3/common.h"
#include "../../../../svn_common/trunk/common_can.h"
#include "libopenstm32/can.h"

#define DISABLE_TXINT 	do{volatile int rdbk;CAN_IER(CAN1) &= ~0x1; rdbk = CAN_IER(CAN1);}while(0) // Disable TX RQCPx interrupt (works in an if statement)
#define ENABLE_TXINT	CAN_IER(CAN1) |= 0x1	// Enable: Interrupt generated when RQCPx is set
 

/* Used to reduce the accumulation of error in intervals scale deviation to "whole.fraction" form. */
#define TIMESCALE	16	// Number of bits to scale deviation of clock upwards

/* Averaging the ticks per 1/64th sec */
#define TICKAVEBITS	7	// Number bits in averaging size
#define TICKAVESIZE	(1 << TICKAVEBITS)	// Use this for scaling


struct CAN_XMITBUFF
{
	struct CAN_XMITBUFF* plinknext;	// Linked list pointer (low value id -> high value)
	struct CANRCVBUF can;		// Msg queued
	u16	retryct;		// Counter for number of retries for TERR errors
	u8	maxretryct;		// Maximum number of TERR retry counts
	u8	bits;			// Use these bits to set some conditions (see below)
};

/*  -- bits -- */
#define	SOFTNART	0x01		// 1 = No retries (including arbitration); 0 = retries
#define NOCANSEND	0x02		// 1 = Do not send to the CAN bus



/* struct CAN_PARAMS holds the values used to setup the CAN registers during 'init' */
// baud rate CAN_BTR[9:0]: brp = (pclk1_freq/(1 + TBS1 + TBS2)) / baudrate;

/* Default value string, base on 36 MHz AHB */
#define CAN_PARAMS_DEFAULT {0,500000,2,0,0,4,5,12,1,0,1,0,0}

struct CAN_PARAMS
{
	u32	iamunitnumber;	// Unit number (left justified; see 'common_can.h'
	u32	baudrate;	// E.g. 500000 = 500K bps
	u8	port;		// port: 0 = PA 11|12; 2 = PB; 3 = PD 0|1;  (1 = not valid; >3 not valid) 
	u8	silm;		// CAN_BTR[31] Silent mode (0 or non-zero)
	u8	lbkm;		// CAN_BTR[30] Loopback mode (0 = normal, non-zero = loopback)
	u8	sjw;		// CAN_BTR[24:25] Resynchronization jump width
	u8	tbs2;		// CAN_BTR[22:20] Time segment 2 (e.g. 5)
	u8	tbs1;		// CAN_BTR[19:16] Time segment 1 (e.g. 12)
	u8	dbf;		// CAN_MCR[16] Debug Freeze; 0 = normal; non-zero =
	u8	ttcm;		// CAN_MCR[7] Time triggered communication mode
	u8	abom;		// CAN_MCR[6] Automatic bus-off management
	u8	awum;		// CAN_MCR[5] Auto WakeUp Mode
	u8	nart;		// CAN_MCR[4] No Automatic ReTry (0 = retry; non-zero = auto retry)
};

/******************************************************************************/
int can_init_pod_sys(struct CAN_PARAMS *p);
/* @brief 	: Setup CAN pins and hardware LEGACY CODE ENTRY
 * @param	: p = pointer to 'struct CAN_PARAMS' with setup values
 * @return	:  n = remaining counts; 0 = enter init mode timedout;-1 = exit init mode timeout; -2 = bad port
*******************************************************************************/
int can_init_pod_varbuf_sys(struct CAN_PARAMS *p, u16 nummsgsbuffed);
/* @brief 	: Setup CAN pins and hardware
 * @param	: p = pointer to 'struct CAN_PARAMS' with setup values
 * @param	: nummsgsbuffed = Number of CAN msgs to be buffered
 * @return	:  n = remaining counts; 
 *		:  0 = enter init mode timedout;
 *		: -1 = exit init mode timeout; 
 *		: -2 = bad port
 *		: -3 = not enough sram for CAN xmit buffer
 *		: -4 = nummsgbuffed = 0 (need at least one buffer!)
*******************************************************************************/
void can_filter_unitid_sys(u32 myunitid);
/* @brief 	: Setup filter bank for Checksum check & Program loading
 * @param	: "my" can id
 * @return	: Nothing for now.
*******************************************************************************/
struct CANRCVBUF * canrcv_get_sys(void);
/* @brief	: Get pointer to non-high priority CAN msg buffer
 * @return	: struct with pointer to buffer, ptr = zero if no new data
*******************************************************************************/
struct CANRCVTIMBUF * canrcvtim_get_sys(void);
/* @brief	: Get pointer to high priority CAN msg buffer
 * @return	: struct with pointer to buffer, ptr = zero if no new data
 ******************************************************************************/
int can_msg_put_sys(struct CANRCVBUF *px);
/* @brief	: Load msg into buffer and start mailbox 0
 * @param	: pointer to struct with msg: id, dlc, data
 * @return	: number msg remaining buffered
 * ==> NOTE: dlc is expected to be set when this routine is entered.
 ******************************************************************************/
int can_msg_put_ext_sys(struct CANRCVBUF *pcan, u8 maxretryct, u8 bits);
/* @brief	: Get a free slot and add msg ('ext' = extended version)
 * @param	: pointer to struct with msg: id, dlc, data (common_can.h)
 * @param	: maxretryct =  0 = use TERRMAXCOUNT; not zero = use this value.
 * @param	: bits = Use these bits to set some conditions (see .h file)
 * @return	:  0 = OK; 
 *		: -1 = Buffer overrun (no free slots for the new msg)
 ******************************************************************************/
int can_msg_busy_sys(void);
/* @brief	: Check of CAN msg buffer has any msgs pending
 * @return	: 0 = no msgs in pending list; not 0 = msgs
 ******************************************************************************/
void can_nxp_setRS_sys(int rs, int board);
/* @brief 	: Set RS input to NXP CAN driver (TJA1051) (on some PODs) (SYSTICK version)
 * @param	: rs: 0 = NORMAL mode; not-zero = SILENT mode 
 * @param	: board: 0 = POD, 1 = sensor RxT6 board
 * @return	: Nothing for now.
*******************************************************************************/
void can_msg_rcv_compress_sys(struct CANRCVBUF *px);
/* @brief	: Set .dlc with count to eliminate high order bytes equal zero
 * @param	: pointer to struct with msg: id, dlc & data
 * @return	: nothing
 ******************************************************************************/
void can_msg_rcv_expand_sys(struct CANRCVBUF *px);
/* @brief	: Fill bytes not received with zeros
 * @param	: pointer to struct with msg: id, dlc & data
 * @return	: nothing
 ******************************************************************************/
void can_msg_setsize_sys(struct CANRCVBUF *px, int count);
/* @brief	: Set .dlc with byte count to send
 * @param	: pointer to struct with msg: id, dlc & data
 * @return	: nothing
 ******************************************************************************/
int can_msg_txchkbuff_sys(void);
/* @brief	: Check buffer status for xmit buffer
 * @return	: 0 = buffer full; (CANXMTBUFSIZE - 1) = slots available
 ******************************************************************************/
u16 can_filtermask16_add_sys(u32 m);
/* @brief 	: Add a 16 bit id & mask to the filter banks
 * @param	: mask and ID: e.g. (CAN_UNITID_MASK | (CAN_TIMESYNC3 >> 16)
 * @return	: Filter number; negative for you have filled it up!
 NOTE: These mask|ID's assign to the FIFO 0 (default)
*******************************************************************************/
/*######################### WARNING UNDER INTERRUPT ##################################### */
int canmsg_send(struct CANRCVBUF * p, int data1, int data2);
/* @param	: p = pointer to message to send
 * @param	: data1 = 1st 4 bytes of payload
 * @param	: data2 = 2nd 4 bytes of payload
 * @brief 	: send CAN msg
 * @return	:  0 = OK; 
 *		: -1 = Buffer overrun (no free slots for the new msg)
 *####################################################################################### */
int canmsg_send_sys_n(struct CANRCVBUF * p, u8* pc, s32 n);
/* @param	: p = pointer to message to send
 * @param	: pc = pointer to bytes to send
 * @param	: n = number of bytes to send
 * @brief 	: send CAN msg that is less than 8 bytes
 * @return	:  0 = OK; 
 *		: -1 = Buffer overrun (no free slots for the new msg)
 ******************************************************************************/
void canwinch_set_I2C1_ER_add_sys(void* p_lowpriority_ptr);
/* @brief	: Set the pointer for low priority interrupt call
 * @param	: pointer to routine to call.
 ******************************************************************************/
void canwinch_set_I2C1_EV_add_sys(void* p_highpriority_ptr);
/* @brief	: Set the pointer for fifo1 low priority interrupt call
 * @param	: pointer to routine to call.
 ******************************************************************************/
/* Debug routines */
int pendlistct(void);
int friilistct(void);


/* Pointers to functions to be executed under a low priority interrupt, forced by a CAN interrupt */
/* Pointers to functions to be executed under a low priority interrupt, forced by a CAN interrupt */
// These hold the address of the function that will be called
extern void 	(*highpriority_ptr)(void);		// FIFO 1 -> I2C1_EV  (low priority)
extern void 	(*lowpriority_ptr)(void);		// FIFO 0 -> I2C1_ER  (low priority
extern void 	(*systickLOpriority_ptr)(void);		// SYSTICK -> IC2C2_EV (low priority)
extern void 	(*systickHIpriority_ptr)(void);		// SYSTICK handler (very high priority) (2048/sec)
extern void 	(*systickHIpriorityX_ptr)(void);	// SYSTICK handler (very high priority) (64/sec)
extern void 	(*fifo1veryLOpriority_ptr)(void);	// FIFO 1 -> I2C1_EV -> CAN_sync() -> I2C2_ER (very low priority)

/* Deal with deviation from ideal */
extern u32	ticks64thideal;		// Ideal number of ticks in 1/64th sec

/* "whole.fraction" amounts for adjusting ticks in each SYSTICK interval (64/sec). */
extern s32	deviation_one64th;	// SCALED ticks in 1/64th sec actual

extern volatile u32	stk_64flgctr;		// 1/64th sec demarc counter/flag



/* DWT cycle counter is saved upon SYSTICK interrupt entry */
extern u32 	systicktime;	// Used for speed computations (or other end-of-1/64th messing around)

extern s32	stk_32ctr;		// 1/2048th tick within 1/64th sec counter

/* Error counts for monitoring. */
extern struct CANWINCHPODCOMMONERRORS can_errors;	// A group of error counts

#endif 

