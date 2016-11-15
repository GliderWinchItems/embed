/******************************************************************************
* File Name          : can_driver.h
* Date First Issued  : 05-28-2015
* Board              : F103 or F4
* Description        : CAN routines common for F103 and F4 CAN drivers
*******************************************************************************/
/* 
*/

#ifndef __CANDRIVER
#define __CANDRIVER

#define LDR_RESET	8
#ifndef NULL 
#define NULL	0
#endif

#include "../../../../svn_discoveryf4/sw_discoveryf4/trunk/lib/libopencm3/cm3/common.h"
#include "common_can.h"

#define DISABLE_TXINT 	do{ __attribute__((__unused__)) int rdbk;CAN_IER(CAN1) &= ~0x1; rdbk = CAN_IER(CAN1);}while(0) // Disable TX RQCPx interrupt (works in an if statement)
#define ENABLE_TXINT	CAN_IER(CAN1) |= 0x1	// Enable: Interrupt generated when RQCPx is set
 


struct CAN_XMITBUFF
{
	struct CAN_XMITBUFF* plinknext;	// Linked list pointer (low value id -> high value)
	struct CANRCVBUF can;		// Msg queued
	u16	retryct;		// Counter for number of retries for TERR errors
	u8	nosend;		// Do not send: 0 = send; 1 = do NOT send on CAN bus (internal use only)
	u8	maxretryct;	// Maximum number of TERR retry counts
	u8	bits;			// Use these bits to set some conditions (see below)
};

/*  -- bits -- */
#define	SOFTNART	0x01		// 1 = No retries (including arbitration); 0 = retries
#define NOCANSEND	0x02		// 1 = Do not send to the CAN bus



/* struct CAN_PARAMS holds the values used to setup the CAN registers during 'init' */
// baud rate CAN_BTR[9:0]: brp = (pclk1_freq/(1 + TBS1 + TBS2)) / baudrate;

/* Default value string, base on 36 MHz AHB */
#define CAN_PARAMS_DEFAULT {0,500000,2,0,0,4,5,12,1,0,1,0,0}

/* IRQ vector number and interrupt priority for this CAN module. */
struct CAN_NVIC 
{
	u32	vectnum;	// Interrupt vector number
	u32 	vectpriority;	// Interrupt priority
};
struct CAN_PARAMS2
{
	u32	baudrate;	// E.g. 500000 = 500K bps
	u8	cannum;		// These params apply to-- 0 = CAN1, 1 = CAN2
	u8	port;		// Port code:  CAN1 port: 0 = PA 11|12; 1 = PB 8|9; 2 = PD 0|1;  (port > 3 not valid) 
				//             CAN2 port: 3 = PB 5|6; 4 = PB 12|13 ;  (port > 5 not valid) 
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
	struct CAN_NVIC tx;	// TX  interrupt (number & interrupt priority)
	struct CAN_NVIC rx0;	// RX0 interrupt (number & interrupt priority)
	struct CAN_NVIC rx1;	// RX1 interrupt (number & interrupt priority)
};

/* Low level interrupts that each higher priority CAN interrupt triggers. 
   Note: vector.c needs to have the vectors as appropriate for
     CAN1 and CAN2--
       CAN_TX, CAN_RX0, CAN_RX1 setup with names:
	CAN1_TX_IRQHandler CAN1_RX0_IRQHandler CAN1_RX1_IRQHandler
	CAN2_TX_IRQHandler CAN2_RX0_IRQHandler CAN1_RX2_IRQHandler
*/  

struct CAN_TXPOINTERS
{
volatile struct CAN_XMITBUFF  pend;		// Always present block, i.e. list pointer head
volatile struct CAN_XMITBUFF  frii;		// Always present block, i.e. list pointer head
volatile struct CAN_XMITBUFF* volatile pxprv;	// pxprv->plinknext points to msg being sent.  pxprv is NULL of TX is idle.
volatile struct CAN_XMITBUFF* volatile  pfor; 	// Loop pointer for the 'for’ loop.  pfor is NULL if the ‘for' loop is not active.
};

struct CAN_RCV_PTRS
{
	struct CANRCVBUF* pstart;	// Beginning of buffer
	struct CANRCVBUF* plast;	// End of buffer + 1
	struct CANRCVBUF* padd;		// Pointer to next available to add
	struct CANRCVBUF* ptake;	// Pointer to next to take from
};

/* Here: everything you wanted to know about a CAN module (i.e. CAN1, CAN2) */
struct CAN_CTLBLOCK
{
	u32	vcan;			// CAN1 or CAN2 base address
	void 	(*func_rx)(struct CAN_CTLBLOCK* pctl, struct CAN_RCV_PTRS* ptrs, u32 dtw);	// Pointer for extending RX0,1 interrupt processing
	struct CAN_TXPOINTERS ptx;	// TX linked list pointers
	struct CAN_RCV_PTRS ptrs0;	// RX0 buffer pointers
	struct CAN_RCV_PTRS ptrs1;	// RX1 buffer pointers
	struct CANWINCHPODCOMMONERRORS can_errors;	// A group of error counts
	short 	ret;			// Return from routine call code
};

/******************************************************************************/
struct CAN_CTLBLOCK* can_driver_init(struct CAN_PARAMS2 *p, u32 ntx, u32 nrx0, u32 nrx1);
/* @brief 	: Setup CAN pins and hardware
 * @param	: p = pointer to 'struct CAN_PARAMS' with setup values
 * @param	: ntx  = number of TX  msgs buffered
 * @param	: nrx0 = number of RX0 msgs buffered
 * @param	: nrx1 = number of RX1 msgs buffered
 * @return	: Pointer to control block for this CAN
 *		:  Pointer->ret = return code
 *		:  NULL = cannum not 1 or 2, calloc of control block failed
 *		:   0 success
 *		:  -1 cannum: CAN number not 1 or 2
 *		:  -2 calloc of linked list failed
 *		:  -3 RX0 get buffer failed
 *		:  -4 RX1 get buffer failed
 *		:  -5 port pin setup failed
 *		:  -6 CAN initialization mode timed out
 *		:  -7 Leave initialization mode timed out
*******************************************************************************/
struct CANRCVBUF* can_driver_get0(struct CAN_CTLBLOCK* pctl);
/* @brief	: Get pointer to non-high priority CAN msg buffer (FIFO 0)
 * @return	: struct with pointer to buffer, ptr = zero if no new data
 ******************************************************************************/
struct CANRCVBUF* can_driver_get1(struct CAN_CTLBLOCK* pctl);
/* @brief	: Get pointer to high priority CAN msg buffer (FIFO 1)
 * @return	: struct with pointer to buffer, ptr = zero if no new data
 ******************************************************************************/
int can_driver_put(struct CAN_CTLBLOCK* pctl, struct CANRCVBUF *pcan, u8 maxretryct, u8 bits);
/* @brief	: Get a free slot and add msg ('ext' = extended version)
 * @param	: pctl = pointer to control block for this CAN modules
 * @param	: pcan = pointer to msg: id, dlc, data (common_can.h)
 * @param	: maxretryct =  0 = use TERRMAXCOUNT; not zero = use this value.
 * @param	: bits = Use these bits to set some conditions (see .h file)
 * @return	:  0 = OK; 
 *		: -1 = Buffer overrun (no free slots for the new msg)
 ******************************************************************************/
void CAN1_TX_IRQHandler(void);
void CAN1_RX0_IRQHandler(void);
void CAN2_RX0_IRQHandler(void);
void CAN1_RX1_IRQHandler(void);
void CAN2_RX1_IRQHandler(void);

#endif 

