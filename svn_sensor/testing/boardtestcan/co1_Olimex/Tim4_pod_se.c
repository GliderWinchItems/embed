/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : Tim4_pod_se.c
* Hackeroo           : deh
* Date First Issued  : 01/26/2013
* Board              : STM32F103RBT6 Olimex
* Description        : Input capture with TIM4 CH1 (PB6) on POD board.
* Note               : Use 'gps_1pps_se.h' with this routine
*******************************************************************************/
/*
01/29/2013 21:04 Rev 112 before big changes to Tim4, Tim2 timing scheme
02/09/2013 12:33 Rev 118 before big 8 possible combo interrupt changes
*/

/*
TIM4 CH1 IC (input capture) is driven by the GPS 1 PPS.  This is used to phase the
OC (output capture) interrupt that runs at 64 Hz.  Upon OC a time sync message is
send on the CAN bus to the sensor units.

*/

#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/gpio.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/bkp.h"
#include "libopenstm32/timer.h"

#include "gps_1pps_se.h"
#include "bit_banding.h"
#include "pinconfig_all.h"
#include "common.h"
#include "canwinch_pod.h"
#include "common_time.h"

volatile unsigned int tim4debug0;
volatile unsigned int tim4debug1;
volatile unsigned int tim4debug2;
volatile unsigned int tim4debug3;
volatile unsigned int tim4debug4;
volatile unsigned int tim4debug5;
volatile unsigned int tim4debug6;
volatile unsigned int tim4debug7;
int db0;
unsigned int db1;
unsigned int db2;
unsigned int db3;
unsigned int db4;
unsigned int db5;
unsigned int db6;



volatile unsigned int tim4cyncnt;
void I2C2_EV_IRQHandler(void);


/* The following variables are used by peripherals in their 'init' routines to set dividers 	*/
/* (see 'lib/libmiscstm32/clockspecifysetup.c') */
extern u32	pclk1_freq;	/*	SYSCLKX/PCLK1DIV	E.g. 32000000 	*/
extern u32	sysclk_freq;	/* 	SYSCLK freq		E.g. 64000000	*/


/* CAN time sync high priority msg--shortest possible, and very high priority. */
// msg: time sync 1, dlc = 0, but RTR set, no data 
const struct CANRCVBUF can_msg_timesync = {CAN_TIMESYNC1, 0x2, {0} };	


/* #### => Base this on 64,000,000 TIM4 clock <= #### */

/* Number of intervals in 1/64th sec */
#define TIM4_NUM_INTERVALS	20	// 1/64th sec is covered by 20 intervals (i.e. interrupts)

/* Each interrupt duration, nominal. */
#define TIM4NOMINAL	50000	// At 64,000,000 p1clk_freq, (50000 * 20) -> 1/64th sec

/* Scale interval durations scale deviation to "whole.fraction" form. */
#define TIM4SCALE	18	// Number of bits to scale deviation of clock upwards

/* CAN time sync msg */
const struct CANRCVBUF can_sync_msg = 
{
CAN_TIMESYNC1 | 0x2,	/* RTR msg */
0,			/* dlc (length) = 0 */
{0}			/* data fields (not used) = 0 */
};


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

/* TIM4 CH1: PB6 GPS 1 PPS, for alternate function input p 156, p 163 */
const struct PINCONFIGALL pb6  = {(volatile u32 *)GPIOB, 6, IN_FLT, 0};

/*  Setup TIM4 CH2: PB7 Output Compare, for alternate function push-pull output p 156, p 163  */
const struct PINCONFIGALL pb7  = {(volatile u32 *)GPIOB, 7, OUT_AF_PP, 0};

/* Pointers to functions to be executed under a low priority interrupt */
void 	(*tim4ic_ptr)(void) = 0;	// Address of function to call forced interrupt

/* Various and sundry items to satisfy the meticulous programmer. */
u16 tim4_oc_ctr;		// 'tim4_interval_ctr' at capture time

u32 Tim4tickctr_prev;

/* The following variables are used by peripherals in their 'init' routines to set dividers 	*/
/* (see 'lib/libmiscstm32/clockspecifysetup.c') */
extern u32	pclk1_freq;	/*	SYSCLKX/PCLK1DIV	E.g. 72000000 	*/
extern u32	sysclk_freq;	/* 	SYSCLK freq		E.g. 72000000	*/

/* These are used (mostly!) by the lowlevel routine */
static u32	tickspersecHI;	// TIM4 clock ticks for one sec: high limit
static u32	tickspersecLO;	// TIM4 clock ticks for one sec: low limit

u32	tim4_ccr2;	// "Previous" OC time

/* "whole.fraction" amounts for adjusting ticks in each OC interval */
s32	deviation_oneinterval = 0;	// Interval adjustment for duration
s32	phasing_oneinterval = 0;	// Interval adjustment for phasing

/* Running sums where the whole is added to an OC interval (and removed from the sum) */
s32	phasing_sum = 0;		// Running sum of accumulated phase adjustment
s32	deviation_sum = 0;		// Running sum of accumulated error

/* Running count of instances where measured tickspersec of clock is outside limits. */
u32	tim4_tickspersec_err;

/* Count cases where there is a '000' interrupt flag isr entry (not expected) */
u32	tim4_zeroflag_ctr;

/* Switch to show 'can_log.c' that the time is ready and logging can start */
u8	tim4_readyforlogging;

/* Count of instances where strAlltime.ull lower 6 bits were not zero at the en of second */
u16	tim4_64th_0_er = 0;	// Count of date/time adjustments at interval 0
u16	tim4_64th_19_er = 0;	// Count of date/time adjustments at interval 19
static u16	tim4_64th_0_er_prev = 0;	// Count of date/time adjustments at interval 0, previous
static u16	tim4_64th_19_er_prev = 0;	// Count of date/time adjustments at interval 19, previous
static u16	tim4_64th_0_ctr = 0;	// Count of changes of 'tim4_64th_0_er'
static u16	tim4_64th_19_ctr = 0;	// Count of changes of 'tim4_64th_19_er'

/* Subroutine prototypes */
static void tim4lowlevel_init(void);

/******************************************************************************
 * void Tim4_pod_init(void);
 * @brief	: Initialize Tim4 for input capture
*******************************************************************************/
void Tim4_pod_init(void)
{
	/* Setup limits */
	tim4lowlevel_init();

	/* ----------- TIM4 CH1  ------------------------------------------------------------------------*/
	/* Enable bus clocking for TIM4 (p 335 for beginning of section on TIM4-TIM7) */
	RCC_APB1ENR |= RCC_APB1ENR_TIM4EN;		// (p 105) 

	/* Enable bus clocking for alternate function */
	RCC_APB2ENR |= (RCC_APB2ENR_AFIOEN);		// (p 103) 

	/* No remap (p 182) */

	/*  Setup TIM4 CH1: PB6 GPS 1 PPS, for alternate function input p 156, p 163  */
	pinconfig_all((struct PINCONFIGALL *)&pb6);	

	/*  Setup TIM4 CH2: PB7 Output Compare, for alternate function push-pull output p 156, p 163  */
	/* NOTE: PB7 is for using 'scope to check 1 PPS leading edge versus output compare. */
//	pinconfig_all((struct PINCONFIGALL *)&pb7);	

	/* TIMx capture/compare mode register 1  (p 250 fig 125, and 399) */
	TIM4_CCMR1 |= TIM_CCMR1_CC1S_IN_TI1;	// (p 379,380)CC1 channel is configured as input, IC1 is mapped on TI1
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
	*(volatile unsigned int*)0xE0001000 |= 0x1;	// Enable DTW_CCNCNT (Data Watch counter)

	NVICIPR (NVIC_I2C2_EV_IRQ, NVIC_I2C2_EV_IRQ_PRIORITY );	// Set interrupt priority ('../lib/libusartstm32/nvicdirect.h')
	NVICISER(NVIC_I2C2_EV_IRQ);			// Enable interrupt controller ('../lib/libusartstm32/nvicdirect.h')

	/* Set and enable interrupt controller for TIM4 interrupt */
	NVICIPR (NVIC_TIM4_IRQ, TIM4_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_TIM4_IRQ);			// Enable interrupt controller for TIM4
	
	/* Enable input capture interrupts */
	// Enable CH2 output compare, CH1 capture, and counter overflow interrupts (p 393)
	TIM4_DIER |= (TIM_DIER_CC2IE | TIM_DIER_CC1IE | TIM_DIER_UIE);	// All three
//	TIM4_DIER |= (TIM_DIER_CC1IE | TIM_DIER_UIE);	// For debugging just IC and overflow

	return;
}
/******************************************************************************
 * unsigned int Tim4_gettime_ui(void);
 * @brief	: Retrieve the extended timer counter count
 * @return	: Current timer count as an unsigned int
*******************************************************************************/
unsigned int Tim4_gettime_ui(void)
{
	union TIMCAPTURE64 strX;			

	strTim4cnt.us[0] = TIM4_CNT;	// (p 327) Get current counter value (16 bits)

	/* This 'do' takes care of the case where the counter turns over during the execution */
	do
	{ // Loop if the overflow count changed since the beginning
		strX.ui[0] = strTim4cnt.ui[0];	// Get low order word of current extended count
		strTim4cnt.us[0] = TIM4_CNT;	// (p 327) Get current counter value (16 bits)
	}
	while ( strX.us[1] != strTim4cnt.us[1] );// Check if extended count changed on us
	return strX.ui[0];			// Return lower 32 bits
}
/******************************************************************************
 * struct TIMCAPTRET32 Tim4_inputcapture_ui(void);
 * @brief	: Retrieve the extended capture timer counter count and flag counter
 * @brief	: Lock interrupts
 * @return	: Current timer count and flag counter in a struct
*******************************************************************************/
struct TIMCAPTRET32 Tim4_inputcapture_ui(void)
{
	struct TIMCAPTRET32 strY;			// 32b input capture time and flag counter
	int	tmp;

	TIM4_DIER &= ~(TIM_DIER_CC1IE | TIM_DIER_UIE);	// Disable CH1 capture interrupt and counter overflow (p 315)
	tmp = TIM4_DIER;				// Readback ensures that interrupts have locked

	strY.ic  = strTim4m.ui[0];			// Get 32b input capture time
	strY.flg = uiTim4ch1_Flag; 			// Get flag counter
	TIM4_DIER |= (TIM_DIER_CC1IE | TIM_DIER_UIE);	// Enable CH1 capture interrupt and counter overflow (p 393)
	
	return strY;
}
/*-----------------------------------------------------------------------------
 * void ocphasing(void);
 * @brief	: Compute value (whole.fraction) used in 'setnextoc()' to bring into phase at next 1 PPS
 -----------------------------------------------------------------------------*/
void ocphasing(void)
/* 
We enter this when there is a 1 PPS IC.  The goal is to either synchronize by jamming
in new values to the OC interrupt scheme if we are way off of correct phasing.
Or, if we are in the 0 or 19 intervals of the 1/64th sec, then we compute a small 
adjustment in the form (whole.fraction) to bring it into close phase over the next
1 sec of OC interrupts. 

Item:  When we come into this routine, as part of the 1 PPS IC handling, the OC may have *preceded*
the IC.  When this happens the oc counter might be at 19, whereas is the OC had been handled first, it
would be at 0.  Hence, the 19 case is checked to see if the OC time difference is "way off".
*/

{
	s32	diff;
	volatile u16 tmp;

tim4debug7 = tim4_oc_ctr;


	if (tim4_oc_ctr == 0)	// Are we in the 1st interval of 1/64th sec?
	{ // Here, the ending OC was after the 1 PPS, so longer OC interval times are needed
		/* Compute the time difference */
		diff = ((int)TIM4_CCR1 - (int)tim4_ccr2);	// (1 PPS IC - OC)
		if (diff < 0)	diff += 65536;	// Adjust for the wrap-around
tim4debug0 = diff;
		if (diff > 12000) diff = 12000;

		/* 'phasing_oneinterval' = ticks difference / number of intervals in 1 second */
		phasing_oneinterval = +( (diff << (TIM4SCALE - 6)) / TIM4_NUM_INTERVALS );	// Interval = whole.fraction

		if ((strAlltime.SYS.ull & 0x3f) != 0)
		{
			strAlltime.SYS.ull &= ~0x3f;	// Reset the lower order 1/64th tick counts
			tim4_64th_0_er += 1;		// Count these for the hapless programmer
		}

	}
	else
	{
		if (tim4_oc_ctr == (TIM4_NUM_INTERVALS - 1)) // Are we in the 20th (last) interval of 1/64th sec?
		{ // Here, the OC ending has yet to occur, so shorter OC interval times are needed
			/* Compute the time difference */
			diff = ((int)TIM4_CCR2 - (int)TIM4_CCR1);	// (OC - 1 PPS IC)
			if (diff < 0)	diff += 65536;	// Adjust for wrap-around
tim4debug0 = -diff; // 
			if (diff > 12000) diff = 12000;
			phasing_oneinterval = -( (diff << (TIM4SCALE - 6)) / TIM4_NUM_INTERVALS );	// Interval = whole.fraction

			if ((strAlltime.SYS.ull & 0x3f) != 63)
			{ // Here, the soon-to-follow OC should increment the 1/64th tick to 0
				strAlltime.SYS.ull |= 0x3f;	// 
				tim4_64th_19_er += 1;		// Count these for the hapless programmer
			}
		}
		else
		{ // Here, we are further out than one OC interval, so "jam" i.e re-initialize.
			TIM4_CCR2 = (TIM4_CCR1 + TIM4NOMINAL);	// Set a new "nominal" increment
			tim4_oc_ctr = 0;		// Interval counter for intervals within 1/64th sec
			phasing_oneinterval = 0;	//
			tmp = TIM4_CCR2;		// Dummy readback to assure new OC register has the new value
			TIM4_SR = ~0x4;			// Get rid of OC if flag it had come on
tim4debug1 += 1;
		}
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

	TIM4_SR = ~0x4;				// Reset OC interrupt flag
	tim4_oc_ctr += 1;			// Count OC intervals to get 1/64th sec
	if (tim4_oc_ctr >= TIM4_NUM_INTERVALS)
	{ // Here, the end of the last interval is the demarcation of the 1/64th sec tick
	/* ======== This is where the CAN msg is intiated ======== */
	can_msg_put((struct CANRCVBUF *)&can_sync_msg);	// Set up CAN thyme sync msg

	/* ======== this is where the time stamp is sync'd to 1/64th sec tick zero ==== */
	strAlltime.SYS.ull += 1;	// Update ticks

	/* See if 'gps_poll.c' signals a new date/time is to be jammed */
		if (gps_poll_flag != 0)
		{
			gps_poll_flag = 0;	// Reset flag
			strAlltime.SYS.ull = strAlltime.GPS.ull; // Load new time (lower 6 bits are zero)
		}
		tim4_oc_ctr = 0;
	}

	/* Goal: set the OC register with the next OC time */

	/* Determine the time increment for the next interval */

	// The following is signed 'whole.fraction' + 'whole.fraction'
	deviation_sum += deviation_oneinterval;	// Add scaled deviation to the running sum

	/* Get the (signed) whole of the 'whole.fraction' */
	tmp = (deviation_sum/(1 << TIM4SCALE));	// Tmp is 'whole'

	/* Adjust the sum to account for the amount applied to the interval */
	//   Remove the 'whole' from 'whole.fraction' (Remember: this is signed)
	deviation_sum -= (tmp  << TIM4SCALE);	// Remove 'whole' from sum

	/* Same process as above, but for phasing. */
	phasing_sum += phasing_oneinterval;
	tmp1 = phasing_sum/(1 << TIM4SCALE);
	phasing_sum -= (tmp1 << TIM4SCALE);

	/* Set up CH2 OC register for next interval duration */
	tim4_ccr2 = TIM4_CCR2;	// Save "previous" OC interval end time

	// Add an increment of ( "nominal" + duration adjustment + phasing adjustment)
	TIM4_CCR2 += (TIM4NOMINAL + tmp + tmp1);	// Set next interval end time

db4 += tmp1;
db6 += tmp;

db5 += (TIM4NOMINAL + tmp + tmp1);
db3 += 1;	// Count OC interrupts

	return;
}
/*#######################################################################################
 * ISR routine for TIM4
 *####################################################################################### */
void TIM4_IRQHandler(void)
{
	u16 usSR = TIM4_SR & 0x7;
	volatile u32 temp;
	s32	diff;

	/* We don't expect this, but to comfirm such a bold expectation, we had better count them! */
	if (usSR == 0)		tim4_zeroflag_ctr += 1;

	/* Extended timing: Handle the 1 PPS input capture (CC!IF) and counter overflow (UIF) flags */
	switch (usSR & 0x3)
	{
	case 0x01:	// Overflow flag only
			TIM4_SR = ~0x1;				// Reset overflow flag
			strTim4cnt.ll	+= 0x10000;		// Increment the high order 48 bits of the time counter
db2 += 1;	// Count OV interrupts
			break;
		
	case 0x2:	// Catpure flag only
tim4cyncnt = *(volatile unsigned int *)0xE0001004; // DWT_CYCNT
tim4debug5 = db5;
tim4debug3 = db3;
tim4debug2 = db2;
tim4debug4 = db4;
tim4debug6 = db6;
		strTim4.us[0] = TIM4_CCR1;		// Read the captured count which resets the capture flag
		strTim4.us[1] = strTim4cnt.us[1];	// Extended time of upper 16 bits of lower order 32 bits
		strTim4.ui[1] = strTim4cnt.ui[1];	// Extended time of upper 32 bits of long long
		strTim4m = strTim4;			// Update buffered value		
		uiTim4ch1_Flag += 1;			// Advance the flag counter to signal mailine IC occurred

		/* Trigger a pending interrupt, to compute interval duration adjustment factor. */
		NVICISPR(NVIC_I2C2_EV_IRQ);	// Set pending (low priority) interrupt ('../lib/libusartstm32/nvicdirect.h')

		break;


	case 0x3:	// Both flags are on	
			// Set up the input capture with extended time
tim4cyncnt = *(volatile unsigned int *)0xE0001004; // DWT_CYCNT
tim4debug5 = db5;
tim4debug3 = db3;

db2 += 1;	// Count OV interrupts
tim4debug2 = db2;
tim4debug4 = db4;
tim4debug6 = db6;


		strTim4.us[0] = TIM4_CCR1;		// Read the captured count which resets the capture flag
		strTim4.us[1] = strTim4cnt.us[1];	// Extended time of upper 16 bits of lower order 32 bits
		strTim4.ui[1] = strTim4cnt.ui[1];	// Extended time of upper 32 bits of long long

		// Take care of overflow flag
		TIM4_SR = ~0x1;				// Reset overflow flag
		strTim4cnt.ll	+= 0x10000;		// Increment the high order 48 bits of the *time counter*

		// Adjust inpute capture: Determine which flag came first.  If overflow came first increment the overflow count
		if (strTim4.us[0] < 32768)		// Is the capture time in the lower half of the range?
		{ // Here, yes.  The IC flag must have followed the overflow flag, so we 
			strTim4.ll	+= 0x10000;	// Increment the high order 48 bits if the *capture counter*
		}
		strTim4m = strTim4;			// Update buffered value		
		uiTim4ch1_Flag += 1;			// Advance the flag counter to signal mailine that an IC occurred		

		/* Trigger a pending interrupt, to compute interval duration adjustment factor. */
		NVICISPR(NVIC_I2C2_EV_IRQ);	// Set pending (low priority) interrupt ('../lib/libusartstm32/nvicdirect.h')

		break;
	
	}
	/* OC output duration & phasing: Handle the 1 PPS input capture (CC!IF) and CH2 output capture (CC2IF) flags */
	switch (usSR & 0x6)	
	{ // Here, an OC interrupt flag
	case 0x2:	// IC only
			ocphasing();	// Phasing adjustment 
		break;
	case 0x4:	// OC only
			setnextoc();	// Set next interval for OC

		break;
	case 0x6:	// IC and OC were both on at isr entry (i.e. when usSR stored the SR)

			/* Is IC ahead of OC? */
			diff = ((int)TIM4_CCR1 - (int)TIM4_CCR2);	// IC - OC (registers)
			if (diff < 0)
				diff += 65536;

			if (diff > 32767)
			{ // Here, OC leads IC
				ocphasing();	// Phasing adjustment 
				setnextoc();	// Set up next OC time
			}
			else
			{ // Here, IC leads OC
				setnextoc();	// Set up next OC time
				ocphasing();	// Phasing adjustment 
			}
		break;
	}
	temp = TIM4_SR;	// Prevent tail-chaining
	return;
}
/*#######################################################################################
 * ISR routine for TIM4 input capture low priority level
 *####################################################################################### */
unsigned int ticks;
unsigned int ticks_dev;
unsigned int ticks_flg;

void I2C2_EV_IRQHandler(void)
{
/* This interrupt is caused by the GPS 1PPS input capture handling, used for further processing at a low interrupt priority */
	s32 ticks_temp;
	s32 tmp;
	struct TIMCAPTRET32 strIC;	// 1 PPS input capture time (32b), and flag counter


	// Here GPS time is good so we can begin measuring tim4 clock rate
//	strIC = Tim4_inputcapture_ui();			// Get input capture time and flag ctr
	strIC.ic  = strTim4m.ui[0];			// Get 32b input capture time
	strIC.flg = uiTim4ch1_Flag; 
	ticks_temp = (s32)(strIC.ic - Tim4tickctr_prev);// Ticks in one second
	Tim4tickctr_prev = strIC.ic;			// Save for next time

	ticks_flg += 1;
ticks = ticks_temp;

	/* Check that we don't have some bogus value (e.g. a startup problem, etc.) */
	if ( ( (u32)ticks_temp > tickspersecLO) && ( (u32)ticks_temp < tickspersecHI) ) // Allow +/- 10%
	{ // Here, passing the check makes it "good enough" 
		/* Compute a "whole.fraction" for each interval */
		tmp = (ticks_temp - (2 * pclk1_freq));		// Deviation of ticks in one sec from perfection
ticks_dev = tmp;
		tmp *= (1 << (TIM4SCALE - 6));	// Scale upwards and divide by 64 (hence the -6);
		deviation_oneinterval = ( tmp / TIM4_NUM_INTERVALS );	// Interval = whole.fraction
	}
	else
	{
		tim4_tickspersec_err += 1;
	}

	/* Logic to wait for GPS and time sync'ing to settled down before signalling can_log.c that logging can begin */
	/* 'ocphasing()' is where the 'er count is made */
	/* When 'tim4_readyforlogging == 0x3' then the time is stable */
	if (tim4_64th_0_er != tim4_64th_0_er_prev)
	{ // Here, the 1 PPS at interval 0 needed adjustment...not ready
		tim4_64th_0_er_prev = tim4_64th_0_er;
		tim4_64th_0_ctr = 0;		 
		tim4_readyforlogging &= ~0x1;
	}
	else
	{ // Here, no adjustment needed.  Count a "few" 
		if (tim4_64th_0_ctr >= 3)
			tim4_readyforlogging |= 0x1;
		else
			tim4_64th_0_ctr += 1;
	}

	/* Do the same as above, but for interval 19. */
	if (tim4_64th_19_er != tim4_64th_19_er_prev)
	{
		tim4_64th_19_er_prev = tim4_64th_19_er;
		tim4_64th_19_ctr = 0;		 
		tim4_readyforlogging &= ~0x2;
	}
	else
	{ // Here, no adjustment needed.  Count a "few" 
		if (tim4_64th_19_ctr >= 3)
			tim4_readyforlogging |= 0x2;
		else
			tim4_64th_19_ctr += 1;
	}
	/* Check that the adjustment factor is under control. */
	if ( (deviation_oneinterval > 500000) || (deviation_oneinterval < -500000) )
	{ // Here, the adjustment is way out we might get bogus no-change intervals 0 and 19.
		tim4_64th_0_ctr = 0; tim4_64th_19_ctr = 0; tim4_readyforlogging = 0;
	}

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
	unsigned int temp = ((2 * pclk1_freq)/10);	// Allow +/- 10% range on ticks per sec

	/* Set limits for valid processor ticks between 1 PPS events */
	tickspersecHI = (2 * pclk1_freq) + temp;
	tickspersecLO = (2 * pclk1_freq) - temp;
//tickspersecLO = (64000000 - 32768);
	return;
}

