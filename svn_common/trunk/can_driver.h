/******************************************************************************
* File Name          : can_driver.h
* Date First Issued  : 05-28-2015
* Board              : F103 or F4
* Description        : CAN routines common for F103 and F4 CAN drivers
*******************************************************************************/
/* 
*/

#ifndef __CAN_DRIVER_R
#define __CAN_DRIVER_R

#define LDR_RESET	8
#ifndef NULL 
#define NULL	0
#endif


#include "common_can.h"

// Disable TX RQCPx and RX0, and RX1 interrupts for CAN1 and CAN2 (works in an 'if' statement)
//#define DISABLE_ALLCANINT  do{ __attribute__((__unused__)) int rdbk;CAN_IER(CAN1) &= ~0x13; CAN_IER(CAN2) &= ~0x13; rdbk = CAN_IER(CAN1); rdbk = CAN_IER(CAN2);}while(0) 
// Enable the above interrupts
//#define ENABLE_ALLCANINT   CAN_IER(CAN1) |= 0x13; CAN_IER(CAN1) |= 0x13
 
/*  -- bits -- */
#define	SOFTNART	0x01		// 1 = No retries (including arbitration); 0 = retries
#define NOCANSEND	0x02		// 1 = Do not send to the CAN bus

/* struct CAN_PARAMS holds the values used to setup the CAN registers during 'init' */
// baud rate CAN_BTR[9:0]: brp = (pclk1_freq/(1 + TBS1 + TBS2)) / baudrate;

/* Low level interrupts that each higher priority CAN interrupt triggers. 
   Note: vector.c needs to have the vectors as appropriate for
     CAN1 and CAN2--
       CAN_TX, CAN_RX0, CAN_RX1 setup with names:
	CAN1_TX_IRQHandler; CAN1_RX0_IRQHandler; CAN1_RX1_IRQHandler
	CAN2_TX_IRQHandler; CAN2_RX0_IRQHandler CAN1_RX2_IRQHandler
*/  
/* IRQ vector numbers and interrupt prioritys for this CAN module. */
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

/* In the following RX uses 'xw' and TX uses 'xb[]' */
union CAN_X
{
	u32	xw;	// TX dtw
	u8	xb[4];	// RX 
};

// retryct xb[0]	// Counter for number of retries for TERR errors
// maxretryct xb[1]	// Maximum number of TERR retry counts
// bits	   xb[2]		// Use these bits to set some conditions (see below)
// nosend  xb[1]	// Do not send: 0 = send; 1 = do NOT send on CAN bus (internal use only)
/*  -- bits -- */
#define	SOFTNART	0x01		// 1 = No retries (including arbitration); 0 = retries
#define NOCANSEND	0x02		// 1 = Do not send to the CAN bus

struct CAN_POOLBLOCK	// Used for common CAN TX/RX linked lists
{
	 struct CANRCVBUF can;		// Msg queued
volatile struct CAN_POOLBLOCK* volatile plinknext;	// Linked list pointer (low value id -> high value)
	 union  CAN_X x;			// Extra goodies that are different for TX and RX

};

struct CAN_RCV_PTRS	// Pointers for RX linked lists
{
volatile struct CAN_POOLBLOCK*	volatile ptail;	// Pointer Head/Tail, RX0 linked list FIFO
	void 	(*func_rx)(void* pctl, struct CAN_POOLBLOCK* pblk);	// Pointer for extending RX0,1 interrupt processing
	int	bct;	// Buffer counter: Number of msg blocks commandeered by this RX 
	int	bmx;	// Buffer count max: Maxminum number msg blks allowed by this RX
};

/* Here: everything you wanted to know about a CAN module (i.e. CAN1, CAN2) */
struct CAN_CTLBLOCK
{
	u32	vcan;			// CAN1 or CAN2 base address
	
volatile struct CAN_POOLBLOCK  pend;		// Always present block, i.e. list pointer head
volatile struct CAN_POOLBLOCK* volatile pxprv;	// pxprv->plinknext points to msg being sent.  pxprv is NULL if TX is idle.
//volatile struct CAN_POOLBLOCK* volatile  pfor; 	// Loop pointer for the 'for’ loop.  pfor is NULL if the ‘for' loop is not active.

	struct CAN_RCV_PTRS	ptrs0;		//
	struct CAN_RCV_PTRS	ptrs1;		//

	struct CANWINCHPODCOMMONERRORS can_errors;	// A group of error counts
	int	txbct;		// Buffer counter: Number of msg blocks commandeered by this TX 
	int	txbmx;		// Buffer count max: Maxminum number msg blks allowed by this TX
	u32	bogusct;	// Count of bogus CAN IDs rejected
	s8 	ret;		// Return code from routine call
};
/* Misc parameters for initializing  CAN1 and CAN1 */
struct CAN_INIT
{
	u32	numbuff;	// TOTAL (both CAN1&2) ct buffered (1)
	int	txmax;		// Max block ct, this CAN TX can use
	int	rx0max;		// Max block ct, this CAN RX0 can use
	int	rx1max;		// Max block ct, this CAN RX1 can use
};
// (1) the 1stnon-zero value is used when two CANs are init'ed
/* Current counts of blocks commandeered by TX, RX0, RX1 */
struct CAN_BLOCK_CTS
{
	int	catx;		// Max block ct, 1st ctl block CAN TX can use
	int	carx0;		// Max block ct, 1st ctl block CAN RX0 can use
	int	carx1;		// Max block ct, 1st ctl block CAN RX1 can use
	int	cbtx;		// Max block ct, 2nd ctl block CAN TX can use
	int	cbrx0;		// Max block ct, 2nd ctl block CAN RX0 can use
	int	cbrx1;		// Max block ct, 2nd ctl block CAN RX1 can use
};
/******************************************************************************/
struct CAN_CTLBLOCK* can_driver_init(struct CAN_PARAMS2 *p, const struct CAN_INIT* pinit);
/* @brief 	: Setup CAN pins and hardware
 * @param	: p = pointer to 'struct CAN_PARAMS' with setup values
 * @param	: pinit = pointer to msg buffer counts for this CAN
 * @return	: Pointer to control block for this CAN
 *		:  NULL = cannum not 1 or 2, calloc of control block failed
 *		:  Pointer->ret = return code
 *		:   0 success
 *		:  -1 cannum: CAN number not 1 or 2
 *		:  -2 calloc of linked list failed
 *		:  -6 CAN initialization mode timed out
 *		:  -7 Leave initialization mode timed out
 *		: -12 port pin setup CAN1
 *		: -13 port pin setup CAN2
*******************************************************************************/
int can_driver_enable_interrupts(void);
/* @brief	: Enable interrupts after all CAN modules have been initialized
 * @return	: 0 = OK; -1 = no buffers were set up
 ******************************************************************************/
void can_driver_toss0(struct CAN_CTLBLOCK* pctl);
void can_driver_toss1(struct CAN_CTLBLOCK* pctl);
/* @brief	: Release msg buffer back to free list
 ******************************************************************************/
struct CANRCVBUF* can_driver_peek0(struct CAN_CTLBLOCK* pctl);
struct CANRCVBUF* can_driver_peek1(struct CAN_CTLBLOCK* pctl);
/* @brief	: Get pointer to a received msg CAN msg buffer.
 * @return	: struct with pointer to earliest buffer, ptr = zero if no data
 ******************************************************************************/
int can_driver_put(struct CAN_CTLBLOCK* pctl, struct CANRCVBUF *pcan, u8 maxretryct, u8 bits);
/* @brief	: Get a free slot and add msg ('ext' = extended version)
 * @param	: pctl = pointer to control block for this CAN modules
 * @param	: pcan = pointer to msg: id, dlc, data (common_can.h)
 * @param	: maxretryct =  0 = use TERRMAXCOUNT; not zero = use this value.
 * @param	: bits = Use these bits to set some conditions (see .h file)
 * @return	:  0 = OK; 
 *		: -1 = Buffer overrun (no free slots for the new msg)
 *		: -2 = Bogus CAN id rejected
 ******************************************************************************/
u32 can_driver_getcount(struct CAN_CTLBLOCK* pctla, struct CAN_CTLBLOCK* pctlb, struct CAN_BLOCK_CTS* pcts);
/* @brief	: Get current linked list counts for all six possible lists.
 * @param	: pctla = pointer to control block for CAN module ("a")
 * @param	: pctla = pointer to control block for CAN module, if a 2nd one used ("b")
 * @param	: pcts = pointer to stucts with current counts for all six possible lists.
 * @return	: Total count originally calloc'ed
 ******************************************************************************/


/* I don't know why the 'weak' in vector.c wasn't working, so the following was needed. */
void CAN1_TX_IRQHandler(void);
void CAN1_RX0_IRQHandler(void);
void CAN2_RX0_IRQHandler(void);
void CAN1_RX1_IRQHandler(void);
void CAN2_RX1_IRQHandler(void);

#endif 

