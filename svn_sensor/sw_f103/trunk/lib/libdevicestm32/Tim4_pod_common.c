/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : Tim4_pod_common.h
* Hackeroo           : deh
* Date First Issued  : 02/01/2013
* Board              : STM32F103VxT6_pod_mm
* Description        : Sensor board timing
*******************************************************************************/
/*
02/01/2013 Hack of ../sensor/co1_Olimex/trunk/Tim4_pod_se.h, rev 115
*/

/*
NOTE:
Ref Manual RM0008 Rev 14, p 350



*/
#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/gpio.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/bkp.h"
#include "libopenstm32/timer.h"

#include "bit_banding.h"
#include "pinconfig_all.h"
#include "common.h"
#include "canwinch_pod_common.h"
#include "Tim4_pod_common.h"

/* There is a delay between the absolute "tick" CAN time sync msg being received by the sensor. */
#define CAN_TICK_DELAY	5600	// CAN msg setup and transmission delay


/* The following variables are used by peripherals in their 'init' routines to set dividers 	*/
/* (see 'lib/libmiscstm32/clockspecifysetup.c') */
extern u32	pclk1_freq;	/*	SYSCLKX/PCLK1DIV	E.g. 32000000 	*/
extern u32	sysclk_freq;	/* 	SYSCLK freq		E.g. 64000000	*/


/* CAN time sync high priority msg--shortest possible, and very high priority. */
// msg: time sync 1, dlc = 0, but RTR set, no data 
const struct CANRCVBUF can_msg_timesync = {CAN_TIMESYNC1, 0x2, {0} };	


/* #### => Base this on 64,000,000 TIM4 clock <= #### */

/* Number of intervals in 1/64th sec */
#define TIM4_INT_CT	20	// 1/64th sec is covered by 20 intervals (i.e. interrupts)

/* Each interrupt duration, nominal. */
#define TIM4NOMINAL	50000	// At 64,000,000 p1clk_freq, (50000 * 20) -> 1/64th sec

/* Used to reduce the accumulation of error in intervals scale deviation to "whole.fraction" form. */
#define TIM4SCALE	16	// Number of bits to scale deviation of clock upwards

/* Number of intervals in 1/64th sec */
#define TIM4_NUM_INTERVALS	20	// 1/64th sec is covered by 20 intervals (i.e. interrupts)

/* Various bit lengths of the timer counters are handled with a union */
// Timer counter extended counts
volatile union TIMCAPTURE64	strTim4cnt;	// 64 bit extended TIM4 CH1 timer count
// Input capture extended counts
volatile union TIMCAPTURE64	strTim4;	// 64 bit extended TIM4 CH1 capture
volatile union TIMCAPTURE64	strTim4m;	// 64 bit extended TIM4 CH1 capture (main)

/* The readings and flag counters are updated upon each capture interrupt */
volatile unsigned int		uiTim4ch1_Flag;		// Incremented when a new capture interrupt serviced, TIM4CH1*

/* Holds the RTC CNT register count that is maintained in memory when power is up */
extern unsigned int uiRTCsystemcounter;	// This mirrors the RTC CNT register, and is updated each RTC Secf interrupt

/*  Setup TIM4 CH2: PB7 Output Compare, for alternate function push-pull output p 156, p 163  */
const struct PINCONFIGALL pb7  = {(volatile u32 *)GPIOB, 7, OUT_AF_PP, 0};

/* Pointers to functions to be executed under a low priority interrupt */
void 	(*tim4ic_ptr)(void) = 0;	// Address of function to call forced interrupt

/* Various and sundry items to satisfy the meticulous programmer. */
u16 Tim4ic;		// capture time
u16 Tim4ic_prev;	// capture time, previous
u16 tim4_oc_ctr;		// 'tim4_interval_ctr' at capture time
u16 tim4_oc_ctr_prev;	// 'tim4_interval_ctr' at capture time, previous

u32 Tim4tickctr;
u32 Tim4tickctr_prev;

/* ARR register count */
u16 tim4_delta = TIM4NOMINAL;		// Current interval ct (ARR register)
u16 tim4_delta_new = TIM4NOMINAL;	// New interval ct (picked up at end of 1/64th sec)

/* The following variables are used by peripherals in their 'init' routines to set dividers 	*/
/* (see 'lib/libmiscstm32/clockspecifysetup.c') */
extern u32	pclk1_freq;	/*	SYSCLKX/PCLK1DIV	E.g. 72000000 	*/
extern u32	sysclk_freq;	/* 	SYSCLK freq		E.g. 72000000	*/

/* These are used (mostly!) by the lowlevel routine */
static u32	can_ticksper64thHI;	// TIM4 clock ticks for one sec: high limit
static u32	can_ticksper64thLO;	// TIM4 clock ticks for one sec: low limit

/* "whole.fraction" amounts for adjusting ticks in each OC interval */
s32	deviation_oneinterval = 0;	// Interval adjustment for duration
s32	phasing_oneinterval = 0;	// Interval adjustment for phasing

/* Running sums where the whole is added to an OC interval (and removed from the sum) */
s32	phasing_sum = 0;		// Running sum of accumulated phase adjustment
s32	deviation_sum = 0;		// Running sum of accumulated error

u8 	GPStimegood = 0;	// 0 = waiting for good GPS; 1 = GPS time sentences good
static u8	ticksaveflag = 0;	// 0 = less than one "round"; 1 = full average
u32	tim4lowlevel_er1 = 0;	// Error count 1: bogus time per sec.
u32	tim4_pod_se_debug0  = 0; // Count of 1/64th out-of-sync's
u32	octickaccum = 0;
u32	tim4_ccr2;

/* Pointer for directing the FIFO 1 interrupt */ 
extern void 	(*timing_sync_ptr)(void);	// Address of function to call upon FIFO 1 interrupt

/* Pointer to directing end of 1/64th sec TIM4 output capture */
void 	(*tim4_end64th_ptr)(void) = 0;	// Address of function to call at end of 1/64th sec interval tick

static void tim4lowlevel_init(void);

/******************************************************************************
 * void Tim4_pod_init(void);
 * @brief	: Initialize Tim4 for input capture
*******************************************************************************/
void Tim4_pod_init(void)
{
	/* Setup low level stuf */
	tim4lowlevel_init();

	/* Setup the gpio pin for PA1 is not needed as reset default is "input" "floating" */

	/* ----------- TIM4 CH1  ------------------------------------------------------------------------*/
	/* Enable bus clocking for TIM4 (p 335 for beginning of section on TIM4-TIM7) */
	RCC_APB1ENR |= RCC_APB1ENR_TIM4EN;		// (p 105) 

	/* Enable bus clocking for alternate function */
	RCC_APB2ENR |= (RCC_APB2ENR_AFIOEN);		// (p 103) 


	/*  Setup TIM4 CH2: PB7 Output Compare, for alternate function push-pull output p 156, p 163  */
	/* NOTE: PB7 is for using 'scope to check 1 PPS leading edge versus output compare. */
//	pinconfig_all((struct PINCONFIGALL *)&pb7);	

	/* TIMx capture/compare mode register 1  (p 250 fig 125, and 399) */
	TIM4_CCMR1 |= TIM_CCMR1_CC2S_OUT;	// CH2 configure as output (default)
	TIM4_CCMR1 &= ~(0x3 << 12);		// Clear any previous OC2M bits
	TIM4_CCMR1 |= TIM_CCMR1_OC2M_INACTIVE;	// [1:0] Set to be "inactive" upon OC

	/* Compare/Capture Enable Reg (p 324,5) */
	// CH1 Configured as input: rising edge trigger
	// CH2 Configured as output: active high
	TIM4_CCER |= (0x1 << 4) | (0x1 << 0); 	//  p 401

	/* Prescalar p 402,3 */
//	TIM4_PSC = 1;	// Prescale to divide by 2: reload ct = reg + 1;

	/* Control register 2 */
	// Default: The TIMx_CH2 pin is connected to TI1 input (p 388)

//	TIM4_CR1 |= TIM_CR1_URS;	// Only overflow/underflow causes update interrupt

	/* Control register 1 */
	TIM4_CR1 |= TIM_CR1_CEN; 			// Counter enable: counter begins counting (p 388)+

/* CYCCNT counter is in the Cortex-M-series core.  See the following for details 
http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0337g/BABJFFGJ.html */
	*(volatile unsigned int*)0xE000EDFC |= 0x01000000; // SCB_DEMCR = 0x01000000;
	*(volatile unsigned int*)0xE0001000 |= 0x1;	// Enable DTW_CYCCNT (Data Watch cycle counter)

	NVICIPR (NVIC_I2C2_EV_IRQ, NVIC_I2C2_EV_IRQ_PRIORITY_A );	// Set interrupt priority ('../lib/libusartstm32/nvicdirect.h')
	NVICISER(NVIC_I2C2_EV_IRQ);			// Enable interrupt controller ('../lib/libusartstm32/nvicdirect.h')

	/* Set and enable interrupt controller for TIM4 interrupt */
	NVICIPR (NVIC_TIM4_IRQ, TIM4_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_TIM4_IRQ);			// Enable interrupt controller for TIM4

	/* ===> Following is based on DTW_CYCCNT running at same speed as TIM4_CNT <==== */
	/* Synchronize DTW_CYCCNT with TIM4_CNT */
	// To avoid counter turnover, wait for lower 16 bits of 32b counter to be "close to zero" 
	while ( ( (*(volatile unsigned int *)0xE0001004 & 0xfff0) ) != 0);
	// Load TIM4 counter with DTW_CYCCNT (lower 16b).  Three cycles to get DTW_CYCNT and load TIM4_CNT
	TIM4_CNT = *(volatile unsigned int *)0xE0001004  - 3;

	/* ==> At this point TIM4_CNT should be the same as the lower 16b of DTW_CYCCNT <== */
	
	// Enable CH2 output compare interrupt (p 393)
	TIM4_DIER |= TIM_DIER_CC1IE;

	return;
}
/*-----------------------------------------------------------------------------
 * void ocphasing1(u32 diff);	// OC leads CAN FIFO 1
 * @brief	: Compute value (whole.fraction) used in 'setnextoc()' to bring into phase at next FIFO 1 interrupt
 -----------------------------------------------------------------------------*/
void ocphasing1(u32 diff)
{
	volatile u16 tmp;

	if (diff > 25000) 
	{ // Here, jam 
		tim4_ccr2 = (fifo1cycnt - CAN_TICK_DELAY);		// Jam DTW_CYCCNT saved at last FIFO 1 interrupt 
		tim4_ccr2 += (TIM4NOMINAL);	// Set next interval end time (32b)
		TIM4_CCR2 = tim4_ccr2;		// Load OC register
		tmp = TIM4_CCR2;		// Dummy readback to assure new OC register has the new value
		TIM4_SR = ~0x4;			// Get rid of OC if flag it had come on
	}	
	else
	{	
		/* 'phasing_oneinterval' = ticks difference / number of intervals in 1 second */
		phasing_oneinterval = +( (diff << TIM4SCALE) / TIM4_NUM_INTERVALS );	// Interval = whole.fraction
	}

	return;
}
/*-----------------------------------------------------------------------------
 * void ocphasing1(u32 diff);	// CAN FIFO 1 leads OC
 * @brief	: Compute value (whole.fraction) used in 'setnextoc()' to bring into phase at next FIFO 1 interrupt
 -----------------------------------------------------------------------------*/
void ocphasing2(u32 diff)
{
	volatile u16 tmp;

		if (diff > 25000) 
		{ // Here, jam 
		tim4_ccr2 = (fifo1cycnt - CAN_TICK_DELAY);		// Jam DTW_CYCCNT saved at last FIFO 1 interrupt 
		tim4_ccr2 += (TIM4NOMINAL);	// Set next interval end time (32b)
		TIM4_CCR2 = tim4_ccr2;		// Load OC register
		tmp = TIM4_CCR2;		// Dummy readback to assure new OC register has the new value
		TIM4_SR = ~0x4;			// Get rid of OC if flag it had come on
	}	
	else
	{ // Here, we are further out than one OC interval, so "jam" i.e re-initialize.
		phasing_oneinterval = -( (diff << TIM4SCALE) / TIM4_NUM_INTERVALS );	// Interval = whole.fraction
	}

	return;
}
/*-----------------------------------------------------------------------------
 * void setnextoc(void);
 * @brief	: Set up next OC register as an increment from 'base'
 -----------------------------------------------------------------------------*/
void setnextoc(void)
{
	s32 	tmp;
	s32	tmp1;

	/* Goal: set the OC register with the next OC time */

	/* Determine the time increment for the next interval */

	/* A running sum adjusts for the accumulation of fractional ticks. */
	// The following is signed 'whole.fraction' + 'whole.fraction'
	deviation_sum += deviation_oneinterval;	// Add scaled deviation to the running sum

	/* Get the (signed) 'whole' of the 'whole.fraction' */
	tmp = (deviation_sum/(1 << TIM4SCALE));	// Tmp is 'whole'

	/* Adjust the sum to account for the amount applied to the interval */
	//   Remove the 'whole' from 'whole.fraction' (Remember: this is signed)
	deviation_sum -= (tmp  << TIM4SCALE);	// Remove 'whole' from sum

	/* Same process as above, but for phasing. */
	phasing_sum += phasing_oneinterval;
	tmp1 = phasing_sum/(1 << TIM4SCALE);
	phasing_sum -= (tmp1 << TIM4SCALE);

	// Add an increment of ( "nominal" + duration adjustment + phasing adjustment)
	tim4_ccr2 += (TIM4NOMINAL + tmp + tmp1);	// Set next interval end time (32b)
	TIM4_CCR2 = tim4_ccr2;	// Load OC register

	return;
}
/*#######################################################################################
 * ISR routine for TIM4
 *####################################################################################### */
void TIM4_IRQHandler(void)
{
	volatile unsigned int temp;
	s32 diff;

	if (TIM4_SR & 0x4)	// OC interrupt flag on?
	{ // Here, yes. OC
		tim4_oc_ctr += 1;
		TIM4_SR = ~0x4;	// Reset CH2 OC flag
		
		/* On completion of last interval set the pin high (for scope "verification") */
		if (tim4_oc_ctr == (TIM4_INT_CT -1))	// Starting last interval?
		{
			TIM4_CCMR1 &= ~(0x3 << 12);		// Clear any previous OC2M bits
			TIM4_CCMR1 |= TIM_CCMR1_OC2M_ACTIVE;	// [0:1] Set to be "active" upon OC
		}

		if (tim4_oc_ctr >= TIM4_INT_CT)	// End of 1/64th sec?
		{ // Here, yes.  End of 1/64th second
			/* ========= Go close out end of 1/64th sec measurements ========== */
			if (tim4_end64th_ptr != 0)	// Skip if not set
				(*tim4_end64th_ptr)();	// Go close out measurements (DON'T TARRY!)

			/* Trigger a pending interrupt that will handle the computation and sending of measurements. */
			NVICISPR(NVIC_I2C2_EV_IRQ);	// Set pending (low priority) interrupt  ('../lib/libusartstm32/nvicdirect.h')

			tim4_oc_ctr = 0;		// Reset interval counter
			/* At the end of the 1st interval, the OC sets the pin "inactive" (for scope "verification") */
			TIM4_CCMR1 &= ~(0x3 << 12);		// Clear any previous OC2M bits
			TIM4_CCMR1 |= TIM_CCMR1_OC2M_INACTIVE;	// [1:0] Set to be "active" upon OC

			/* Is CAN FIFO 1 ahead of OC? */
			diff = ((int)fifo1cycnt - (int)tim4_ccr2);
			if (diff < 0)
				diff += 4294967296;

			if (diff > 2147483647)
			{ // Here, OC leads FIFO 1
				ocphasing1(diff);	// Phasing adjustment 
				setnextoc();		// Set up next OC time
			}
			else
			{ // Here, FIFO 1 leads OC
				setnextoc();		// Set up next OC time
				ocphasing2(diff);	// Phasing adjustment 
			}
		}
	}

	temp = TIM4_SR;		// Readback to prevent tailchaining

	return;
}
/*#######################################################################################
 * void CAN_sync(s32 fifo1cycnt);
 * entry is from low-level 'I2C1_EV_IRQHandler' following FIFO 1 interrupt
 * fifo1cycnt = DTW_CYCCNT (32b) processor cycle counter saved at FIFO 1 interrupt entry.
 * A FIFO 1 (time sync) msg causes interrupt to CAN_RX1_Handler, which saves the DTW_CYCCNT
 * count, then triggers 'I2C1_EV_IRQHandler' interrupt for low priority level handling, which
 * calls this routine, 'CAN_sync'.
 * This routine serves as a "pseudo" input capture caused by a CAN time sync msg
 *####################################################################################### */
static u32	fifo1cycnt_prev;	// Previous DTW_CYCCNT.  Used to compute difference
static u32	can_ticksper64th;	// Number of ticks between 1/64th sec CAN msg interrupts

/* Averaging the ticks per 1/64th sec */
#define TIM4_TICKAVEBITS	4	// Number bits in averaging size
#define TIM4_TICKAVESIZE	(1 << TIM4_TICKAVEBITS)	// Use this for scaling
static s16	ticksave[TIM4_TICKAVESIZE];
static u16	tickaveidx = 0;
static s32	tickavesum = 0;

/* Deal with deviation from ideal */
static u32	ticks64thideal = 1000000;	// Ideal number of ticks in 1/64th sec

void CAN_sync(s32 fifo1cycnt)
{ // Here, entry is from low-level 'I2C1_EV_IRQHandler' following FIFO 1 interrupt
	s32	tmp;

	/* Ticks per 1/64th sec */
	can_ticksper64th = (fifo1cycnt - fifo1cycnt_prev);
	fifo1cycnt_prev = fifo1cycnt;

	/* Compute +/- clock deviation from ideal */
	tmp = ((int)can_ticksper64th - (int)ticks64thideal);
	
	/* Check range */
	if ((can_ticksper64th < can_ticksper64thHI) && (can_ticksper64th > can_ticksper64thLO))
	{ // Here, reasonable range
		/* Build an average */
		tickavesum -= ticksave[tickaveidx];
		tickavesum += tmp;
		ticksave[tickaveidx] = tmp;
		tickaveidx += 1; if (tickaveidx > TIM4_TICKAVESIZE) tickaveidx = 0;

		/* Compute average as the number of cases builds (a startup issue) */
		ticksaveflag += 1; 
		if (ticksaveflag >= TIM4_TICKAVESIZE)
			ticksaveflag = TIM4_TICKAVESIZE;
		tmp = ( (tickavesum << TIM4_TICKAVEBITS) / (ticksaveflag + 1) );
	}

		/* Compute a "whole.fraction" for each interval */
		tmp = (tmp << (TIM4SCALE -TIM4_TICKAVEBITS)) + (1 << (TIM4SCALE -TIM4_TICKAVEBITS - 1));	// Scale upwards w rounding
		deviation_oneinterval = ( tmp / TIM4_INT_CT );	// Interval = whole.fraction	

	/* Trigger a pending interrupt, which will cause a chain of related routines to execute */
	NVICISPR(NVIC_I2C2_EV_IRQ);	// Set pending (low priority) interrupt ('../lib/libusartstm32/nvicdirect.h')
	
	return;
};

/*#######################################################################################
 * ISR routine for 'CAN_sync (above) entered from CAN FIFO 1 interrupt low level
 *####################################################################################### */
unsigned int tim4debug1;
unsigned int tim4debug2;

void I2C2_EV_IRQHandler(void)
{

	return;
}
/*-----------------------------------------------------------------------------
 * static void tim4lowlevel_init(void);
 * @brief	: Setup for low level Tim4_pod_se.c
 * @return	: 
 -----------------------------------------------------------------------------*/
/* This routine is a mere shadow of the original */
static void tim4lowlevel_init(void)
{
	unsigned int dur = ((2 * pclk1_freq)/64 ); // Number ticks per OC interrupt
	unsigned int delta = (dur / 10);	// Allow +/- 10% range

	/* Set limits for valid processor ticks between 1 PPS events */
	can_ticksper64thHI = dur + delta;
	can_ticksper64thLO = dur - delta;

	ticks64thideal = pclk1_freq/64;

	return;
}


