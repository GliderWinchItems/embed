/******************************************************************************
* File Name          : can_gps_phasing.c
* Date First Issued  : 06/03/2015
* Board              : F01 F4
* Description        : Phasing systick clocking with GPS CAN1 msgs
*******************************************************************************/
/*
07/17/2013 rev 209 has glitch problem with systick interrupt priority not being set resolved.
*/
/* 

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
//#include "PODpinconfig.h"
//#include "pinconfig_all.h"
//#include "pinconfig.h"

#include "can_driver.h"
#include "DTW_counter.h"
#include "can_gps_phasing.h"
#include "can_msg_reset.h"


#define LDR_RESET	8
#ifndef NULL 
#define NULL	0
#endif


/* Routine prototypes */
static void can_gps_phasing_msg(void* pctl, struct CAN_POOLBLOCK* pblk);
// void 	(*func_rx)(void* pctl, struct CAN_POOLBLOCK* pblk);

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


/* Pointers to functions to be executed under a low priority interrupt, forced by a CAN interrupt */
// These hold the address of the function that will be called
void 	(*highpriority_ptr)(void) = 0;		// FIFO 1 -> I2C1_EV  (low priority)
void 	(*lowpriority_ptr)(void) = 0;		// FIFO 0 -> I2C1_ER  (low priority
void 	(*systickLOpriority_ptr)(void) = 0;	// SYSTICK -> IC2C2_EV (low priority)
void 	(*systickHIpriority_ptr)(void) = 0;	// SYSTICK handler (very high priority) (2048/sec)
void 	(*systickHIpriorityX_ptr)(void) = 0;	// SYSTICK handler (very high priority) (64/sec)
void 	(*fifo1veryLOpriority_ptr)(void) = 0;	// FIFO 1 -> I2C1_EV -> CAN_sync() -> I2C2_ER (very low priority)


/* Procesor cycle counter is saved upon FIFO 1 interrupt entry */
// DTW module 32b processor cycle counter
static u32	fifo1cycnt;	// Used for processor ticks per sec measurment

/* DWT cycle counter is saved upon SYSTICK interrupt entry */
u32 	systicktime;	// Used for speed computations (or other end-of-1/64th messing around)

static struct CAN_CTLBLOCK* pctlsav;

/*******************************************************************************
 * void can_gps_phasing_init (struct CAN_CTLBLOCK* pctl);
 * @brief	: Initializes SYSTICK and low level interrupts
 * @param	: pctl = pointer to CAN control block 
********************************************************************************/
void can_gps_phasing_init (struct CAN_CTLBLOCK* pctl)
{
	u32	can_ticksper64thdelta;

	pctlsav = pctl;	// Save for later use

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

	/* Configure low-level interrupt interrupt handling of CAN msgs. */
	// Set the callback in 'can_msg_reset' (which is called from 'can_driver' RX) to
        //   to call 'can_gps_phasing_msg' in this routine (which has provision for chaining
        //   a call to another routine.  HOWEVER, this is done under the RX interrupt so
        //   being timely about all of this is important.
	can_msg_reset_ptr = (void*)&can_gps_phasing_msg;	// Entry for CAN RX0 or RX1 handler

	// 'can_gps_phasing' triggers the following low level interrupt if msg of interest--
	// 'I2C1_EV_IRQHandler' (below) calls 'CAN_sync' (below)
	NVICIPR (NVIC_I2C1_EV_IRQ, NVIC_I2C1_EV_IRQ_PRIORITY );	// Set interrupt priority ('../lib/libusartstm32/nvicdirect.h')
	NVICISER(NVIC_I2C1_EV_IRQ);			// Enable interrupt controller ('../lib/libusartstm32/nvicdirect.h')

	/* Every 32 interrupts a 'NVIC_I2C2_EV_IRQ' interrupt is forced. */
	NVICIPR (NVIC_I2C2_EV_IRQ, NVIC_I2C2_EV_IRQ_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_I2C2_EV_IRQ);				// Enable interrupt controller

	// Handles non-high priority msgs, e.g. sensor readings
	NVICIPR (NVIC_I2C1_ER_IRQ, NVIC_I2C1_ER_IRQ_PRIORITY );	// Set interrupt priority ('../lib/libusartstm32/nvicdirect.h')
	NVICISER(NVIC_I2C1_ER_IRQ);			// Enable interrupt controller ('../lib/libusartstm32/nvicdirect.h')

	// Handles non-high priority msgs, e.g. sensor readings
	NVICIPR (NVIC_I2C2_ER_IRQ, NVIC_I2C2_ER_IRQ_PRIORITY );	// Set interrupt priority ('../lib/libusartstm32/nvicdirect.h')
	NVICISER(NVIC_I2C2_ER_IRQ);			// Enable interrupt controller ('../lib/libusartstm32/nvicdirect.h')

	/* Reset current value */
	STK_VAL = 0;

	*(u32*)0xE000ED20 |= (SYSTICK_PRIORITY_SE << 24);	// Set SYSTICK priority
//	SCB_SHPR3 |= (SYSTICK_PRIORITY_SE << 24);	// Set SYSTICK interrupt priority
	
	/* Clock runs at AHB bus speed, interrupts when reaching zero | interrupt enabled */
	STK_CTRL = (STK_CTRL_CLKSOURCE | STK_CTRL_TICKINT | STK_CTRL_ENABLE);

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
		pctlsav->can_errors.nosyncmsgctr = 0;	// Reset missing sync msg counter.
		break;

	case 0: // If no sync mesgs during the interval.  Likely to precede or follow case 2.
		phasing_oneinterval = 0;	// Don't accumulate phasing with bogus information

		/* Check if we lost sync msgs, in which case phasing could add to 1/64th sec error */
		pctlsav->can_errors.nosyncmsgctr += 1;
		if (pctlsav->can_errors.nosyncmsgctr > 4)
		{
			phasing_sum = 0;	// Sum no longer contributes.
		}
		break;

	default:	
		/* Time is grossly in error if there are more than two FIFO1 sync messages in one 1/64th sec systick interval. */
		pctlsav->can_errors.error_fifo1ctr += 1;		// Count the bad things.
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
 * static void setnextoc(void);
 * @brief	: Set up next OC register as an increment from 'base'
 -----------------------------------------------------------------------------*/
static void setnextoc(void)
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
/*#######################################################################################
 * Callled from can_driver.c RX0 or RX1 IRQHandler
 *####################################################################################### */
void (*can_gps_phasing_ptr)(void* pctl, struct CAN_POOLBLOCK* pblk) = NULL;	// Pointer for extending RX0,1 interrupt processing

static void can_gps_phasing_msg(void* pctl, struct CAN_POOLBLOCK* pblk)
{
	__attribute__((__unused__)) void* x = pctl;	// Get rid of unused warning.
	if (pblk->can.id != CANID_HB_TIMESYNC) return;	// Return, CAN msg of no interest

	/* Here, a time sync msg. */
	fifo1cycnt = pblk->x.xw; 	// Save system tick counter (DTW) time-stamp
	fifo1msgflg += 1;		// Flag/ctr for low level interrupt routine

	/* Extend interrupt processing, if pointer set. */
	if (can_gps_phasing_ptr != NULL)	// Skip if no address is setup
		(*can_gps_phasing_ptr)(pctl, pblk);	// Go do something (like trigger low level int)

	/* Trigger a pending interrupt, which will cause a chain of related routines to execute */
	NVICISPR(NVIC_I2C1_EV_IRQ);	// Set pending (low priority) interrupt 
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

void SYSTICK_IRQHandler(void)
{
	 __attribute__((__unused__))int temp;

	CAN_IER(CAN1) &= ~CAN_IER_FMPIE1;	// Disable FIFO1 interrupts
	temp = CAN_IER(CAN1);			// Readback to assure disabling

	systicktime = DTWTIME; // DWT_CYCNT

	fifo1time[idxfifo1] = fifo1cycnt;	
	fifo1msgflgB = fifo1msgflg;		// Save time that was saved by FIFO1 msg interrupt
	CAN_IER(CAN1) |= CAN_IER_FMPIE1;	// Re-enable CAN1 FIFO1 interrupts

	SCB_CFSR &= ~SCB_ICSR_PENDSTCLR;	// Clear SYSTICK interrupt

	/* Count down suspending the sending of CAN msgs counter. */
//$	if (can_suspend_msgs_work > 0)	// Time to end squelch?
//$		can_suspend_msgs_work -= 1;	// Count down

	/* Get any new squelch sending CAN msg time counts rcvd by FIFO1 */
//$	if (can_suspend_msgs_flag != 0) // Did FIFO1 receive a new count?
//$	{ // Here, yes.
//$		can_suspend_msgs_work = can_suspend_msgs_ctr; // Copy to working counter
//$		can_suspend_msgs_flag = 0;	// Reset flag
//$	}

	/* Call (very short) routine(s) for 1/2048 tick sampling.  These should close out
           end-of-1/64th_sec measurements.  */
	if (systickHIpriority_ptr !=  NULL)	// Having no address for the following is bad.
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
		if (systickHIpriorityX_ptr !=  NULL)	// Somebody could set an address if they wanted.
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
 * ISR routine for FIFO 1 (higher than other low priority level)
 *####################################################################################### */
void I2C1_EV_IRQHandler(void)
{
	/* Call routine to compute system clock ticks between time sync messages. */
	CAN_sync(fifo1cycnt);
//while(1==1);
/* This interrupt is caused by the CAN FIFO 1 (time sync message) interrupt for further processing at a low interrupt priority */

	/* Call other routines if an address is set up */
	if (highpriority_ptr !=  NULL)	// Having no address for the following is bad.
		(*highpriority_ptr)();	// Go do something
	return;
}
/*#######################################################################################
 * ISR routine for FIFO 0 low priority level
 *####################################################################################### */
void I2C1_ER_IRQHandler(void)
{
	/* Call other routines if an address is set up */
	if (lowpriority_ptr !=  NULL)	// Having no address for the following is bad.
		(*lowpriority_ptr)();	// Go do something
	return;
}
/*#######################################################################################
 * ISR routine for SYSTICK low priority level: triggered by 'SYSTICK_IRQHandler' (above)
 *####################################################################################### */
void I2C2_EV_IRQHandler(void)
{
	/* Call other routines if an address is set up */
	if (systickLOpriority_ptr !=  NULL)	// Having no address for the following is bad.
		(*systickLOpriority_ptr)();	// Go do something
	return;
}
/*#######################################################################################
 * ISR routine for very low priority following CAN_sync routine
 *####################################################################################### */
void I2C2_ER_IRQHandler(void)
{
	/* Call other routines if an address is set up */
	if (fifo1veryLOpriority_ptr !=  NULL)	// Having no address for the following is bad.
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

