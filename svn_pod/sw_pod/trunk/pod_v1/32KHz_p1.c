/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : 32KHz_p1.c
* Hackeroo           : deh
* Date First Issued  : 09/03/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : 32KHz osc/clock, customized for svn_pod/sw_pod/trunk/pod_v1
*******************************************************************************/
/* NOTE:
Backup register BKP_DR1 holds the increment for updating the ALR register
to generate 1 second ticks.
*/

/*
@1 = ~/svn_pod/sw_stm32/trunk/devices/gps_packetize.c
@2 = ~/svn_pod/sw_stm32/trunk/devices/tickadjust.c

*/




#include "spi1ad7799.h"
#include "PODpinconfig.h"
#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/gpio.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/rtc.h"
#include "libopenstm32/pwr.h"
#include "libopenstm32/bkp.h"
#include "libopenstm32/timer.h"

#include "libmiscstm32/systick1.h"

#include "p1_common.h"

static unsigned int RTC_debug1;	// Debugging 32 KHz osc not setup as expected
static unsigned int RTC_debug2;	// Type of reset "we think we had"
static unsigned short DIFtmpctrPrev;	// For updating flag

void 		(*p1_rtc_secf_ptr)(void);	// Address of function to call during RTC_IRQHandler of SECF (Seconds flag)
volatile char	p1_cResetFlag;		// Out of a reset: 1 = 32 KHz osc was not setup; 2 = osc setup OK, backup domain was powered
short		sPreTickReset;		// Reset count for adjusting time for temperature effects and freq offset


/* Time counts used by all */
volatile struct ALLTIMECOUNTS	strAlltime;	// In-memory rtc count, linux real time, and differences


/*
17.3.2 Resetting RTC registers
       All system registers are asynchronously reset by a System Reset or Power Reset, except for
       RTC_PRL, RTC_ALR, RTC_CNT, and RTC_DIV.
       The RTC_PRL, RTC_ALR, RTC_CNT, and RTC_DIV registers are reset only by a Backup
       Domain reset. Refer to Section 6.1.3 on page 82.

17.3.4 Configuring RTC registers
       To write in the RTC_PRL, RTC_CNT, RTC_ALR registers, the peripheral must enter
       Configuration Mode. This is done by setting the CNF bit in the RTC_CRL register.
       In addition, writing to any RTC register is only enabled if the previous write operation is
       finished. To enable the software to detect this situation, the RTOFF status bit is provided in
       the RTC_CR register to indicate that an update of the registers is in progress. A new value
       can be written to the RTC registers only when the RTOFF status bit value is ’1’.
       Configuration procedure:
       1.    Poll RTOFF, wait until its value goes to ‘1’
       2.    Set the CNF bit to enter configuration mode
       3.    Write to one or more RTC registers
       4.    Clear the CNF bit to exit configuration mode
       5.    Poll RTOFF, wait until its value goes to ‘1’ to check the end of the write operation.
       The write operation only executes when the CNF bit is cleared; it takes at least three
       RTCCLK cycles to complete.
17.3.5 RTC flag assertion
       The RTC Second flag (SECF) is asserted on each RTC Core clock cycle before the update
       of the RTC Counter.
       The RTC Overflow flag (OWF) is asserted on the last RTC Core clock cycle before the
       counter reaches 0x0000.
       The RTC_Alarm and RTC Alarm flag (ALRF) (see Figure 180) are asserted on the last RTC
       Core clock cycle before the counter reaches the RTC Alarm value stored in the Alarm
       register increased by one (RTC_ALR + 1). The write operation in the RTC Alarm and RTC
       Second flag must be synchronized by using one of the following sequences:
       ●     Use the RTC Alarm interrupt and inside the RTC interrupt routine, the RTC Alarm
             and/or RTC Counter registers are updated.
       ●     Wait for SECF bit to be set in the RTC Control register. Update the RTC Alarm and/or
             the RTC Counter register.


*/


/******************************************************************************
 * unsigned int p1_Reset_and_RTC_init(void);
 * @brief	: Setup RTC after coming out of a RESET
 * @return	: RCC_CSR register (at the time this routine begins), see p. 109
 ******************************************************************************/
unsigned int p1_Reset_and_RTC_init(void)
{

	unsigned int uiRCC_CSR;		// Type of reset: RCC_CSR register saved

/*
p 447 Ref Manual--
After reset, access to the Backup registers and RTC is disabled and the Backup domain
(BKP) is protected against possible parasitic write access. To enable access to the Backup
registers and the RTC, proceed as follows:
●    enable the power and backup interface clocks by setting the PWREN and BKPEN bits
     in the RCC_APB1ENR register
●    set the DBP bit the Power Control Register (PWR_CR) to enable access to the Backup
     registers and RTC. */
	RCC_APB1ENR |= (RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN); // (see p. 105)

	/* DBP: Disable backup domain write protection. Page 70 Ref Manual */
	PWR_CR |= PWR_CR_DBP;	// Necessary to be able to write to registers after a reset/power up

/* 
Consequently when reading the RTC registers, after having disabled the RTC APB1
interface, the software must first wait for the RSF bit (Register Synchronized Flag) in the
RTC_CRL register to be set by hardware.
Note that the RTC APB1 interface is not affected by WFI and WFE low-power modes.

p 453 Ref Manual
This bit is set by hardware at each time the RTC_CNT and RTC_DIV registers are updated
and cleared by software. Before any read operation after an APB1 reset or an APB1 clock
stop, this bit must be cleared by software, and the user application must wait until it is set to
be sure that the RTC_CNT, RTC_ALR or RTC_PRL registers are synchronized.
0: Registers not yet synchronized.
1: Registers synchronized. 
*/
	uiRCC_CSR =RCC_CSR;			// Save RCC_CSR register for return
//------------------- Start 32 KHz osc if not already running -------------------------------------------------------------
	// Here, we think the backup domain/RTC were powered before this reset event. e.g coming out of STANDBY
	if ( (RCC_BDCR & ( (1<<15) | (0x01<<8)  | 0x01  ) ) != ( (1<<15) | (0x01<<8)  | 0x01  ))
	{ // Here, the 32 KHz is not setup as was expected 	// RCC_BDCR (p 108)
		RCC_BDCR = ( (1<<15) | (0x01<<8)  | 0x01  );	// Enable, select and turn on low speed external osc
		p1_cResetFlag = 1;					// Show the RTC osc had to be setup
		while ( (RCC_BDCR & 0x01) == 0 );		// Wait for 32 KHz external osc to become ready

RTC_debug1 += 1;	// This will be useful for testing STANDBY
RTC_debug2 = 1;		// Setup was done
	}	
	else 
	{ // Here, the osc setup looks good.
		p1_cResetFlag = 2;					// Show the RTC osc did not have to be setup
RTC_debug2 = 2;		// Setup not needed
RTC_debug1 += 1;	// This will be useful for testing STANDBY
	}
//-------------------------------------------------------------------------------------------------------------------------
/* 
Consequently when reading the RTC registers, after having disabled the RTC APB1
interface, the software must first wait for the RSF bit (Register Synchronized Flag) in the
RTC_CRL register to be set by hardware.
Note that the RTC APB1 interface is not affected by WFI and WFE low-power modes.

p 453 Ref Manual
This bit is set by hardware at each time the RTC_CNT and RTC_DIV registers are updated
and cleared by software. Before any read operation after an APB1 reset or an APB1 clock
stop, this bit must be cleared by software, and the user application must wait until it is set to
be sure that the RTC_CNT, RTC_ALR or RTC_PRL registers are synchronized.
0: Registers not yet synchronized.
1: Registers synchronized. 
*/
	/* Make sure registers are synchronized before proceeding */
	RTC_CRL &=  ~RTC_CRL_RSF;		// Clear RSF bit  
	while ( (RTC_CRL & RTC_CRL_RSF) == 0 );	// Wait until registers are sychronized

//-------------------------------------------------------------------------------------------------------------------------
	RCC_CSR |= RCC_CSR_RMVF;	// ReMoVe reset Flags

	/* Clear any pending interrupt flags in the RTC CRL (p. 452) */
	RTC_CRH = RTC_CRH_SECIE;		// SECF interrupt enable.
	/* Wait for register update to complete (otherwise there might be a false interrupt) */
	while (	(RTC_CRL & RTC_CRL_RTOFF) == 0 );// This can take up to 3 RTC_CLK cycles

	/* Set and enable interrupt controller for RTC interrupt (Secons, or Overflow, or Alarm) */
	NVICIPR (NVIC_RTC_IRQ, RTC_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_RTC_IRQ);			// Enable interrupt controller for RTC

// --------------------------------------------------------------------------------------------------------------------------
	/* Set and enable interrupt controller for doing software interrupt */
	NVICIPR (NVIC_TIM6_IRQ, TIM6_IRQ_PRIORITY );	// Set interrupt priority ('../lib/libusartstm32/nvicdirect.h')
	NVICISER(NVIC_TIM6_IRQ);			// Enable interrupt controller for RTC ('../lib/libusartstm32/nvicdirect.h')

	return uiRCC_CSR;	// Return the initial RCC_CSR register (that holds type of reset)
}
/******************************************************************************
 * void p1_RTC_reg_load(struct RTCREG1 * strRtc);
 * @param	: Pointer to struct with values to be loaded into PRL, CNT, ALR
 * @brief	: Setup RTC registers (PRL is decremented by 1 before loading)
 * @brief	: DR1 used for saving the increment to ALR register for 1 sec ticks
 ******************************************************************************/
void p1_RTC_reg_load(struct RTCREG1 * strRtc)
{
	/* Set bit to enter register configuration mode.  This also clears overflow, alarm, and second interrupt flags */
	RTC_CRL = RTC_CRL_CNF;				// Set bit to enter register configuration mode (p 452)

	/* Wait for any previous write operation to complete	*/
	while (	(RTC_CRL & RTC_CRL_RTOFF) == 0 );		// This can take up to 3 RTC_CLK cycles

	/* Set bit to enter register configuration mode.  This also clears overflow, alarm, and second interrupt flags */
	RTC_CRL = RTC_CRL_CNF;				// Set bit to enter register configuration mode (p 452)

	/* Setup registers with values */
	RTC_PRLH = (((strRtc->prl-1) >> 16) & 0x000f);	// Set high order bits of prescaler (p 454)
	RTC_PRLL = ( (strRtc->prl-1) & 0xffff);		// Set low order bits of prescaler (p 454)
	RTC_CNTH = (strRtc->cnt >> 16);			// Set high order word of counter reload register (p 455)
	RTC_CNTL = (strRtc->cnt & 0xffff);		// Set low order word of counter reload register (p 455)
	RTC_ALRH = (strRtc->alr >> 16);			// Set high order word of alarm register (p 456)
	RTC_ALRL = (strRtc->alr & 0xffff);		// Set low order word of alarm register (p 456)

	RTC_CRL = 0;					// Exit configuration mode
	/* Wait for registers to update	*/
	while (	(RTC_CRL & RTC_CRL_RTOFF) == 0 );	// This can take up to 3 RTC_CLK cycles
	
	return;
}
/******************************************************************************
 * void p1_RTC_reg_load_alr(unsigned int uiAlr);
 * @param	: Value to loaded in RTC alarm register
 * @brief	: Set RTC alarm register
 ******************************************************************************/
void p1_RTC_reg_load_alr(unsigned int uiAlr)
{
	/* Wait for any previous write operation to complete	*/
	while (	(RTC_CRL & RTC_CRL_RTOFF) == 0 );		// This can take up to 3 RTC_CLK cycles

	/* Set bit to enter register configuration mode.  This also clears overflow, alarm, and second interrupt flags */
	RTC_CRL = RTC_CRL_CNF;				// Set bit to enter register configuration mode (p 452)

	/* Setup registers with values */
	RTC_ALRH = (uiAlr >> 16);			// Set high order word of alarm register (p 456)
	RTC_ALRL = (uiAlr & 0xffff);			// Set low order word of alarm register (p 456)

	RTC_CRL = 0;					// Exit configuration mode
	/* Wait for registers to update	*/
	while (	(RTC_CRL & RTC_CRL_RTOFF) == 0 );	// This can take up to 3 RTC_CLK cycles
	
	return;
}
/******************************************************************************
 * void p1_RTC_reg_load_prl(unsigned int uiPrl);
 * @param	: Value to loaded in RTC prescalar reload register
 * @brief	: Set up the prescalar
 ******************************************************************************/
void p1_RTC_reg_load_prl(unsigned int uiPrl)
{
	/* Wait for any previous write operation to complete	*/
	while (	(RTC_CRL & RTC_CRL_RTOFF) == 0 );		// This can take up to 3 RTC_CLK cycles

	/* Set bit to enter register configuration mode.  This also clears overflow, alarm, and second interrupt flags */
	RTC_CRL = RTC_CRL_CNF;				// Set bit to enter register configuration mode (p 452)

	/* Setup registers with values */
	RTC_PRLH = (((uiPrl-1) >> 16) & 0x000f);	// Set high order bits of prescaler (p 454)
	RTC_PRLL = ( (uiPrl-1) & 0xffff);		// Set low order bits of prescaler (p 454)

	RTC_CRL = 0;					// Exit configuration mode
	/* Wait for registers to update	*/
	while (	(RTC_CRL & RTC_CRL_RTOFF) == 0 );	// This can take up to 3 RTC_CLK cycles
	
	return;
}


/******************************************************************************
 * void p1_RTC_reg_read(struct RTCREG1 * strRtc);
 * @param	: Pointer to struct into which values are stored
 * @brief	: Read RTC registers and store in struct
 ******************************************************************************/
void p1_RTC_reg_read(struct RTCREG1 * strRtc)
{
	/* Wait for registers to update	*/
	while (	(RTC_CRL & RTC_CRL_RTOFF) == 0 );	// This can take up to 3 RTC_CLK cycles
	strRtc->prl  = (RTC_PRLH << 16);		// Read high order bits of prescaler (p 454)
	strRtc->prl |= (RTC_PRLL & 0xffff);		// Read low order bits of prescaler (p 454)
	strRtc->cnt  = (RTC_CNTH << 16);		// Read high order word of counter register (p 455)
	strRtc->cnt |= (RTC_CNTL & 0xffff);		// Read low order word of counter register (p 455)
	strRtc->alr  = (RTC_ALRH << 16);		// Read high order word of alarm register (p 456)
	strRtc->alr |= (RTC_ALRL & 0xffff);		// Read low order word of alarm register (p 456)
	return;
}
/******************************************************************************
 * unsigned int p1_RTC_reg_read_cnt(void);
 * @brief	: Read RTC CNT (counter that counts TR_CLK)
 * @return	: CNT register as a 32 bit word
 ******************************************************************************/
unsigned int p1_RTC_reg_read_cnt(void)
{
	/* Wait for registers to update	*/
	while (	(RTC_CRL & RTC_CRL_RTOFF) == 0 );	// This can take up to 3 RTC_CLK cycles

	/*  Read high order word and OR with low order word of counter (p 455) */
	return ( (RTC_CNTH << 16) | (RTC_CNTL & 0xffff) );
}
/******************************************************************************
 * unsigned int p1_RTC_tick_synchronize(void);
 * @brief	: Return when RTC_CNT low order two bits have just turned to 00
 * @return	: RTC_CNT 
 ******************************************************************************/
unsigned int p1_RTC_tick_synchronize(void)
{
	unsigned int uiRtc_cnt;

	/* Synchronize with rtc interrupt ticks.
           We sync to the low order two bits, since the software is dividing by 4 
           before incrementing the tick-counter. */
	
	/* First, be sure we are not in the middle of a low bit 00 count */
	uiRtc_cnt = p1_RTC_reg_read_cnt();		// Get current low ord two bits
	while (p1_RTC_reg_read_cnt() == uiRtc_cnt );	// Wait for a tick

	/* Now wait for the two low order bits to become 00 */
	while ( ((uiRtc_cnt = p1_RTC_reg_read_cnt()) & (PRL_DIVIDE_PRE-1)) != 0 );	// Wait for low order bits
	
	return uiRtc_cnt;
}


/*#######################################################################################
 * ISR routine for OWF, ALRF, SECF flags
 *####################################################################################### */
/*
p 453
Note: 1 Any flag remains pending until the appropriate RTC_CR request bit is reset by software,
       indicating that the interrupt request has been granted.
     2 At reset the interrupts are disabled, no interrupt requests are pending and it is possible to
       write to the RTC registers.
     3 The OWF, ALRF, SECF and RSF bits are not updated when the APB1 clock is not running.
     4 The OWF, ALRF, SECF and RSF bits can only be set by hardware and only cleared by
       software.
     5 If ALRF = 1 and ALRIE = 1, the RTC global interrupt is enabled. If EXTI Line 17 is also
       enabled through the EXTI Controller, both the RTC global interrupt and the RTC Alarm
       interrupt are enabled.
     6 If ALRF = 1, the RTC Alarm interrupt is enabled if EXTI Line 17 is enabled through the EXTI
       Controller in interrupt mode. When the EXTI Line 17 is enabled in event mode, a pulse is
       generated on this line (no RTC Alarm interrupt generation).
*/

/* This interrupt is asserted by the RTC_Second, RTC_Overflow, or RTC_Alarm come on and
are not masked */

void p1_RTC_IRQHandler(void)
{
	unsigned int uitemp;

	if ((RTC_CRL & RTC_CRL_SECF) != 0 ) // (see p 452)
	{ // Here, "Seconds" flag, (osc divided by prescalar) TR_CLK (see p 449).

		/* Be sure registers are not in the update process from a previous write */
		while (	(RTC_CRL & RTC_CRL_RTOFF) == 0 );	// We don't expect this to loop	

		/* Since we are only interested in the SECF flag, reset all flags.  Since it take up to 3 RTCLK cycles 
		   for the upate, RTC_CRL writing takes place whilst the fruits of the interrupt are harvested. */
		RTC_CRL = 0;		// Note: the RTOFF bit is read-only, the the others write-zero only

		strAlltime.sPreTick += 1;		// Software divider gets adjusted to adjust for temperature
		if (strAlltime.sPreTick >= PRL_DIVIDE_PRE)	// Divide 4 completed?
		{ // Here, yes, 32768 Osc has been divided by 16 (4 in hardware, 4 with this counter)

			/* The routine to adjust the time for freq error due to temperature sets sPreTickReset (@2) */
			strAlltime.sPreTick = sPreTickReset;	// Reset divide count (usually to zero, but maybe +1 or -1)
			if (sPreTickReset != 0 ) 	// Only one tick adjustment at a time.
				sPreTickReset = 0;	// Clear tick adjust back to zero.

			/* Update the in-memory counter that mirrors the CNT register (differing by adjustment for drift) */

			/* Disable interrupts, since TIM2 is a higher interrupt priority the following might be caught 
                           during the increment. This is needed because the input capture due to the 1 pps pulse 
                           stores strAlltime.SYS.ull */
			TIM2_DIER &= ~(TIM_DIER_CC2IE | TIM_DIER_UIE);	// Disable CH2 capture interrupt and counter overflow (p 315)
			uitemp = TIM2_DIER;				// Readback ensures that interrupts have locked
	
			/* THIS IT!  The time tick counter used by all just and fair-minded citizens.  As you probably have surmised
                           this counter runs at 2048 per sec.  This is the RTC_CNT synchronized and shifted right two bits (since
                           the RTC_CNT counter is running at 8192 per sec).  */
			strAlltime.SYS.ull += 1;	// RTC_CNT, adjusted for drift

			TIM2_DIER |= (TIM_DIER_CC2IE | TIM_DIER_UIE);	// RE-enable CH2 capture interrupt and counter overflow (p 315)

			
			/* GPS adjustment of time.  'DIF is setup in 'gps_packetize.c' */
			if (DIFtmpctr > DIFtmpctrPrev)	// Is there a new 'DIF value?
			{ // Here, yes.  There is a new difference to be handled.
				DIFtmpctrPrev = DIFtmpctr;	// Update new data flag ctr
				if (DIFjamflag == 1)	// Take the difference one big step?
				{ // Here, yes, take update in one step
					strAlltime.SYS.ll += strAlltime.DIF.ll;	// Make the adjustment
					strAlltime.nTickAdjust = 0;		// Zero out 'tickadjust.c' vars
					strAlltime.uiNextTickAdjTime = 0;
					strAlltime.nTickErrAccum = 0;
					DIFjamflag = 0;
				}
			}	

			/* Bring adjust time to GPS via ticks */
			if (TickAdjustflg != 0 ) // Do we have a new tick adjust value?
			{ // Here, yes.
				TickAdjustflg = 0;			// Reset flag
				strAlltime.nTickAdjustRTC = strAlltime.nTickAdjust;// Get local copy of value
				strAlltime.nTickAdjust = 0;		// Reset the sender's value
			}

			/* Adjust one tick at a time unti 'nTickAdjustRTC' is exhausted */
			if (strAlltime.nTickAdjustRTC != 0)	// Done? (Usually zero)
			{ // Here. no.
				if (strAlltime.nTickAdjustRTC < 0)	// Drop ticks?
				{ // Here, negative--we need to drop ticks
					sPreTickReset = +1;	// Drop one tick the next cycle
					strAlltime.nTickAdjustRTC += 1;	// Count up to zero
				}
				else
				{ // Here, positive--we need to add ticks
					sPreTickReset = -1;	// Add one tick the next cycle
					strAlltime.nTickAdjustRTC -= 1;	// Count down to zero
				}
			}

			/* This tick counter times the interval between tick-adjust updates */
			strAlltime.uiNextTickAdjTime += 1;	

			/* Trigger a pending interrupt for TIM6, which will cause a chain of tick related routines to execute */
			NVICISPR(NVIC_TIM6_IRQ);	// Set pending (low priroity) interrupt for TIM6 ('../lib/libusartstm32/nvicdirect.h')

			return;
		}
	}
	else
	{
		/* Be sure registers are not in the update process from a previous write */
		while (	(RTC_CRL & RTC_CRL_RTOFF) == 0 );	// We don't expect this to loop	

		/* Since we are only interested in the SECF flag, reset all flags */
		RTC_CRL = 0;		// Note: the PTOFF bit is read-only, the the others write-zero only
	}

	return;
}
/*#######################################################################################
 * ISR routine for TIM6 (low priority level)
 *####################################################################################### */
void p1_TIM6_IRQHandler(void)
{
/* This interrupt is caused by the RTC interrupt handler when further processing is required */
	/* Call other routines if an address is set up */
	if (p1_rtc_secf_ptr != 0)		// Having no address for the following is bad.
		(*p1_rtc_secf_ptr)();	// Go do something (e.g. poll the AD7799)	
	
	return;
}

/*#######################################################################################
 * ISR routine for ALARM (direct) or WAKEUP pin
 *####################################################################################### */
/* This interrupt comes on when the RTC counter matches the RTC alarm register, when the 
processor is not in STANDBY mode.  In STANDBY mode the the RTC alarm (or Wake up pin) causes
an exit of the STANDBY mode */
void p1_RTC_ALARM_IRQHandler(void)
{
	/* Be sure registers are not in the update process from a previous write */
	while (	(RTC_CRL & RTC_CRL_RTOFF) == 0 );	// We don't expect this to loop	

	/* Since we are only interested in the SECF flag, reset all flags */
	RTC_CRL = 0;		// Note: the PTOFF bit is read-only, the the others write-zero only

	return;
}




