/******************************************************************************
* File Name          : rpmsensor.c
* Date First Issued  : 07/04/2013
* Board              : RxT6
* Description        : RPM sensing
*******************************************************************************/
/* 
Routine to measure rpm on RxT6 board with spark detection hardware fitted.

PB9 - TIM4_CH4, TIM11_CH1, SDIO5_D5, I2C1_SDA, CAN_TX: input from inverter.


*/
#include "libopenstm32/timer.h"
#include "libusartstm32/nvicdirect.h" 
#include "libusartstm32/commonbitband.h"
#include "libmiscstm32/clockspecifysetup.h"

#include "common.h"
#include "../../../../svn_common/trunk/common_can.h"
#include "pinconfig_all.h"
#include "adcsensor_eng.h"
#include "canwinch_pod_common_systick2048.h"
#include "rpmsensor.h"

#include <math.h>

/* Static routines buried in this mess. */
static void Tim4_eng_init(void);
static void rpmsensor_computerpm(void);
static void rpmsensor_savetime(void);

/* RPM computing */
#define RPMSCALE 600	// RPS to RPM * 10
#define PULSEPERREV	4	// Initial, or default, number of pulses per rev
u32 	pulseperrev;	// Number of pulses per revolution (e.g. V8 engine = 4)
static long long llrecip;	// Conversion of ticks to rpm


/* Pointers to functions to be executed under a low priority interrupt */
// These hold the address of the function that will be called
void 	(*systickHIpriority2X_ptr)(void) = 0;	// SYSTICK handler (very high priority) continuation--1/64th
void 	(*systickLOpriority2_ptr)(void) = 0;	// SYSTICK handler (low high priority) continuation--1/2048th
void 	(*systickLOpriority2X_ptr)(void) = 0;	// SYSTICK handler (low high priority) continuation--1/64th


/* Various bit lengths of the timer counters are handled with a union */
// Timer counter extended counts
volatile union TIMCAPTURE64	strTim4cnt;	// 64 bit extended TIM4 CH4 timer count
// Input capture extended counts
volatile union TIMCAPTURE64	strTim4;	// 64 bit extended TIM4 CH4 capture
volatile union TIMCAPTURE64	strTim4m;	// 64 bit extended TIM4 CH4 capture (main)

/* The readings and flag counters are updated upon each capture interrupt */
volatile u32		usTim4ch4_Flag;		// Incremented when a new capture interrupt serviced, TIM4_CH4

/******************************************************************************
 * void rpmsensor_init(void);
 * @brief 	: Initialize TIM4_CH4 and routines to measure rpm
*******************************************************************************/
void rpmsensor_init(void)
{
	/* Some scaling constants */
	llrecip = pclk1_freq;
	pulseperrev = PULSEPERREV; 	// Pick this up from calibration table.
	llrecip = (llrecip * 2 * RPMSCALE) / pulseperrev;

	/* The timer routines are slight mod's to POD timer routines. */
	Tim4_eng_init();	// Initialize TIM4_CH4

	/* This is call from SYSTICK, high priority, 64/sec. */
	systickHIpriorityX_ptr = &rpmsensor_savetime;

	/* This is a low priority post SYSTICK interrupt call, 2048 per sec. */
	systickLOpriority_ptr = &rpmsensor_computerpm;

	return;
}
/******************************************************************************
 * static void Tim4_eng_init(void);
 * @brief	: Initialize Tim4 for input capture
*******************************************************************************/
const struct PINCONFIGALL pin_pb9 = {(volatile u32 *)GPIOB, 9, IN_FLT, 0};


static void Tim4_eng_init(void)
{

// Delay starting TIM4 counter
//volatile int i;
//for (i = 0; i < 1000; i++);	// 1000 results in 13060 count difference between the two timers

	/* Setup the gpio pin for PB9 is not really needed as reset default is "input" "floating" */
	pinconfig_all( (struct PINCONFIGALL *)&pin_pb9);	// p 156

	/* ----------- TIM4 CH4  ------------------------------------------------------------------------*/
	/* Enable bus clocking for TIM4 (p 335 for beginning of section on TIM4-TIM7) */
	RCC_APB1ENR |= RCC_APB1ENR_TIM4EN;		// (p 112) 

	/* Enable bus clocking for alternate function */
	RCC_APB2ENR |= (RCC_APB2ENR_AFIOEN);		// (p 110) 

// Delay starting TIM3 counter
//volatile int i;
//for (i = 0; i < 1000; i++);	// 1000 results in 13060 count difference between the two timers


	/* TIMx capture/compare mode register 1  (p 400) */
	TIM4_CCMR2 |= TIM_CCMR2_CC4S_IN_TI4;		// (p 354)Fig 100 CC4 channel is configured as input, IC4 is mapped on TI4

	/* Compare/Capture Enable Reg (p 324,5) */
	//  Configured as input: falling edge trigger
	TIM4_CCER |= TIM_CCER_CC4E; 	// (0x3<<12);	// Capture Enabled (p 401) 

	/* Control register 2 */
	// Default: The TIMx_CH2 pin is connected to TI1 input (p 372)

	/* Control register 1 */
	TIM4_CR1 |= TIM_CR1_CEN; 			// Counter enable: counter begins counting (p 371,2)

	/* Set and enable interrupt controller for TIM4 interrupt */
	NVICIPR (NVIC_TIM4_IRQ, TIM4_PRIORITY_SE );	// Set interrupt priority
	NVICISER(NVIC_TIM4_IRQ);			// Enable interrupt controller for TIM4
	
	/* Enable input capture interrupts */
	TIM4_DIER |= (TIM_DIER_CC4IE | TIM_DIER_UIE);	// Enable CH4 capture interrupt and counter overflow (p 388,9)

	return;
}
/******************************************************************************
 * unsigned long long Tim4_gettime_ll(void);
 * @brief	: Retrieve the extended timer counter count
 * @return	: Current timer count as an unsigned long long
*******************************************************************************/
unsigned long long Tim4_gettime_ll(void)
{
	union TIMCAPTURE64 strX;
	strTim4cnt.us[0] = TIM4_CNT;	// (p 327) Get current counter value (16 bits)
	/* This 'do' takes care of the case where the counter turns over during the execution */
	do
	{ // Loop if the overflow count changed since the beginning
		strX = strTim4cnt;			// Get current extended count
		strTim4cnt.us[0] = TIM4_CNT;	// (p 327) Get current counter value (16 bits)
	}
	while ( ( strX.ll & ~0xffffLL)  !=   (strTim4cnt.ll & ~0xffffLL) );	// Check if count changed on us
	return strX.ll;	
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

	TIM4_DIER &= ~(TIM_DIER_CC4IE | TIM_DIER_UIE);	// Disable CH4 capture interrupt and counter overflow (p 315)
	tmp = TIM4_DIER;				// Readback ensures that interrupts have locked

/* The following is an alternative for to the two instructions above for assuring that the interrupt enable bits
   have been cleared.  The following results in exactly the same number of instructions and bytes as the above.
   The only difference is the last compiled instruction is 'str' rather than 'ldr'. */
//	tmp = TIM4_DIER;
//	tmp &= ~(TIM_DIER_CC4IE | TIM_DIER_UIE);
//	TIM4_DIER = tmp;
//	TIM4_DIER = tmp;


	strY.ic  = strTim4m.ui[0];			// Get 32b input capture time
	strY.flg = usTim4ch4_Flag; 			// Get flag counter
	TIM4_DIER |= (TIM_DIER_CC4IE | TIM_DIER_UIE);	// Enable CH4 capture interrupt and counter overflow (p 315)
	
	return strY;
}
/*#######################################################################################
 * ISR routine for TIM4
 *####################################################################################### */
void TIM4_IRQHandler(void)
{
	volatile unsigned int temp;

	unsigned short usSR = TIM4_SR & 0x11;	// Get capture & overflow flags

	switch (usSR)	// There are three cases where we do something.  The "00" case is bogus.
	{
	// p 394

	case 0x01:	// Overflow flag only
			TIM4_SR = ~0x1;				// Reset overflow flag
			strTim4cnt.ll	+= 0x10000;		// Increment the high order 48 bits of the timer counter
			temp = TIM4_SR;				// Readback register bit to be sure is cleared
			break;

	case 0x00:	// Case where ic flag got turned off by overlow interrupt reset coinciding with ic signal

	case 0x10:	// Capture flag only
			strTim4.us[0] = TIM4_CCR4;		// Read the captured count which resets the capture flag
			strTim4.us[1] = strTim4cnt.us[1];	// Extended time of upper 16 bits of lower order 32 bits
			strTim4.ui[1] = strTim4cnt.ui[1];	// Extended time of upper 32 bits of long long
			usTim4ch4_Flag += 1;			// Advance the flag counter to signal mailine IC occurred
			strTim4m = strTim4;			// Update buffered value		
			temp = TIM4_SR;				// Readback register bit to be sure is cleared
			break;

	case 0x11:	// Both flags are on	

			// Take care of overflow flag
			TIM4_SR = ~0x1;				// Reset overflow flag

			// Set up the input capture with extended time
			strTim4.us[0] = TIM4_CCR4;		// Read the captured count which resets the capture flag
			strTim4.us[1] = strTim4cnt.us[1];	// Extended time of upper 16 bits of lower order 32 bits
			strTim4.ui[1] = strTim4cnt.ui[1];	// Extended time of upper 32 bits of long long
			// Adjust inpute capture: Determine which flag came first.  If overflow came first increment the overflow count
			if (strTim4.us[0] < 0x8000)		// Is the capture time in the lower half of the range?
			{ // Here, yes.  The IC flag must have followed the overflow flag, so we 
				// First copy the extended time count upper 48 bits (the lower 16 bits have already been stored)
				strTim4.ll	+= 0x10000;	// Increment the high order 48 bits
			}


			usTim4ch4_Flag += 1;			// Advance the flag counter to signal mailine IC occurred		
			strTim4m = strTim4;			// Update buffered value		

			strTim4cnt.ll	+= 0x10000;		// Increment the high order 48 bits of the timer counter

			temp = TIM4_SR;				// Readback register bit to be sure is cleared

			break;
	
	}
	return;
}
/* ######################### UNDER HIGH PRIORITY SYSTICK INTERRUPT ############################### 
 * Enter from SYSTICK interrupt each 64 per sec
 * ############################################################################################### */
static unsigned int endtime;
static unsigned int rpmzeroflg = 0;	// not-zero = wait for rpm pulse
static struct TIMCAPTRET32 endic;
static struct TIMCAPTRET32 endic_prev;
static struct TIMCAPTRET32 endic_prev_prev;

static void rpmsensor_savetime(void)
{
	/* The following values will be used later for computations under low priority interrupt. */
	endtime = Tim4_gettime_ui();	// Save current 32b time tick count (time)
	endic = Tim4_inputcapture_ui();	// Save the last 32b input capture time and ic counter
	
	/* Call other routines if an address is set up */
	if (systickHIpriority2X_ptr != 0)	// Having no address for the following is bad.
		(*systickHIpriority2X_ptr)();	// Go do something	

	return;
}
/* ########################## UNDER LOW PRIORITY SYSTICK INTERRUPT ############################### 
 * Compute the rpm
 * ############################################################################################### */
/* 
   This routine is entered from the SYSTICK interrupt handler triggering I2C2_EV low priority interrupt. 
*/
/******************************************************************************
 * static u32 rpm_arith(u32 numerator, u32 denominator);
 * @brief	: Do the arithemtic to compute rpm (with tick rate adjustment)
 * @return	: rpm
*******************************************************************************/
static u32 rpm_arith(u32 numerator, u32 denominator)
{
/* return = (a * b)/c);
 Where: a = ticks between pulses; b = ticks for 1/64th actual; c = ticks for 1/64th sec ideal
*/
double dnum = numerator;
double dden = denominator;
double dideal = ticks64thideal;
double ddev = deviation_one64th;
double dactual = dideal - (ddev/65536);
double x1 = (dnum / dden) * (dactual/dideal);
double recip = llrecip;
double drpm = recip / x1;
u32 irpm = drpm;
return irpm;

	long long llnum = numerator;			// Ticks between pulses
	long long lldelta = -deviation_one64th;		// Average ticks deviation per 1/64th sec (scaled)
	long long llideal = ticks64thideal;		// With 64MHz sys clk, this would be 1E6 ticks per 1/64th sec
		lldelta += (llideal << TIMESCALE); 	// Average actual ticks per 1/64th sec (scaled by 65536)

	long long llx1 = lldelta * llnum;
	llx1 = llx1/llideal;	// llx1 = (number_of-pulses * actual_clk / ideal_clk) 
	llx1 = llx1/denominator;// llx1 = (number_of_pulses / time_interval) = 1/RPM
	return ( (u32)(llrecip/(llx1 >> TIMESCALE)) ); 	// Return: RPM scaled with "#define RPMSCALE 600"
}

static u32 stk_64flgctr_prev;
u32 rpm;

static void rpmsensor_computerpm(void)
{
	unsigned int ctrdiff;
	unsigned int icdiff;

	/* Was this one the 1/64th sec demarcation? 'canwinch_pod_common_systick2048.c' increments
           'stk_64flgctr' at the end of each 1/64th sec interval. */
	if (stk_64flgctr != stk_64flgctr_prev)
	{ // Here, yes.  Compute the rpm.
	// NOTE: 'adcsensor_eng.c' will set up the CAN message that has pressure and this rpm.
		stk_64flgctr_prev = stk_64flgctr;

		ctrdiff = (endic.flg - endic_prev.flg);	// Number of ic's in last 1/64th sec period;	
		if (ctrdiff != 0)
		{ // Here, one or more rpm pulses during the last 1/64th sec interval
			if (rpmzeroflg == 0)
			{			
				icdiff = (endic.ic - endic_prev.ic);	// Number of intervening timer ticks
				rpm = rpm_arith(icdiff,ctrdiff);	// Compute RPM
			}
			else
			{	
				rpmzeroflg = 0;			// Reset the flag showing rpm forced-to-zero.
				rpm = 0;			// Next cycle will compute a non-zero rpm.
			}
			endic_prev_prev = endic_prev;		// Push the save down one level.
			endic_prev = endic;			// Update previous for next cycle

		}
		else
		{ // Here, there were no rpm pulses during the last 1/64th sec interval.
			if (rpmzeroflg == 0) // Are we in a forced-to-zero situation?
			{ // Here, no.  We have't timed out to qualify for forcing zero rpm.
				icdiff = (endtime - endic_prev.ic);	// Use the time of the 1/64th sec ending
				if ( icdiff > 64000000 ) // Is the interval so that we consider the rpm zero?
				{ // Yes, we timed out between input captures.
					rpmzeroflg = 1;	// Set timed out flag.
					rpm = 0;	// Force rpm to be zero.
				}
				else
				{ // Here, we haven't timed out,
					/* Is the duration from the last pulse until now, greater than the last pulse pair duration? */
					if ( (endtime - endic_prev.ic) > (endic_prev.ic - endic_prev_prev.ic) )
					{ // Here, yes.  Compute a new (and lower) RPM
						rpm = rpm_arith((endtime - endic_prev.ic), 1);	// Compute RPM
					}
					else
					{ // Here, another pulse is likely to arrive
						// Leave the RPM unchanged.
					}
				}
			}
			else
			{ // Here, we timed out and now are waiting for a rpm pulse to get us off of a forced zero.
					rpm = 0;  // Force rpm to be zero.
			}
		}
	}
	/* Call other routines if an address is set up--2048 per sec. */
	if (systickLOpriority2_ptr != 0)	// Having no address for the following is bad.
		(*systickLOpriority2_ptr)();	// Go do something

	return;
}

