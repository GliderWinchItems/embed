/******************************************************************************
* File Name          : canwinch_pod_common_systick2048.c
* Author             : deh
* Date First Issued  : 06/30/2013
* Board              : RxT6
* Description        : CAN routines for winch instrumentation--sensor, 2048/sec sync'd
*******************************************************************************/
/*
05/26/2015 rev 689 Added volatile to pfor and pxprv and that appears to 
  have fixed the missing 'loadlmbx's.

05/15/2015 rev 646 Singly linked list for TX (seems to work) 
  Problem corrected--
This corrects the issue that a low priority msg queued in the mailbox which is losing
arbitration to other higher priority msgs on the CAN bus cannot delay the sending of
high priority msg(s) that the program has added to the queue.

With this change the highest priority msg in the list will always get the next arbitration 
on the CAN bus.  

The order of sending msgs with the same priority remains in the order they are added to
the list.

  Summary of linked list approach:

  Linked list--
There are two "head"s to the linked list:
  'pend' points to a struct for the head of the "msgs pending" list.
  'frii' points to a struct for the head of the "slots free" list.

  Initialization--
The number of msgs to be buffered is passed in as an argument, and
'malloc' is called to get memory for each msg.  In this process the msg buffers are
linked with a pointer (named 'plinknext').  The last msg in the list has a 'plinknext'
value of NULL (zero).  Initially all the msg buffers are in the free list.  The
pending list head has a 'plinknext' of zero, indicating that the pending list is empty.

  Adding msgs--
Msgs are added in 'can_msg_put_ext_sys' from the mainline, or a low level interrupt,
(but not both!).  The process is to first obtain a msg buffer from the free list, 
prepared the msg (i.e. copy the msg and set parameters), then locate, in CAN id
priority, where the msg should be inserted into the pending list so that the pending
list is always in sorted order, with the highest priority msg on top. 

Msgs with the
same CAN id are added following the last msg with the same CAN id so that they are
sent in the order in which they arrive.

  TX interrupt--
The hardware NART bit is set on so that there is a TX interrupt after each attempt to 
send.  After each attempt, the msg that is highest priority on the pending list is loaded
next (which is simply the msg pointed to by pend.plinknext, unless it is zero in which
case the pend list is now empty).

If a msg attempt results in a TERR flag (transmission error), then a count for that msg
is advanced and if the count exceeds the max, the msg is removed from the pending list and
placed on the free list.  If a msg attempt results in an arbitration failure, the msg
remains on the list until it either results in an OK, or TERR failure.

In all cases the highest priority msg is always loaded next.  That way if a low priority
msg is trying to be sent, and the mainline loads a higher priority msg(s), the highest 
priority msg will be attempted at the next arbitration on the CAN bus and that lower
priority msg(s) will get their turn after the highest priority msg is completed.
    
  Notes on strategy--
Generally, the pointer being used points to a block *before* the msg of interest.  The
msg of interest is pointed to by 'ptr->plinknext'.  This scheme is used since unlinking
a block requires having the pointer before as well as after the block of interest.

'pxprv' is a pointerl used by the TX interrupt routine.  When a msg is loaded into the mailbox
it is the highest priority msg in the pending list at that time.  While that msg is being
sent other msgs may have been added to the list.  When the TX interrupt occurs it needs to
know the msg that was being sent, which is pointered to 'pxprv->plinknext'.  This poses a
special problem with the addition of msgs, in that a msg might be inserted between 'pxprv'
and 'pxprv->plinknext'.  Attention to TX interrupt locking and the linking/unlinking has 
been given so that an "unfortunate coincidence" doesn't cause problems.


07/17/2013 rev 209 has glitch problem with systick interrupt priority not being set resolved.
*/
/* 
Page numbers: Ref Manual RM0008 rev 14
CAN starts at p 628.

This routine is based on 'canwinch_pod.c'.

This routine combines the CAN msg handling with the time sync'ing to the incoming
time sync CAN msgs from the unit with the GPS (e.g., see ../sensors/co1_Olimex/trunk).

The DTW_CYCCNT (32b) counter is used to measure the number of processor ticks between
time sync msgs.  This measure is used to time the 1/64th sec interval.  The interval 
is timed using the SYSTICK counter (24b).  With a nominal 64,000,000 system clock there
are 31250 system ticks per 1/2048 tick.


Interrupt sequences--
NOTE: pay attention to the 'EV' versus 'ER' for the lower level interrupt handlers.

1) void CAN_RX1_Handler(void) -- High priority CAN time sync msgs (FIFO 1)

The highest interrupt priority is assigned to this vector.

Only high priority msgs are assigned to FIFO 1.  These are three possible 
time sync msgs and a 'reset' msg.

For timing computations 'fifo1cycnt' and 'stk_val' are set with the current 
DTW_CYCNT counter and SYSTICK value.  The msg is stored in a circular buffer
and a flag counter incremented.  To continue the less time dependent work an
interrupt with a low priority is forced--

  void I2C1_EV_IRQHandler(void)

This interrupt handler calls 'CAN_sync' that does the editing and computation
of the time duration between CAN time sync msgs.  CAN_sync ends by forcing a
still lower priority interrupt.  (Before returing, it will also call a funciton
if a pointer to the function is non-null) --

 void I2C2_ER_IRQHandler(void)

This interrupt handler will call a function to continue if the pointer is 
non-null.


2) void USB_LP_CAN_RX0_IRQHandler(void) -- Lower priority CAN msgs (FIFO 0)

This routine saves the msg in a circular buffer and increments the flag counter,
then forces a lower level priority interrupt--

  void I2C1_ER_IRQHandler(void)

This routine calls a function if the pointer is not null.


3) void USB_HP_CAN_TX_IRQHandler(void) -- Sending CAN msgs

With breathtaking brevity this routine sends another CAN msg if the
circular buffer is not empty.


4) void SYSTICK_IRQHandler (void) -- 1/64th sec duration timing

This routine is next in priority behind the FIFO 1.

This routine checks if a FIFO 1 interrupt has taken place and if so
does the timing & phasing adjustment.  If not it uses the old values
for the timing.

A call to a function will take place if a pointer to the function is
not null.  The purpose is for this function to do some simple (*quick*)
closeout of any measurements, e.g. save a couple of values needed for
the later computation of speed.  Following this a low level interrupt 
is forced --

  void I2C2_EV_IRQHandler(void)

This routine calls function if the pointer is not null, e.g. in 
../sensor/pod6/trunk/, 'tick_po6.c' is called to complete the
preparation of the measurement and set the msg in the buffer for
sending.
*/

/*  Interrupt triggering tree--
void CAN_RX1_Handler(void) 		highpriority_ptr	[FIFO1 interrupt]
  void I2C1_EV_IRQHandler(void)		systickLOpriority_ptr	
    void CAN_sync(u32 fifo1cycnt)				[compute system clock rate]

void SYSTICK_IRQHandler (void)					[2048 per sec nominal rate]
  systickHIpriority_ptr						[Hi priority post SYSTICK interrupt--don't tarry]
  void I2C2_ER_IRQHandler(void)		fifo1veryLOpriority_ptr	[Send measurements] (tick_pod6.c sets pointer)

*/
#include <malloc.h>
#include "db/gen_db.h"
#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/gpio.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/can.h"
#include "libopenstm32/scb.h"
#include "libopenstm32/systick.h"
#include "PODpinconfig.h"
#include "pinconfig_all.h"
#include "pinconfig.h"
#include "SENSORpinconfig.h"
#include "canwinch_pod_common_systick2048.h"
#include "DTW_counter.h"

#define LDR_RESET	8
#ifndef NULL 
#define NULL	0
#endif

#define DISABLE_INTS	__asm__ volatile ("CPSID I")
#define ENABLE_INTS	__asm__ volatile ("CPSIE I")

/* There is a delay between the absolute GPS sync'd "tick" and the CAN time sync msg being received by the sensor. */
#define CAN_TICK_DELAY	5600	// CAN msg setup and transmission delay
//#define CAN_TICK_DELAY	0	// CAN msg setup and transmission delay (TEST)

/* Averaging the ticks per 1/64th sec */
static s16	ticksave[TICKAVESIZE];
static u16	tickaveidx = 0;
static u16	ticksaveflag = 0;
static s32	tickavesum = 0;

/* Nominal clock ticks in 1/64th second */
static u32	systicknominal;		// Nominal number of ticks in 1/64th sec
s32	stk_32ctr;		// 1/2048th tick within 1/64th sec counter
static u32	fifo1msgflg;		// Running count of FIFO 1 interrupts
volatile u32	stk_64flgctr;		// 1/64th sec demarc counter/flag


/* Deal with deviation from ideal */
u32	ticks64thideal = 1000000;	// Ideal number of ticks in 1/64th sec

/* "whole.fraction" amounts for adjusting ticks in each SYSTICK interval (64/sec). */
s32		deviation_one64th;	// SCALED ticks in 1/64th sec actual

/* These are used (mostly!) by the lowlevel routine */
static u32	can_ticksper64thHI;	// TIM4 clock ticks for one sec: high limit
static u32	can_ticksper64thLO;	// TIM4 clock ticks for one sec: low limit

/* The following variables are used by peripherals in their 'init' routines to set dividers 	*/
/* (see 'lib/libmiscstm32/clockspecifysetup.c') */
extern u32	pclk1_freq;	/*	SYSCLKX/PCLK1DIV	E.g. 72000000 	*/
extern u32	sysclk_freq;	/* 	SYSCLK freq		E.g. 72000000	*/

/* Interrupts with received messages that are accepted by the hardware filtering are stored in
   a buffer.  The mainline gets a pointer to the buffer, or a NULL if the buffer is empty. */
// The following is for non-high priority CAN msgs (e.g. sensor readings, date/time/tick count, prog loadiing)

#define CANRCVBUFSIZE	16
static struct CANRCVBUF canbuf[CANRCVBUFSIZE];	// CAN message circular buffer
static int canbufIDXm = 0;		// Index for mainline removing data from buffer
static int canbufIDXi = 0;		// Index for interrupt routine adding data
// The following is for high priority CAN msgs (e.g. RESETALL, TIMESYNCx)

#define CANRCVTIMBUFSIZE	8	// Can data plus timer ticks 
static struct CANRCVTIMBUF cantimbuf[CANRCVTIMBUFSIZE];	// CAN message circular buffer
static int canbuftimIDXm = 0;		// Index for mainline removing data from buffer
static int canbuftimIDXi = 0;		// Index for interrupt routine adding data

/* Pointers to functions to be executed under a low priority interrupt, forced by a CAN interrupt */
// These hold the address of the function that will be called
void 	(*highpriority_ptr)(void) = 0;		// FIFO 1 -> I2C1_EV  (low priority)
void 	(*lowpriority_ptr)(void) = 0;		// FIFO 0 -> I2C1_ER  (low priority
void 	(*systickLOpriority_ptr)(void) = 0;	// SYSTICK -> IC2C2_EV (low priority)
void 	(*systickHIpriority_ptr)(void) = 0;	// SYSTICK handler (very high priority) (2048/sec)
void 	(*systickHIpriorityX_ptr)(void) = 0;	// SYSTICK handler (very high priority) (64/sec)
void 	(*fifo1veryLOpriority_ptr)(void) = 0;	// FIFO 1 -> I2C1_EV -> CAN_sync() -> I2C2_ER (very low priority)

/* Transmit buffering */
#define CANXMTBUFNUM_DEFAULT	16		// Number of message that can be queued for sending

volatile struct CAN_XMITBUFF  pend;		// Always present block, i.e. list pointer head
volatile struct CAN_XMITBUFF  frii;		// Always present block, i.e. list pointer head
volatile struct CAN_XMITBUFF* volatile pxprv = NULL;// pxprv->plinknext points to msg being sent.  pxprv is NULL of TX is idle.
volatile struct CAN_XMITBUFF* volatile pfor; // Loop pointer for the 'for’ loop.  pfor is NULL if the ‘for' loop is not active.

#define TERRMAXCOUNT	12	// Max number of TERR retries of a msg

u8 defaultnart = 0;	// 

/* Filtering */
static u16 can_filt16num;	// 16 bit scale filter number: last assigned

/* Procesor cycle counter is saved upon FIFO 1 interrupt entry */
// DTW module 32b processor cycle counter
static u32	fifo1cycnt;	// Used for processor ticks per sec measurment

/* DWT cycle counter is saved upon SYSTICK interrupt entry */
u32 	systicktime;	// Used for speed computations (or other end-of-1/64th messing around)

#define CAN_TIMEOUT	2000000		// Break 'while' loop count during CAN initialization

/* Countdown counter for suspending the sending of CAN msgs. */
 u32 can_suspend_msgs_ctr = 0;	// Set by payload from FIFO1 msg CAN_RESETALL|CAN_DATAID_WAITDELAYRESET (appropriately shifted)
static u32 can_suspend_msgs_flag = 0;	// 0 = no new count; 1 = new count loaded
static u32 can_suspend_msgs_work = 0;	// Working counter

/* Prototypes */
static void CAN_sync(u32 fifo1cycnt);	// Edit and compute ticks per sec
static int can_init_pod_common_sys(struct CAN_PARAMS *p);
int can_msg_put_ext_sys(struct CANRCVBUF *pcan, u8 maxretryct, u8 bits);
static void loadmbx(void);

/* Error counts for monitoring. */
struct CANWINCHPODCOMMONERRORS can_errors;	// A group of error counts

/* Counts for debugging. */
/* For debugging: Count the number in the list */
static int listctr(volatile struct CAN_XMITBUFF* p)
{
	int  ct = 0;
	while (p->plinknext != NULL)
	{
		ct += 1;
		p = p->plinknext;
	}
	return ct;
}
int pendlistct(void) {return listctr(&pend);}
int friilistct(void) {return listctr(&frii);}

/******************************************************************************
 * static int adv_index(int idx, int size)
 * @brief	: Advance circular buffer index
 * @param	: idx = incoming index
 * @param	: size = number of items in FIFO
 * return	: index advanced by one
*****************************************************************************/
static int adv_index(int idx, int size)
{
	int localidx = idx;
	localidx += 1; if (localidx >= size) localidx = 0;
	return localidx;
}
/*-----------------------------------------------------------------------------
 * static void SYSTICK_init(void);
 * @brief	: Initializes SYSTICK for interrupting
 ------------------------------------------------------------------------------*/
static void SYSTICK_init (void)
{
	u32	can_ticksper64thdelta;

	STK_CTRL = 0;	// Be sure it is disabled

	/* Ticks per 1/64th sec and set allowable range. */
	systicknominal = (sysclk_freq/2048 );	// Nominal ticks per 1/64th sec

	ticks64thideal = (systicknominal*32);

	/* Set limits for valid processor ticks between 1/64th CAN msg events */
	can_ticksper64thdelta = ( ticks64thideal / 40);	// Allow +/- 2.5% range
	can_ticksper64thHI =      ticks64thideal + can_ticksper64thdelta;
	can_ticksper64thLO =      ticks64thideal - can_ticksper64thdelta;

	/* Set reload register with initial interval for 1/64th sec interrupts. */
	// The high order byte must be clear, so we use '&' just to be safe 
	STK_LOAD = (systicknominal - 1) & 0x00ffffff;	// Start off with nominal duration

	/* Enable a lower priority interrupt for handling post-systick timing. */
	NVICIPR (NVIC_I2C2_EV_IRQ, NVIC_I2C2_EV_IRQ_PRIORITY );	// Set interrupt priority ('../lib/libusartstm32/nvicdirect.h')
	NVICISER(NVIC_I2C2_EV_IRQ);			// Enable interrupt controller ('../lib/libusartstm32/nvicdirect.h')

	/* Reset current value */
	STK_VAL = 0;

	*(u32*)0xE000ED20 |= (SYSTICK_PRIORITY_SE << 24);	// Set SYSTICK priority
//	SCB_SHPR3 |= (SYSTICK_PRIORITY_SE << 24);	// Set SYSTICK interrupt priority
	
	/* Clock runs at AHB bus speed, interrupts when reaching zero | interrupt enabled */
	STK_CTRL = (STK_CTRL_CLKSOURCE | STK_CTRL_TICKINT | STK_CTRL_ENABLE);

	return;
}
/*---------------------------------------------------------------------------------------------
 * static u8 bitcvt(u8 bit);
 * @brief	: convert a non-zero byte into a 1
 * @param	: input bit
 * @return	: bit
 ----------------------------------------------------------------------------------------------*/
static u8 bitcvt(u8 bit)
{
	if (bit != 0) return 1;
	return 0;
}
/******************************************************************************
 * int can_init_pod_varbuf_sys(struct CAN_PARAMS *p, u16 nummsgsbuffed);
 * @brief 	: Setup CAN pins and hardware
 * @param	: p = pointer to 'struct CAN_PARAMS' with setup values
 * @param	: nummsgsbuffed = Number of CAN msgs to be buffered
 * @return	:  n = remaining counts; 
 *		:  0 = enter init mode timedout;
 *		: -1 = exit init mode timeout; 
 *		: -2 = bad port
 *		: -3 = not enough sram for CAN xmit buffer (calloc failed)
 *		: -4 = nummsgbuffed = 0 (need at least one buffer!)
*******************************************************************************/
int can_init_pod_varbuf_sys(struct CAN_PARAMS *p, u16 nummsgsbuffed)
{
	int i;
	struct CAN_XMITBUFF* plst = (struct CAN_XMITBUFF*)&frii;	// Beginning of "free" list.
	struct CAN_XMITBUFF* ptmp;

	if (nummsgsbuffed == 0) return -4;	// At least one is required

	/* Get CAN xmit buffer and init plinknext. */	
	for (i = 0; i < nummsgsbuffed; i++)
	{
		ptmp = (struct CAN_XMITBUFF*)calloc(1, sizeof(struct CAN_XMITBUFF));
		if ( ptmp == NULL) return -3; // Get buff failed

		plst->plinknext = ptmp;
		plst = ptmp;
	}

	/* For emphasis purposes: calloc will initialize this to zero. */
	pend.can.id = 0x00000000;	// Highest possible CAN id priority

	// Continue initialization
	return can_init_pod_common_sys(p);
}
/******************************************************************************
 * int can_init_pod_sys(struct CAN_PARAMS *p);
 * @brief 	: Setup CAN pins and hardware  LEGACY CODE ENTRY
 * @param	: p = pointer to 'struct CAN_PARAMS' with setup values
 * @return	:  n = remaining counts; 
 *		:  0 = enter init mode timedout;
 *		: -1 = exit init mode timeout; 
 *		: -2 = bad port
 *		: -3 = not enough sram for CAN xmit buffer
*******************************************************************************/
int can_init_pod_sys(struct CAN_PARAMS *p)	// LEGACY CODE ENTRY
{
	return can_init_pod_varbuf_sys(p, CANXMTBUFNUM_DEFAULT);
}
/******************************************************************************
 * static int can_init_pod_common_sys(struct CAN_PARAMS *p);
 * @brief 	: Setup CAN pins and hardware
 * @param	: p = pointer to 'struct CAN_PARAMS' with setup values
 * @return	:  n = remaining counts; -9 = enter init mode timedout;-1 = exit init mode timeout; -2 = bad port
*******************************************************************************/
static int can_init_pod_common_sys(struct CAN_PARAMS *p)
{
	int can_timeout;	// Counter for breaking 'while' loops

	/* Enable clocking to CAN module */
	RCC_APB1ENR |= (1<<25);		// CAN1_EN p 144

	RCC_APB2ENR |= RCC_APB2ENR_AFIOEN;	// enable clock for Alternate Function

/* NOTE: The following statement is located in '../lib/libmiscstm32/clocksetup.c' & 'clockspecifysetup.c' */
//	RCC_APB2ENR |= 0x7c;	// Enable A thru Ebvc 

	 /* Note: calculations for PCLK1 = 36MHz */
//	unsigned int brp = (pclk1_freq / 18) / 500000;	// baudrate is set to 500k bit/s

	/* REMAP p 180
	Bits 14:13 CAN_REMAP[1:0]: CAN alternate function remappingport
            These bits are set and cleared by software. They control the mapping of alternate functions
            CAN_RX and CAN_TX in devices with a single CAN interface.
            00: CAN_RX mapped to PA11, CAN_TX mapped to PA12
            01: Not used
            10: CAN_RX mapped to PB8, CAN_TX mapped to PB9 (not available on 36-pin package)
            11: CAN_RX mapped to PD0, CAN_TX mapped to PD1 
	OLIMEX = 0x2
	*/

/* Output pins configure for alternate function output push-pull */
const struct PINCONFIGALL pa12 = {(volatile u32 *)GPIOA, 12, OUT_AF_PP, MHZ_50};
const struct PINCONFIGALL pb9  = {(volatile u32 *)GPIOB,  9, OUT_AF_PP, MHZ_50};
const struct PINCONFIGALL pd1  = {(volatile u32 *)GPIOD,  1, OUT_AF_PP, MHZ_50};

/* Input pins configure for input pull-up */
const struct PINCONFIGALL pa11 = {(volatile u32 *)GPIOA, 11, IN_PU, 0};
const struct PINCONFIGALL pb8  = {(volatile u32 *)GPIOB,  8, IN_PU, 0};
const struct PINCONFIGALL pd0  = {(volatile u32 *)GPIOD,  0, IN_PU, 0};


	/* Setup remapping and configure port pins */
	switch (p->port)
	{
	case 0:	// CAN on port A

		/*  Setup CAN TXD: PA12 for alternate function push/pull output p 156, p 163  */
		pinconfig_all((struct PINCONFIGALL *)&pa12);

		/* Setup CAN RXD: PB11 for input pull up p 164, 167 */
		pinconfig_all((struct PINCONFIGALL *)&pa11);

		AFIO_MAPR |= (0x0 << 13);	 // 00: CAN_RX mapped to PA11, CAN_TX mapped to PA12 p 179
		break;

	case 2:	// CAN on port B

		/*  Setup CAN TXD: PB9 for alternate function push/pull output p 156, p 163  */
		pinconfig_all((struct PINCONFIGALL *)&pb9);

		/* Setup CAN RXD: PB8 for input pull up p 164, 167 */
		pinconfig_all((struct PINCONFIGALL *)&pb8);

		AFIO_MAPR |= (0x2 << 13);	 // 00: CAN_RX mapped to PA11, CAN_TX mapped to PA12 p 179
		break;

	case 3:	// CAN on port D

		/*  Setup CAN TXD: PD1 for alternate function push/pull output p 156, p 163  */
		pinconfig_all((struct PINCONFIGALL *)&pd1);

		/* Setup CAN RXD: PD0 for input pull up p 164, 167 */
		pinconfig_all((struct PINCONFIGALL *)&pd0);

		AFIO_MAPR |= (0x3 << 13);	 // 00: CAN_RX mapped to PA11, CAN_TX mapped to PA12 p 179
		break;
	default:
		return -2;
		break;
	}

	/* ---------- Set Initialization mode -------------------- */
	/* Request initialization p 632, 648.  DEBUG freeze bit on */
	CAN_MCR(CAN1) &= CAN_MCR_DBF;	// Clear DBF since it is set coming out RESET
	CAN_MCR(CAN1) = ((bitcvt(p->dbf) << 16) | (CAN_MCR_INRQ) );

	/* The initialization request (above) causes the INAK bit to be set.  This bit goes off when 11 consecutive
	   recessive bits have been received p 633 */
	can_timeout = CAN_TIMEOUT;	// Counter to break loop for timeout
	while ( ((CAN_MSR(CAN1) & CAN_MSR_INAK) == 0 ) && (can_timeout-- >= 0) );	// Wait until initialization mode starts or times out
	if (can_timeout <= 0 ) return -9;	// Timed out

	/* Compute Baud Rate Prescalar (+1). */
	u32 brp = (pclk1_freq / (1 + p->tbs1 + p->tbs2) ) / p->baudrate;

	/* Setup Bit Timing Register. */
	CAN_BTR(CAN1)  =  (bitcvt(p->silm)        << 31);	// Silent mode bit
	CAN_BTR(CAN1) |=  (bitcvt(p->lbkm)        << 30);	// Loopback mode bit
	CAN_BTR(CAN1) |=  (((p->sjw  - 1) & 0x03) << 24);	// Resynchronization jump width
	CAN_BTR(CAN1) |=  (((p->tbs2 - 1) & 0x07) << 20);	// Time segment 2
	CAN_BTR(CAN1) |=  (((p->tbs1 - 1) & 0x0F) << 16);	// Time segment 1
	CAN_BTR(CAN1) |=  (((brp-1)      & 0x3FF) <<  0);	// Baud rate prescalar

	/* p 649 Option bits in CAN_MCR
Bit 7 TTCM: Time triggered communication mode
        0: Time Triggered Communication mode disabled.
        1: Time Triggered Communication mode enabled
      Note: For more information on Time Triggered Communication mode, please refer to
              Section 24.7.2: Time triggered communication mode.
Bit 6 ABOM: Automatic bus-off management
        This bit controls the behavior of the CAN hardware on leaving the Bus-Off state.
        0: The Bus-Off state is left on software request, once 128 occurrences of 11 recessive bits
        have been monitored and the software has first set and cleared the INRQ bit of the
        CAN_MCR register.
        1: The Bus-Off state is left automatically by hardware once 128 occurrences of 11 recessive
        bits have been monitored.
        For detailed information on the Bus-Off state please refer to Section 24.7.6: Error
        management.
Bit 5 AWUM: Automatic wakeup mode
        This bit controls the behavior of the CAN hardware on message reception during Sleep
        mode.
        0: The Sleep mode is left on software request by clearing the SLEEP bit of the CAN_MCR
        register.
        1: The Sleep mode is left automatically by hardware on CAN message detection.
        The SLEEP bit of the CAN_MCR register and the SLAK bit of the CAN_MSR register are
        cleared by hardware.
Bit 4 NART: No automatic retransmission
        0: The CAN hardware will automatically retransmit the message until it has been
        successfully transmitted according to the CAN standard.
        1: A message will be transmitted only once, independently of the transmission result
        (successful, error or arbitration lost).
 */
	CAN_MCR(CAN1) &= ~(0xfe);			// Clear bits except for INAQ
	CAN_MCR(CAN1) |=  (bitcvt(p->ttcm) << 7);	// Time triggered communication mode
	CAN_MCR(CAN1) |=  (bitcvt(p->abom) << 6);	// Automatic bus-off management
	CAN_MCR(CAN1) |=  (bitcvt(p->awum) << 5);	// Auto WakeUp Mode

	/* Override the 'nart' bit in the struct.  It gets handled in software. */
//	CAN_MCR(CAN1) |=  (bitcvt(p->nart) << 4);	// No Automatic ReTry (0 = retry; non-zero = xmit only once)
	defaultnart = (bitcvt(p->nart));		// Save for later default calls
	CAN_MCR(CAN1) |=  (1 << 4);			// No Automatic ReTry (0 = retry; non-zero = xmit only once)

	/* Leave initialization mode */
	CAN_MCR(CAN1) &= ~CAN_MCR_INRQ;
	can_timeout = CAN_TIMEOUT;	// Counter to break loop for timeout
	while ( ((CAN_MSR(CAN1) & CAN_MSR_INAK) != 0 ) && (can_timeout-- >= 0) );	// Wait until initialization mode starts or times out
	if (can_timeout <= 0 ) return -1;	// Timed out

/* CYCCNT counter is in the Cortex-M-series core.  See the following for details 
http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0337g/BABJFFGJ.html */
	*(volatile unsigned int*)0xE000EDFC |= 0x01000000; // SCB_DEMCR = 0x01000000;
	*(volatile unsigned int*)0xE0001000 |= 0x1;	// Enable DTW_CYCCNT (Data Watch cycle counter)

	/* Set and enable interrupt controller for doing software interrupt */
	// Handles high priority CAN msgs, e.g. RESETALL, TIMESYNCx
	NVICIPR (NVIC_I2C1_EV_IRQ, NVIC_I2C1_EV_IRQ_PRIORITY );	// Set interrupt priority ('../lib/libusartstm32/nvicdirect.h')
	NVICISER(NVIC_I2C1_EV_IRQ);			// Enable interrupt controller ('../lib/libusartstm32/nvicdirect.h')

	// Handles non-high priority msgs, e.g. sensor readings
	NVICIPR (NVIC_I2C1_ER_IRQ, NVIC_I2C1_ER_IRQ_PRIORITY );	// Set interrupt priority ('../lib/libusartstm32/nvicdirect.h')
	NVICISER(NVIC_I2C1_ER_IRQ);			// Enable interrupt controller ('../lib/libusartstm32/nvicdirect.h')

	// Handles non-high priority msgs, e.g. sensor readings
	NVICIPR (NVIC_I2C2_ER_IRQ, NVIC_I2C2_ER_IRQ_PRIORITY );	// Set interrupt priority ('../lib/libusartstm32/nvicdirect.h')
	NVICISER(NVIC_I2C2_ER_IRQ);			// Enable interrupt controller ('../lib/libusartstm32/nvicdirect.h')

	/* Set and enable interrupt controller for CAN interrupts */
	NVICIPR (NVIC_CAN_RX1_IRQ, NVIC_CAN_RX1_IRQ_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_CAN_RX1_IRQ);				// Enable interrupt controller

	NVICIPR (NVIC_CAN_SCE_IRQ, NVIC_CAN_SCE_IRQ_PRIORITY );// Set interrupt priority
	NVICISER(NVIC_CAN_SCE_IRQ);				// Enable interrupt controller

	NVICIPR (NVIC_USB_HP_CAN_TX_IRQ, NVIC_USB_HP_CAN_TX_IRQ_PRIORITY );// Set interrupt priority
	NVICISER(NVIC_USB_HP_CAN_TX_IRQ);				// Enable interrupt controller

	NVICIPR (NVIC_USB_LP_CAN_RX0_IRQ, NVIC_USB_LP_CAN_RX0_IRQ_PRIORITY );// Set interrupt priority
	NVICISER(NVIC_USB_LP_CAN_RX0_IRQ);				// Enable interrupt controller

	/* CAN interrupt enable register (CAN_IER) */
	/* Bit 1 FMPIE0: FIFO message pending interrupt enable
	       0: No interrupt generated when state of FMP[1:0] bits are not 00b.
	       1: Interrupt generated when state of FMP[1:0] bits are not 00b.
	   Bit 4 FMPIE1: FIFO message pending interrupt enable
	       0: No interrupt generated when state of FMP[1:0] bits are not 00b.
	       1: Interrupt generated when state of FMP[1:0] bits are not 00b. */

	CAN_IER(CAN1) |= CAN_IER_FMPIE0;	// FIFO 0 p 655
	CAN_IER(CAN1) |= CAN_IER_FMPIE1;	// FIFO 1 p 655

	/* Wait until ready */
	while ( (CAN_TSR(CAN1) & CAN_TSR_TME0) == 0)         // Wait for transmit mailbox 0 to be empty
		CAN_TSR(CAN1)  = 0X1;
	CAN_IER(CAN1) |= 0x1;	// Bit0: 1: Interrupt generated when RQCPx bit is set. p 656.

	/* Get systick for timing setup */	
	SYSTICK_init ();

	return (CAN_TIMEOUT - can_timeout);	// Return wait loop count
}
/*******************************************************************************
 * void can_nxp_setRS_sys(int rs, int board);
 * @brief 	: Set RS input to NXP CAN driver (TJA1051) (on some PODs) (SYSTICK version)
 * @param	: rs: 0 = NORMAL mode; not-zero = SILENT mode 
 * @param	: board: 0 = POD, 1 = sensor RxT6 board
 * @return	: Nothing for now.
*******************************************************************************/
void can_nxp_setRS_sys(int rs, int board)
{
	/* RS (S) control PB7 (on sensor board) PD11 on pod board */
	// Floating input = resistor controls slope
	// Pin HI = standby;
	// Pin LO = high speed;
	if (board == 0)
	{
		configure_pin ((volatile u32 *)GPIOD, 11);	// configured for push-pull output
		if (rs == 0)
			GPIO_BRR(GPIOD)  = (1<<11);	// Set bit LO for SILENT mode
		else
			GPIO_BSRR(GPIOD) = (1<<11);	// Set bit HI for NORMAL mode
	}
	else
	{
		configure_pin ((volatile u32 *)GPIOB,  7);	// configured for push-pull output	
		if (rs == 0)
			GPIO_BRR(GPIOB)  = (1<< 7);	// Set bit LO for SILENT mode
		else
			GPIO_BSRR(GPIOB) = (1<< 7);	// Set bit HI for NORMAL mode
	}
	return;
}
/*******************************************************************************
 * void can_filter_unitid_sys(u32 myunitid);
 * @brief 	: Setup filter bank for Checksum check & Program loading
 * @param	: "my" can id
 * @return	: Nothing for now.
*******************************************************************************/
static u32 myunitid_local;
void can_filter_unitid_sys(u32 myunitid)
{
	myunitid_local = myunitid;	// Save unit id number locally (for RESET msg checking)

/* p 632
      To initialize the registers associated with the CAN filter banks (mode, scale, FIFO
      assignment, activation and filter values), software has to set the FINIT bit (CAN_FMR). Filter
      initialization also can be done outside the initialization mode.
Note: When FINIT=1, CAN reception is deactivated.
      The filter values also can be modified by deactivating the associated filter activation bits (in
      the CAN_FA1R register).
      If a filter bank is not used, it is recommended to leave it non active (leave the corresponding
      FACT bit cleared).
*/
	/* CAN filter master register p 665 */
	CAN_FMR(CAN1)  |= CAN_FMR_FINIT;	// FINIT = 1; Initialization mode ON for setting up filter banks

	/* CAN filter scale register p 639 p 666.  Default is dual 16 bit mode. */
	CAN_FS1R(CAN1) |= 0x4; // Bank 2, in 32b mode


	/* CAN filter mode register  p 639 666 */
	// CAN_FM1R(CAN1) |= 0x0;	// default 32b mask mode

	/* This register can only be modified when the FACTx bit of the CAN_FAxR register is cleared
	   or when the FINIT bit of the CAN_FMR register is set. */

	/*   Set 16 bit filter mask and ID p 640 fig 229 */
	// Note: '>>16' is because for 16b scale mode the unit id goes in the low order word.
	// (CANx, <bank>) =  <[15:8] mask>      <[7:0] id>
	CAN_FiR1(CAN1, 0) = CAN_UNITID_MASK | (CAN_RESETALL  >> 16);	// All units accept this msg
	CAN_FiR2(CAN1, 0) = CAN_UNITID_MASK | (CAN_TIMESYNC1 >> 16);	// All units accept this msg
	CAN_FiR1(CAN1, 1) = CAN_UNITID_MASK | (CAN_TIMESYNC2 >> 16);	// All units accept this msg
	CAN_FiR2(CAN1, 1) = CAN_UNITID_MASK | (CAN_TIMESYNC3 >> 16);	// All units accept this msg
	CAN_FiR1(CAN1, 2) = myunitid;					// 'IAMUNITNUMBER' is unique to this unit
	CAN_FiR2(CAN1, 2) = CAN_UNITID_MASK ;				// Pass all 11 bit ID's of IAMUNITNUMBER
	can_filt16num = 6;	// Show that filter bank 2, FiR2 is available

/* ==> NOTES <==
For the situation--Using the default scale: each 32b register of the filter bank *pair* holds a 16b mask and 16b id
1) The filter bank registers are NOT initialized by reset, i.e. they hold random data out of reset.
2) Both (32b) registers of the pair must be setup, otherwise the register that is not set will be used in the filtering,
   and this can cause strange results since the mask|id might pass unwanted msgs.
3) Setting a register to zero passes *all* messages, (since the mask is 100% don't cares).
*/
	/* Assign filter to FIFO 1. Default: All messages pass through FIFO p 667 */
	CAN_FFA1R(CAN1) |= 0x3;	// Filter banks 0 and 1 pass through FIFO 1

	/* Filter activation */
	/* Bits 27:0 FACTx: Filter active
            The software sets this bit to activate Filter x. To modify the Filter x registers (CAN_FxR[0:7]),
            the FACTx bit must be cleared or the FINIT bit of the CAN_FMR register must be set.
            0: Filter x is not active
            1: Filter x is active */
	CAN_FA1R(CAN1) |= 0x7;	// Activate filter banks 0, 1, 2. p 667

	/* Remove filter registers from initialization mode */
	CAN_FMR(CAN1)  &= ~CAN_FMR_FINIT;	// FINIT = 0; Set initialization mode off for filter banks p 665

	return;
}
/*******************************************************************************
 * u16 can_filtermask16_add_sys(u32 m);
 * @brief 	: Add a 16 bit id & mask to the filter banks
 * @param	: mask and ID: e.g. (CAN_UNITID_MASK | (CAN_TIMESYNC3 >> 16)
 * @return	: Filter number; negative for you have filled it up!
 NOTE: These mask|ID's assign to the FIFO 0 (default)
*******************************************************************************/
u16 can_filtermask16_add_sys(u32 m)
{
	/* F103 has 14 banks that can hold two 16 bit mask|ID's */
	if (can_filt16num >= (14 * 2)) return -1;

	/* CAN filter master register p 665 */
	CAN_FMR(CAN1)  |= CAN_FMR_FINIT;	// FINIT = 1; Initialization mode ON for setting up filter banks

	if ((can_filt16num & 0x1) != 0)	// Does the last filter bank assigned have an unused slot?
	{ // Here, odd, therefore we can add this to the currently assigned filter bank
		CAN_FiR2(CAN1,(can_filt16num >> 1)) = m;	// Fill the 2nd slot
	}
	else
	{ /* Here, the count is even, so a new filter bank must be used */
		CAN_FiR1(CAN1,(can_filt16num >> 1)) = m;	// Fill the 1st slot
		CAN_FiR2(CAN1,(can_filt16num >> 1)) = CAN_NEVERUSEID;	// Initialize the odd, 2nd 16b mask|id 

		/* Activate the newly assigned filter bank */
		CAN_FA1R(CAN1) |= (1 << (can_filt16num >> 1));
	}

	/* Update the count of mask|ID's we have set up */
	can_filt16num += 1;	// Show another was added

	/* Remove filter registers from initialization mode */
	CAN_FMR(CAN1)  &= ~CAN_FMR_FINIT;	// FINIT = 0; Set initialization mode off for filter banks p 665
	
	return 0;
}
/******************************************************************************
 * struct CANRCVBUF* canrcv_get(void);
 * @brief	: Get pointer to non-high priority CAN msg buffer
 * @return	: struct with pointer to buffer, ptr = zero if no new data
 ******************************************************************************/
struct CANRCVBUF* canrcv_get_sys(void)
{
	struct CANRCVBUF *p;
	if (canbufIDXi == canbufIDXm) return 0;	// Return showing no new data

	p = &canbuf[canbufIDXm];	// Get return pointer value

	canbufIDXm = adv_index(canbufIDXm, CANRCVBUFSIZE);
	return p;			// Return pointer to buffer
}
/******************************************************************************
 * struct CANRCVTIMBUF * canrcvtim_get(void);
 * @brief	: Get pointer to high priority CAN msg buffer
 * @return	: struct with pointer to buffer, ptr = zero if no new data
 ******************************************************************************/
struct CANRCVTIMBUF* canrcvtim_get_sys(void)
{
	struct CANRCVTIMBUF *p;
	if (canbuftimIDXi == canbuftimIDXm) return 0;	// Return showing no new data

	p = &cantimbuf[canbuftimIDXm];	// Get return pointer value	

	/* Advance index ('m' = mainline) */
	canbuftimIDXm = adv_index(canbuftimIDXm, CANRCVTIMBUFSIZE);
	return p;			// Return pointer to buffer
}
/******************************************************************************
 * int can_msg_txchkbuff_sys(void);
 * @brief	: Check buffer status for xmit buffer
 * @return	: 0 = buffer full; not zero = remaining slots
 ******************************************************************************/
int can_msg_txchkbuff_sys(void)
{
	return	(int)pend.plinknext;
}
/******************************************************************************
 * int can_msg_busy_sys(void);
 * @brief	: Check of CAN msg TX buffer has any msgs pending
 * @return	: 0 = no msgs in pending list; not 0 = msgs
 ******************************************************************************/
int can_msg_busy_sys(void)
{
	return (pxprv != NULL);
}
/******************************************************************************
 * void can_msg_rcv_compress_sys(struct CANRCVBUF *px);
 * @brief	: Set .dlc with count to eliminate high order bytes equal zero
 * @param	: pointer to struct with msg: id, dlc & data
 * @return	: nothing
 ******************************************************************************/
void can_msg_rcv_compress_sys(struct CANRCVBUF *px)
{
	int temp;

	/* Set length to send only the signficant bytes (DLC p 660) */
	px->dlc &= ~0xf;	// Clear DLC field
	
	/* Find most significant byte */
	for (temp = 7; temp >= 0; temp--)
	{
		if (px->cd.uc[temp] != 0) break;
	}
	if ( temp >= 0)
	{ // Here, temp = 0->7 , so send 1 -> 8 bytes
		px->dlc |= ((temp+1) & 0xf);   // Number of bytes
	}
	else
	{ // All bytes zero.  Send as RTR
		px->id |= 0x2; // RTR
	}
	return;
}
/******************************************************************************
 * void can_msg_rcv_expand_sys(struct CANRCVBUF *px);
 * @brief	: Fill bytes not received with zeros
 * @param	: pointer to struct with msg: id, dlc & data
 * @return	: nothing
 ******************************************************************************/
void can_msg_rcv_expand_sys(struct CANRCVBUF *px)
{
	int i;
	int temp = (px->dlc & 0xf);	// Get byte count in received msg
	if (temp > 8) temp = 8;		// JIC the number exceeds the array
	if (temp <= 0) { px->cd.ull = 0; return; }
	
	/* Clear bytes not received. */
	for (i = 7; i >= temp; i--)	// Work from hi-ord byte down
		px->cd.uc[i] = 0;	// Set unsent byte to zero

	return;
}
/******************************************************************************
 * void can_msg_setsize_sys(struct CANRCVBUF *px, int count);
 * @brief	: Set .dlc with byte count to send
 * @param	: pointer to struct with msg: id, dlc & data
 * @return	: nothing
 ******************************************************************************/
void can_msg_setsize_sys(struct CANRCVBUF *px, int count)
{
	px->dlc &= ~0xf;	// Clear dlc field
	px->dlc |= count & 0xf;	// Add new count
	return;
}
/*######################### WARNING UNDER INTERRUPT #####################################
 * int canmsg_send(struct CANRCVBUF * p, int data1, int data2);
 * @param	: p = pointer to message to send
 * @param	: data1 = 1st 4 bytes of payload
 * @param	: data2 = 2nd 4 bytes of payload
 * @brief 	: send CAN msg
 * @return	:  0 = OK; 
 *		: -1 = Buffer overrun (no free slots for the new msg)
 *####################################################################################### */
int canmsg_send(struct CANRCVBUF * p, int data1, int data2)
{
	p->cd.ui[0] = data1;
	p->cd.ui[1] = data2;

	can_msg_setsize_sys(p, 8);	// Set byte count: Fixed xmt

	return can_msg_put_sys(p);
}
/******************************************************************************
 * int canmsg_send_sys_n(struct CANRCVBUF * p, u8* pc, s32 n);
 * @param	: p = pointer to message to send
 * @param	: pc = pointer to bytes to send
 * @param	: n = number of bytes to send
 * @brief 	: send CAN msg that is less than 8 bytes
 * @return	:  0 = OK; 
 *		: -1 = Buffer overrun (no free slots for the new msg)
 ******************************************************************************/
int canmsg_send_sys_n(struct CANRCVBUF * p, u8* pin, s32 n)
{
	int i;
	
	/* Copy input bytes into payload */
	u8* pout = &p->cd.uc[0];	// Pointer into CAN msg payload field
	if (n > 8) n = 8; if (n < 0) n = 0; // Make sure 'n' is bounded.
	for (i = 0; i < n; i++) *pout++ = *pin++;

	can_msg_setsize_sys(p, n);	// Set byte count: Fixed xmt

	return can_msg_put_sys(p);
}
/******************************************************************************
 * int can_msg_put_sys(struct CANRCVBUF *pcan);
 * @brief	: Get a free slot and add msg (LEGACY CODE ENTRY uses defaults)
 * @param	: pointer to struct with msg: id, dlc, data
 * @return	: Number msgs added minus number removed
 ******************************************************************************/
int can_msg_put_sys(struct CANRCVBUF *pcan)	// FOR LEGACY CODE ENTRY
{
	/* Use default TERR retry count, and set soft-NART bit one in struct CAN_PARAMS */
	return can_msg_put_ext_sys(pcan, TERRMAXCOUNT, defaultnart);
}
/******************************************************************************
 * int can_msg_put_ext_sys(struct CANRCVBUF *pcan, u8 maxretryct, u8 bits);
 * @brief	: Get a free slot and add msg ('ext' = extended version)
 * @param	: pointer to struct with msg: id, dlc, data (common_can.h)
 * @param	: maxretryct =  0 = use TERRMAXCOUNT; not zero = use this value.
 * @param	: bits = Use these bits to set some conditions (see .h file)
 * @return	:  0 = OK; 
 *		: -1 = Buffer overrun (no free slots for the new msg)
 ******************************************************************************/
int can_msg_put_ext_sys(struct CANRCVBUF *pcan, u8 maxretryct, u8 bits)
{

	
	struct CAN_XMITBUFF* pnew;

	/* Get a free block from the free list. */
	DISABLE_TXINT;	// TX interrupt might move a msg to the free list.
	pnew = frii.plinknext;
	if (pnew == NULL) 
	{
		ENABLE_TXINT;
		can_errors.can_msgovrflow += 1;	// Count overflows
		return -1;	// Return failure: no space & screwed
	}	
	frii.plinknext = pnew->plinknext;
	ENABLE_TXINT;
	/* 'pnew' now points to the block that is free (and not linked). */

	/* Build struct/block for addition to the pending list. */
	pnew->can = *pcan;	// Copy CAN msg.
	pnew->maxretryct = maxretryct;	// Maximum number of TERR retry counts
	pnew->bits	= bits;	// Use these bits to set some conditions (see .h file)
	pnew->retryct	= 0;	// Retry counter for TERRs.

	/* Find location to insert new msg.  Lower value CAN ids are higher priority, 
           and when the CAN id msg to be inserted has the same CAN id as the 'pfor' one
           already in the list, then place the new one further down so that msgs with 
           the same CAN id do not get their order of transmission altered. */
 	DISABLE_TXINT;
	for (pfor = &pend; pfor->plinknext != NULL; pfor = pfor->plinknext)
	{
		if (pnew->can.id < (pfor->plinknext)->can.id) // Pay attention: "value" vs "priority"
			break;
		/* Each loop of the search allow a TX interrupt to send the next msg. */
		ENABLE_TXINT;	// Open window to allow a TX interrupt
		DISABLE_TXINT;	// Close window for TX interrupts
	}

	/* Add new msg to pending list. (TX interrupt is still disabled) */
	pnew->plinknext = pfor->plinknext; 	// Insert new msg into 
	pfor->plinknext = pnew;			//   pending list.
	pfor =  NULL;	// Signal TX interrupt that search loop is not active.
	if (pxprv == NULL) // Is sending complete?
	{ // pxprv == NULL means CAN mailbox did not get loaded, so CAN is idle.
		loadmbx(); // Start sending
	}
	else
	{ // CAN sending is in progress.
		if (pxprv->plinknext == pnew) // Does pxprv need adjustment?
		{ // Here yes. We inserted a msg between 'pxprv' and 'pxprv->linknext'
			pxprv = pnew;	// Update 'pxprv' so that it still points to msg TX using.
			can_errors.can_pxprv_fwd_one += 1;	// Count: Instances that pxprv was adjusted in 'for' loop
		}
	}
	ENABLE_TXINT;

	return 0;	// Success!
}
/*#######################################################################################
 * ISR routines for CAN:There four interrupt vectors, p 647 fig 235
 *####################################################################################### */
/*---------------------------------------------------------------------------------------------
 * static void loadmbx(void)
 * @brief	: Load mailbox
 ----------------------------------------------------------------------------------------------*/
static void loadmbx()
{
	if (pend.plinknext == NULL)
	{
		pxprv = NULL;
		return; // Return if no more to send
	}

	pxprv = &pend;	// Save in a static var

	/* Load the mailbox with the message.  CAN ID low bit starts xmission. */
	CAN_TDTxR(CAN1, CAN_MBOX0) =  (pend.plinknext)->can.dlc;	 // CAN_TDT0R:  mailbox 0 time & length p 660
	CAN_TDLxR(CAN1, CAN_MBOX0) =  (pend.plinknext)->can.cd.ui[0];	 // CAN_TDL0RL: mailbox 0 data low  register p 661 
	CAN_TDHxR(CAN1, CAN_MBOX0) =  (pend.plinknext)->can.cd.ui[1];	 // CAN_TDL0RH: mailbox 0 data low  register p 661 
	/* Load CAN ID and set TX Request bit */
	CAN_TIxR (CAN1, CAN_MBOX0) = ((pend.plinknext)->can.id | 0x1); // CAN_TI0R:   mailbox 0 identifier register p 659
	return;
}
/*#######################################################################################
 * ISR routine for CAN: "Transmit Interrupt" <-- (RQCP0 | RQCP1 | RQCP2)
 *####################################################################################### */
/* p 647 The transmit interrupt can be generated by the following events:
–   Transmit mailbox 0 becomes empty, RQCP0 bit in the CAN_TSR register set.
–   Transmit mailbox 1 becomes empty, RQCP1 bit in the CAN_TSR register set.
–   Transmit mailbox 2 becomes empty, RQCP2 bit in the CAN_TSR register set. */

/*
(CAN_TSR(CAN1) & 0xf)
Bit 3 TERR0: Transmission error of mailbox0
  This bit is set when the previous TX failed due to an error.
Bit 2 ALST0: Arbitration lost for mailbox0
  This bit is set when the previous TX failed due to an arbitration lost.
Bit 1 TXOK0: Transmission OK of mailbox0
  The hardware updates this bit after each transmission attempt.
   0: The previous transmission failed
   1: The previous transmission was successful
  This bit is set by hardware when the transmission request on mailbox 1 has been completed
  successfully. Please refer to Figure 227
Bit 0 RQCP0: Request completed mailbox0
  Set by hardware when the last request (transmit or abort) has been performed.
  Cleared by software writing a “1” or by hardware on transmission request (TXRQ0 set in
  CAN_TI0R register).
  Clearing this bit clears all the status bits (TXOK0, ALST0 and TERR0) for Mailbox 0.

*/
static void moveremove(void);

void USB_HP_CAN_TX_IRQHandler(void)
{
	__attribute__((__unused__))int temp1;	// Dummy for readback of hardware registers

	/* JIC: mailboxes 1 & 2 are not used and should not have a flag */
	if ((CAN_TSR(CAN1) & (CAN_TSR_RQCP1 | CAN_TSR_RQCP2)) != 0)
	{ // Here, something bogus going on.
		CAN_TSR(CAN1) = (CAN_TSR_RQCP1 | CAN_TSR_RQCP2);	// Turn flags OFF
		can_errors.can_cp1cp2 += 1;	// Count: (RQCP1 | RQCP2) unexpectedly ON
		temp1 = CAN_TSR(CAN1);	// JIC Prevent tail-chaining
		return;
	}

	u32 tsr = CAN_TSR(CAN1);	// Copy of status register mailbox 0 bits

	/* Do this early so it percolates down into the hardware. */
	CAN_TSR(CAN1) = CAN_TSR_RQCP0;	// Clear mailbox 0: RQCP0 (which clears TERR0, ALST0, TXOK0)

	/* Check for a bogus interrupt. */
	if ( (tsr & CAN_TSR_RQCP0) == 0) // Is mailbox0 RQCP0 (request complete) ON?
	{ // Here, no RXCPx bits are on, so interrupt is bogus.
		temp1 = CAN_TSR(CAN1);	// JIC Prevent tail-chaining
		return;
	}

	if ((tsr & CAN_TSR_TXOK0) != 0) // TXOK0: Transmission OK for mailbox 0?
	{ // Here, yes. Flag msg for removal from pending list.
		moveremove();	// remove from pending list, add to free list
	}
	else if ((tsr & CAN_TSR_TERR0) != 0) // Transmission error for mailbox 0? 
	{ // Here, TERR error bit, so try it some more.
		can_errors.can_txerr += 1; 	// Count total CAN errors
		(pxprv->plinknext)->retryct += 1;	// Count errors for this msg
		if ((pxprv->plinknext)->retryct > (pxprv->plinknext)->maxretryct)
		{ // Here, too many error, remove from list
			can_errors.can_tx_bombed += 1;	// Number of bombouts
			moveremove();	// Remove msg from pending queue
		}
	}
	else if ((tsr & CAN_TSR_ALST0) != 0) // Arbitration lost for mailbox 0?
	{ // Here, arbitration for mailbox 0 failed.
		can_errors.can_tx_alst0_err += 1; // Running ct of arb lost: Mostly for debugging/monitoring
		if (((pxprv->plinknext)->bits & SOFTNART) != 0)
		{ // Here this msg was not to be re-sent, i.e. NART
			can_errors.can_tx_alst0_nart_err += 1; // Mostly for debugging/monitoring
			moveremove();	// Remove msg from pending queue
		}
	}
	else
	{ // Here, no bits on, therefore something bogus.
		can_errors.can_no_flagged += 1; // Count for monitoring purposes
	}

	loadmbx();		// Load mailbox 0.  Mailbox should be available/empty.
	temp1 = CAN_TSR(CAN1);	// JIC Prevent tail-chaining
	return;
}
/* --------------------------------------------------------------------------------------
static * void moveremove(void);
* @brief	: Remove msg from pending list and add to free list
  --------------------------------------------------------------------------------------- */
static void moveremove(void)
{
	struct CAN_XMITBUFF* pmov;

	/* JIC we got a TX completion when we did not load the mailbox, e.g. initialization. */
	if (pend.plinknext == NULL) return;

	/* Remove from pending; move to free list. */
	if ((pfor != NULL) && (pfor == pxprv->plinknext)) // Does pfor need adjustment?
	{ // Here, yes. 'pfor' points to the msg removed in the following statements.
		pfor = pxprv; // Move 'pfor' back "up" one msg in linked list
		can_errors.can_pfor_bk_one += 1; // Keep track of instances
	}

	// Remove from pending list
	pmov = pxprv->plinknext;	// Pts to removed item
	pxprv->plinknext = pmov->plinknext;

	// Adding to free list
	pmov->plinknext = frii.plinknext; 
	frii.plinknext  = pmov;
	return;
}
/*#######################################################################################
 * ISR routine for CAN: "FIFO 0 interrupt" -- FMP0, FULL0, FOVR0
 *####################################################################################### */
/* p 647 The FIFO 0 interrupt can be generated by the following three events (only X = enabled):
X   Reception of a new message, FMP0 bits in the CAN_RF0R register are not ‘00’.
–   FIFO0 full condition, FULL0 bit in the CAN_RF0R register set.
–   FIFO0 overrun condition, FOVR0 bit in the CAN_RF0R register set. */
u32 can_rx0err = 0;	// Count of FIFO 0 overrun counter

void USB_LP_CAN_RX0_IRQHandler(void)
{
	__attribute__((__unused__))int temp2;
//while(1==1);
	/* Save message in a circular buffer */
	canbuf[canbufIDXi].id     = CAN_RI0R(CAN1);	// ID, RTR, IDE
	canbuf[canbufIDXi].dlc	  = CAN_RDT0R(CAN1);	// time, data length CAN_RDTxR p 663
	canbuf[canbufIDXi].cd.ui[0] = CAN_RDL0R(CAN1);	// Data (32b) High, Data (32b) Low
	canbuf[canbufIDXi].cd.ui[1] = CAN_RDH0R(CAN1);	// Data (32b) High, Data (32b) Low

	/* Count errors. */
	if ( (CAN_RF0R(CAN1) & (1 << 4)) != 0 ) can_rx0err += 1;


	/* Release FIFO 0 */	
	CAN_RF0R(CAN1) |= CAN_RF0R_RFOM0;	// Write bit 5

//	if ( (canbuf[canbufIDXi].id & ~1) == ( myunitid_local | (CAN_EXTRID_DATA_CMD << CAN_EXTRID_SHIFT)) )
	if ( (canbuf[canbufIDXi].id & ~1) == myunitid_local )
	{ // Here, a loader command (data type) for our unitid.  Is this a forced reset to this unit only?
		if ( (canbuf[canbufIDXi].dlc == 1) && (canbuf[canbufIDXi].cd.u8[0] == LDR_RESET) )
			SCB_AIRCR = (0x5FA << 16) | SCB_AIRCR_SYSRESETREQ;	// Cause a RESET
	}

//	if ( (canbuf[canbufIDXi].id & ~0x7) == ( CAN_RESETALL | CAN_DATAID_MASTERRESET ) )
//	{
//		SCB_AIRCR = (0x5FA << 16) | SCB_AIRCR_SYSRESETREQ;	// Cause a RESET
//		while(1==1);
//	}

	/* Advance index in circular buffer */
	canbufIDXi = adv_index(canbufIDXi, CANRCVBUFSIZE);

	/* Trigger a pending interrupt, which will cause a chain of related routines to execute */
	NVICISPR(NVIC_I2C1_ER_IRQ);	// Set pending (low priority) interrupt ('../lib/libusartstm32/nvicdirect.h')

	temp2 = CAN_RF0R(CAN1);	// Read register to avoid tail-chaining

	return;
}
/*#######################################################################################
 * ISR routine for CAN: "FIFO 1 interrupt" -- FMP1, FULL1, FOVR1
 *####################################################################################### */
/* p 647,8 The FIFO 1 interrupt can be generated by the following three events (only X = enabled):
X   Reception of a new message, FMP1 bits in the CAN_RF1R register are not ‘00’.
–   FIFO1 full condition, FULL1 bit in the CAN_RF1R register set.
–   FIFO1 overrun condition, FOVR1 bit in the CAN_RF1R register set. */
u32 can_rx1err = 0;	// Count of FIFO 1 overrun counter

void CAN_RX1_Handler(void)
{ // Here, FMP1 bits are not 00.
	__attribute__((__unused__))int temp3;
//while(1==1);
//	fifo1cycnt = *(volatile unsigned int *)0xE0001004; // DWT_CYCNT
	fifo1cycnt = DTWTIME; // DWT_CYCNT
	fifo1msgflg += 1;

	/* Save time & message in a circular buffer */
	cantimbuf[canbuftimIDXi].U.ull    = fifo1cycnt;// Get current tick time
	cantimbuf[canbuftimIDXi].R.id     = CAN_RI1R(CAN1);	// ID, RTR, IDE
	cantimbuf[canbuftimIDXi].R.dlc	  = CAN_RDT1R(CAN1);	// time, data length CAN_RDTxR p 663
	cantimbuf[canbuftimIDXi].R.cd.ui[0] = CAN_RDL1R(CAN1);	// Data (32b) Low
	cantimbuf[canbuftimIDXi].R.cd.ui[1] = CAN_RDH1R(CAN1);	// Data (32b) High

	/* Count errors. */
	if ( (CAN_RF1R(CAN1) & (1 << 4)) != 0 ) can_rx1err += 1;

	/* Release FIFO 1 */	
	CAN_RF1R(CAN1) |= CAN_RF1R_RFOM1;		// Write bit 5 to release FIFO msg

	// Note: in the following the RTR & IDE bits must be off, so it won't be confused with 29 bit addresses
	/* Did someone call for *everybody* to RESET?  If so, execute a system RESET */
	if ( (cantimbuf[canbuftimIDXi].R.id & ~0x7) ==  CAN_RESETALL )
	{
		SCB_AIRCR = (0x5FA << 16) | SCB_AIRCR_SYSRESETREQ;	// Cause a RESET
		while(1==1);
	}

	// Note: in the following the RTR & IDE bits must be off, so it won't be confused with 29 bit addresses
	/* Did someone ask to suspend the sending of CAN msgs? */
	if ( ( (cantimbuf[canbuftimIDXi].R.id & ~0x1) == CAN_SQUELCH ) && ((cantimbuf[canbuftimIDXi].R.id & ~0x1) != (myunitid_local & ~0x1)))
	{
		if ( (cantimbuf[canbuftimIDXi].R.dlc & 0xf) == 8 ) 
		{	// Skip suspending for everybody except the CAN ID in the 2nd word of the payload
			if (cantimbuf[canbuftimIDXi].R.cd.ui[0] <= CAN_WAITDELAYMAX)	// Limit size of squelch duration
			{
				can_suspend_msgs_ctr = cantimbuf[canbuftimIDXi].R.cd.ui[0];	// Set number of 1/2048th sec ticks to count down
				can_suspend_msgs_flag = 1;	// Signal count-down that we have a new value
			}
		}
	}

	/* Advance index in circular buffer */
	canbuftimIDXi = adv_index(canbuftimIDXi, CANRCVTIMBUFSIZE);

	/* Trigger a pending interrupt, which will cause a chain of related routines to execute */
	NVICISPR(NVIC_I2C1_EV_IRQ);	// Set pending (low priority) interrupt ('../lib/libusartstm32/nvicdirect.h')

	temp3 = CAN_RF1R(CAN1);	// Read back register to avoid tail-chaining

	return;
}
/*#######################################################################################
 * ISR routine for CAN: "Status Change Error" -- EWGF, EPVF, BOFF, WKUI, SLAKI
 *####################################################################################### */
/* p 648 The error and status change interrupt can be generated by the following events:
–   Error condition, for more details on error conditions please refer to the CAN Error
    Status register (CAN_ESR).
–   Wakeup condition, SOF monitored on the CAN Rx signal.
–   Entry into Sleep mode. */
void CAN_SCE_Handler(void)
{
//while(1==1);
	return;
}

/*-----------------------------------------------------------------------------
 * void ocphasing1(void);
 * @brief	: Compute value (whole.fraction) used in 'setnextoc()' to bring into phase at next FIFO 1 interrupt
 -----------------------------------------------------------------------------*/
/* Debuggers */
s32 CAN_dif;
s32 CAN_dbg1;
s32 CAN_dbg2;
s32 CAN_dbg3;
s32 CAN_dbg1X;
s32 CAN_dbg2X;
u32 CAN_ticks;
s32 CAN_dev;
s32 CAN_ave;

/* Goal: given the systick register time saved at the FIFO1 interrupt (time sync message), compute the
the amount of adjustment needed to bring the systick timing into "phase", i.e. the 'stk_32ctr' that holds
the current 1/32nd interval within the 1/64th sec time demarcation is brought down to 0 or 31 and the error
of the systick register is brought down to minimum.  One should see the 'stk_32ctr' bouncing around between
31 and 0 when the phase is correct. */

/* Running sums where the whole is added to an OC interval (and removed from the sum) */
static s32	phasing_sum = 0;		// Running sum of accumulated phase adjustment
static s32	deviation_sum = 0;		// Running sum of accumulated error
static s32	phasing_oneinterval = 0;	//

/* "whole.fraction" amounts for adjusting ticks in each SYSTICK interval (2048/sec). */
static s32	deviation_oneinterval = 0;	// Interval adjustment for duration

/* Used for number of ticks in 1/64th sec */
static u32	stk_sum;	// 32 b running sum of SYSTICK increments w/o phasing adjustment
static u32 	stk_sum_prev;	// Previous 'stk_sum'

/* Larger 'PHASINGGAIN' helps jitter.  11 is about right for both xtal and no-xtal */
#define PHASINGGAIN	11	// Number of shifts: 5b for 32 ticks/ 1/64th sec, 6 ticks for gain

/* Used for saving FIFO1 message times during a 1/64th sec SYSTICK period. */
#define	NUMSYNCPER16TH	8	// Number of possible sync msgs per 1/64th sec
static int	idxfifo1 = 0;
static u32	fifo1time[NUMSYNCPER16TH];


/* -------------------------------------------------------------------------------------- 
 * static u32 phase_error(u32 dwt_diff, u32 sum_diff);
 * @param	: dwt_diff = difference between saved FIFO1 time and SYSTICK time
 * @param	: sum_diff = number of ticks in one 1/64th sec period
 * @return	: (signed) error in ticks
   -------------------------------------------------------------------------------------- */
static u32 phase_error(u32 dwt_diff, u32 sum_diff)
{
	s32 delta;

	dwt_diff -= (u32)CAN_TICK_DELAY;
	if ((s32)dwt_diff < 0)	// Did this cause a wrap-around?
	{ // Here, yes.  This changes the sign of the correction.
		delta = -(int)dwt_diff;		
	}
	else
	{ // Here, 'dwt_diff' remained positive after delay adjustment.
		if (dwt_diff > (sum_diff >> 1) )	// Over half-way of for 1/64th sec SYSTICK (ctr = 0 ) interval?
		{ // Here, yes.  The time sync message arrived after the previous SYSTICK (ctr = 0) interrupt
			delta = ((int)sum_diff - (int)dwt_diff); // Increase the counts
		}
		else
		{ // Here, no.  The time sync message arrive before our demarcation.
			delta = -dwt_diff;	// Reduce the counts
		}
	}
	return delta;
}
void systickphasing(void)
{ // Here, we have a new STK_VAL to use for phasing
	s32	delta = 0;	// Sync message arrival ticks before or after the SYSTICK interrupt
s32 x;
	/* Calc duration of previous interval from running sum of duration tick counts. */
	u32	sum_diff = (stk_sum - stk_sum_prev);	// Ticks in preceding 1/64th interval
	stk_sum_prev = stk_sum;
//CAN_dev = sum_diff;
CAN_dbg2 = idxfifo1;

	switch (idxfifo1)
	{
	case 2:	// Phasing may be very close so last one just before, previous one just after prior SYSTICK cycle
		delta = -phase_error((systicktime - fifo1time[1]), sum_diff);
//CAN_dbg3 = delta;
//CAN_dbg2 = (systicktime - fifo1time[1]);
CAN_dbg1 += 1;
		// Drop through and do the 'case 1', summing the two errors.
	case 1:	// The usual case of just one FIFO1 message in one SYSTICK 1/64th sec cycle.
		x = phase_error((systicktime - fifo1time[0]), sum_diff);
		delta += x;
CAN_dif = x;
		can_errors.nosyncmsgctr = 0;	// Reset missing sync msg counter.
		break;

	case 0: // If no sync mesgs during the interval.  Likely to precede or follow case 2.
		phasing_oneinterval = 0;	// Don't accumulate phasing with bogus information

		/* Check if we lost sync msgs, in which case phasing could add to 1/64th sec error */
		can_errors.nosyncmsgctr += 1;
		if (can_errors.nosyncmsgctr > 4)
		{
			phasing_sum = 0;	// Sum no longer contributes.
		}
		break;

	default:	
		/* Time is grossly in error if there are more than two FIFO1 sync messages in one 1/64th sec systick interval. */
		can_errors.error_fifo1ctr += 1;		// Count the bad things.
		break;
	}

//CAN_dev = delta;
//CAN_dbg3 = can_errors.error_fifo1ctr;
phasing_sum = 0;	
	phasing_oneinterval = delta ;	//
//phasing_oneinterval = 0;	// $$$$$$$$$$ Check the clock jitter (no phasing) $$$$$$$$$$$$
	return;
}
/*-----------------------------------------------------------------------------
 * void setnextoc(void);
 * @brief	: Set up next OC register as an increment from 'base'
 -----------------------------------------------------------------------------*/

void setnextoc(void)
{
	s32 	tmp;
	s32 	tmp1;

	/* Goal: set the OC register with the next countdown time */

	/* Determine the time increment for the next interval */

	/* The 'deviation_oneinterval' is for adjusting for the error in the 
           system clock from ideal (64,000,000.0 Hz).  The system tick counts
           between time sync messages is averaged, essentially measuring the
           system clock.  The difference between the measurement and the ideal is
           is used--hence, 'deviation_oneinterval'

           The 'deviation_oneinterval' adjusts for sytem clock *rate*.
           The 'phasing_oneinterval' adjusts for *phasing*.  

           Note--pay attention to the scaling and that the arithemtic is signed.     
        */

	/* A running sum adjusts for the accumulation of fractional ticks. */
	// The following is signed 'whole.fraction' + 'whole.fraction'
	deviation_sum += deviation_oneinterval;	// Add scaled deviation to the running sum

	/* Get the (signed) 'whole' of the 'whole.fraction' */
	tmp = (deviation_sum/(1 << TIMESCALE));	// Tmp is 'whole'

	/* Adjust the sum to account for the amount applied to the interval */
	//   Remove the 'whole' from 'whole.fraction' (Remember: this is signed)
	deviation_sum -= (tmp  << TIMESCALE);	// Remove 'whole' from sum

	/* Same process as above, but for phasing. */
	// 32 intervals per 1/64th sec time msg (5b).  Add some averaging/scaling (8b).
	phasing_sum += phasing_oneinterval;
	tmp1 = (phasing_sum/(1 << PHASINGGAIN));	// 
	phasing_sum -= (tmp1 << PHASINGGAIN);
	
	// Add an increment of ( "nominal" + duration adjustment + phasing adjustment)
	stk_sum += (systicknominal + tmp);	// 32 b running sum of SYSTICK increments w/o phasing adjustment
	STK_LOAD = (systicknominal + tmp + tmp1 - 1);	// Set next interval end time (32b)
	
//CAN_dbg2X += STK_LOAD + 1;
CAN_dbg1X += tmp1;

	return;
}
/*#######################################################################################
 * ISR routine for SYSTICK
 *####################################################################################### */
/*
The systick timer interrupts at (nominally) 2048 per second.  Thirty two intervals comprise
1/64th second.  Time sync messages arrive at 64 per second and update the system clock rate 
measurement and phasing.  The rate and phase adjustments are scale and the fractional amounts
are used, which means the values that are loaded into the systick reload counter vary slightly 
between time sync message updates.

At each systick interrupt (2048 per second) 'systickHIpriority_ptr', if not NULL, is used to
call a routine.  This routine might store an ADC sample for example.  These routines cannot be
called by any other routine that runs at a different interrupt priority.  Furthermore, they
must be fast as this interrupt has a very high priority that preempts other high priority
interrupt processing.

Every 32 interrupts a 'NVIC_I2C2_EV_IRQ' interrupt is forced.  This interrupt has a low priority
for setting up CAN messages to be sent, etc.

*/
static u32	fifo1msgflg_prev;
static u32	fifo1msgflgB;

void SYSTICK_IRQHandler (void)
{
	volatile u32 temp;

	CAN_IER(CAN1) &= ~CAN_IER_FMPIE1;	// Disable FIFO1 interrupts
	temp = CAN_IER(CAN1);			// Readback to assure disabling
//	systicktime = *(volatile unsigned int *)0xE0001004; // DWT_CYCNT
	systicktime = DTWTIME; // DWT_CYCNT

	fifo1time[idxfifo1] = fifo1cycnt;	
	fifo1msgflgB = fifo1msgflg;		// Save time that was saved by FIFO1 msg interrupt
	CAN_IER(CAN1) |= CAN_IER_FMPIE1;	// Re-enable FIFO1 interrupts

	SCB_CFSR &= ~SCB_ICSR_PENDSTCLR;	// Clear SYSTICK interrupt

	/* Count down suspending the sending of CAN msgs counter. */
	if (can_suspend_msgs_work > 0)	// Time to end squelch?
		can_suspend_msgs_work -= 1;	// Count down

	/* Get any new squelch sending CAN msg time counts rcvd by FIFO1 */
	if (can_suspend_msgs_flag != 0) // Did FIFO1 receive a new count?
	{ // Here, yes.
		can_suspend_msgs_work = can_suspend_msgs_ctr; // Copy to working counter
		can_suspend_msgs_flag = 0;	// Reset flag
	}

	/* Call (very short) routine(s) for 1/2048 tick sampling.  These should close out
           end-of-1/64th_sec measurements.  */
	if (systickHIpriority_ptr != 0)	// Having no address for the following is bad.
		(*systickHIpriority_ptr)();	// Go, BUT DON'T TARRY.	

	/* If there was a new fifo1 msg, then step to the next position for saving the times. */
	if (fifo1msgflgB != fifo1msgflg_prev)
	{
		fifo1msgflg_prev = fifo1msgflgB;
		idxfifo1 += 1; if (idxfifo1 >= NUMSYNCPER16TH) idxfifo1 = NUMSYNCPER16TH -1; // Don't overrun buffer
	}	

	stk_32ctr += 1;
	if ( (stk_32ctr & 0x1f) == 0)	// End of 1/64th sec?
	{ // Here, should be the 1/64th sec demarcation
		stk_64flgctr = stk_32ctr;	// Flag ctr for NVIC_I2C2_EV_IRQ interrupt routines
//CAN_dbg2 = CAN_dbg2X;
CAN_dev = CAN_dbg1X;
//CAN_dbg2X = 0;
CAN_dbg1X = 0;
		/* Call (very short) routine(s) for end-of-1/64th_sec measurements.  */
		if (systickHIpriorityX_ptr != 0)	// Somebody could set an address if they wanted.
			(*systickHIpriorityX_ptr)();	// Go, BUT DON'T TARRY.	

		systickphasing();	// Compute an adjustment for phasing
		idxfifo1 = 0;		// Reset index for next SYSTICK cycle of 32.
	}

	/* Set systick reload register with value for next interval */
	setnextoc();

	/* Trigger a lower level pending interrupt, which will cause a chain of related routines to execute */
	// 'stk_64flgctr' is used by these routines to determine if this 2048 tick is the 64th sec tick.
	NVICISPR(NVIC_I2C2_EV_IRQ);	// Set pending (low priority) interrupt ('../lib/libusartstm32/nvicdirect.h')

	temp = SCB_CFSR;	// Readback to avoid tail-chaining causing re-entry
	return;
}
/*#######################################################################################
 * void CAN_sync(u32 fifo1cycnt);
 * entry is from low-level 'I2C1_EV_IRQHandler' following FIFO 1 interrupt
 * fifo1cycnt = DTW_CYCCNT (32b) processor cycle counter saved at FIFO 1 interrupt entry.
 * A FIFO 1 (time sync) msg causes interrupt to CAN_RX1_Handler, which saves the DTW_CYCCNT
 * count, then triggers 'I2C1_EV_IRQHandler' interrupt for low priority level handling, which
 * calls this routine, 'CAN_sync'.
 * This routine serves as a "pseudo" input capture caused by a CAN time sync msg
 *####################################################################################### */
static u32	fifo1cycnt_prev;	// Previous DTW_CYCCNT.  Used to compute difference
static u32	can_ticksper64th;	// Number of ticks between 1/64th sec CAN msg interrupts

static void CAN_sync(u32 fifo1cycnt)
{ // Here, entry is from low-level 'I2C1_EV_IRQHandler' following FIFO 1 interrupt
	s32	tmp;

	/* Ticks per 1/64th sec */
	can_ticksper64th = (fifo1cycnt - fifo1cycnt_prev);
	fifo1cycnt_prev = fifo1cycnt;
CAN_ticks = can_ticksper64th;
	/* Compute +/- clock deviation from ideal */
	tmp = ((int)can_ticksper64th - (int)ticks64thideal);
	/* Check range */
	if ((can_ticksper64th < can_ticksper64thHI) && (can_ticksper64th > can_ticksper64thLO))
	{ // Here, reasonable range
//CAN_dev = tmp;	
		/* Build an average */
		tickavesum -= ticksave[tickaveidx];
		tickavesum += tmp;
		ticksave[tickaveidx] = tmp;
		tickaveidx = adv_index(tickaveidx, TICKAVESIZE);

		/* Compute average as the number of cases builds (a startup issue) */
		ticksaveflag += 1; 
		if (ticksaveflag >= TICKAVESIZE)
			ticksaveflag = TICKAVESIZE;
		tmp = ( (tickavesum << TICKAVEBITS) / ticksaveflag );
CAN_ave = (int)ticks64thideal + tmp/TICKAVESIZE;
		/* Compute a "whole.fraction" for each interval */
		tmp = (tmp << (TIMESCALE -TICKAVEBITS)) + (1 << (TIMESCALE -TICKAVEBITS - 1));	// Scale upwards w rounding
		deviation_one64th = tmp;	// Deviation from ideal for 1/64th sec
		deviation_oneinterval = tmp/32;	// Convert to deviation for 32 ticks per 1/64th sec
//CAN_dev = deviation_oneinterval/(1<<16);	
	}
	
	return;
}
/*#######################################################################################
 * ISR routine for FIFO 1 (higher than other low priority level)
 *####################################################################################### */
void I2C1_EV_IRQHandler(void)
{
	/* Call routine to compute system clock ticks between time sync messages. */
	CAN_sync(fifo1cycnt);
//while(1==1);
/* This interrupt is caused by the CAN FIFO 1 (time sync message) interrupt for further processing at a low interrupt priority */

	/* Call other routines if an address is set up */
	if (highpriority_ptr != 0)	// Having no address for the following is bad.
		(*highpriority_ptr)();	// Go do something
	return;
}
/*#######################################################################################
 * ISR routine for FIFO 0 low priority level
 *####################################################################################### */
void I2C1_ER_IRQHandler(void)
{
	/* Call other routines if an address is set up */
	if (lowpriority_ptr != 0)	// Having no address for the following is bad.
		(*lowpriority_ptr)();	// Go do something
	return;
}
/*#######################################################################################
 * ISR routine for SYSTICK low priority level
 *####################################################################################### */
void I2C2_EV_IRQHandler(void)
{
	/* Call other routines if an address is set up */
	if (systickLOpriority_ptr != 0)	// Having no address for the following is bad.
		(*systickLOpriority_ptr)();	// Go do something
	return;
}
/*#######################################################################################
 * ISR routine for very low priority following CAN_sync routine
 *####################################################################################### */
void I2C2_ER_IRQHandler(void)
{
	/* Call other routines if an address is set up */
	if (fifo1veryLOpriority_ptr != 0)	// Having no address for the following is bad.
		(*fifo1veryLOpriority_ptr)();	// Go do something
	return;
}
/******************************************************************************
 * void canwinch_set_I2C1_ER_add_sys(void* p_lowpriority_ptr);
 * @brief	: Set the pointer for fifo0 low priority interrupt call
 * @param	: pointer to routine to call.
 ******************************************************************************/
void canwinch_set_I2C1_ER_add_sys(void* p_lowpriority_ptr)
{
	lowpriority_ptr = p_lowpriority_ptr;
	return;
}
/******************************************************************************
 * void canwinch_set_I2C1_EV_add_sys(void* p_highpriority_ptr);
 * @brief	: Set the pointer for fifo1 low priority interrupt call
 * @param	: pointer to routine to call.
 ******************************************************************************/
void canwinch_set_I2C1_EV_add_sys(void* p_highpriority_ptr)
{
	highpriority_ptr = p_highpriority_ptr;
	return;
}

