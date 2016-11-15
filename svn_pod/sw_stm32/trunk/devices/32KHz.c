/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : 32KHz.c
* Hackerees          : deh
* Date First Issued  : 06/24/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : 32KHz ocs/clock routines
*******************************************************************************/
/* NOTE:
Backup register BKP_DR1 holds the increment for updating the ALR register
to generate 1 second ticks.
*/


#include "spi1ad7799.h"
#include "PODpinconfig.h"
#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/gpio.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/rtc.h"
#include "libopenstm32/pwr.h"
#include "libopenstm32/bkp.h"
#include "libmiscstm32/systick1.h"


#include "gps_1pps.h"		// Timer routines for this app
#include "32KHz.h"

unsigned int RTC_debug0;	// Debugging bogus interrupt
unsigned int RTC_debug1;	// Debugging 32 KHz osc not setup as expected
unsigned int RTC_debug2;	// Type of reset "we think we had"


void 		(*rtc_secf_ptr)(void);	// Address of function to call during RTC_IRQHandler of SECF (Seconds flag)
char		cResetFlag;		// Out of a reset: 1 = 32 KHz osc was not setup; 2 = osc setup OK, backup domain was powered
unsigned int	uiSecondsFlag;		// 1 second tick: 0 = not ready, + = seconds cound

/* Holds the RTC CNT register count that is maintained in memory when power is up */
unsigned int uiRTCsystemcounter;	// This mirrors the RTC CNT register, and is updated each RTC Secf interrupt

/* Count TR_CLK (32768 / 16) to generate a one second flag 'uiSecondsFlag' */
static u32 uiOnesecTickctr;


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
 * unsigned int Reset_and_RTC_init(void);
 * @brief	: Setup RTC after coming out of a RESET
 * @return	: RCC_CSR register (at the time this routine begins), see p. 109
 ******************************************************************************/
unsigned int Reset_and_RTC_init(void)
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
	{ // Here, the 32 KHz is not setup as was expected 
		RCC_BDCR = ( (1<<15) | (0x01<<8)  | 0x01  );	// Enable, select and turn on low speed external osc
		cResetFlag = 1;					// Show the RTC osc had to be setup
		while ( (RCC_BDCR & 0x01) == 0 );		// Wait for 32 KHz external osc to become ready

RTC_debug1 += 1;	// This will be useful for testing STANDBY
RTC_debug2 = 1;		// Setup was done
	}	
	else 
	{ // Here, the osc setup looks good.
		cResetFlag = 2;					// Show the RTC osc did not have to be setup
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
	NVICIPR (NVIC_RTC_IRQ, RTC_PRIORITY );	// Set interrupt priority ('../lib/libusartstm32/nvicdirect.h')
	NVICISER(NVIC_RTC_IRQ);			// Enable interrupt controller for RTC ('../lib/libusartstm32/nvicdirect.h')

// --------------------------------------------------------------------------------------------------------------------------
	/* Set and enable interrupt controller for doing software interrupt */
	NVICIPR (NVIC_TIM6_IRQ, TIM6_IRQ_PRIORITY );	// Set interrupt priority ('../lib/libusartstm32/nvicdirect.h')
	NVICISER(NVIC_TIM6_IRQ);			// Enable interrupt controller for RTC ('../lib/libusartstm32/nvicdirect.h')


	return uiRCC_CSR;	// Return the initial RCC_CSR register (that holds type of reset)
}
/******************************************************************************
 * void RTC_reg_load(struct RTCREG * strRtc);
 * @param	: Pointer to struct with values to be loaded into PRL, CNT, ALR
 * @brief	: Setup RTC registers (PRL is decremented by 1 before loading)
 * @brief	: DR1 used for saving the increment to ALR register for 1 sec ticks
 ******************************************************************************/
void RTC_reg_load(struct RTCREG * strRtc)
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
 * void RTC_reg_load_alr(unsigned int uiAlr);
 * @param	: Value to loaded in RTC alarm register
 * @brief	: Set RTC alarm register
 ******************************************************************************/
void RTC_reg_load_alr(unsigned int uiAlr)
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
 * void RTC_reg_load_prl(unsigned int uiPrl);
 * @param	: Value to loaded in RTC prescalar reload register
 * @brief	: Set up the prescalar
 ******************************************************************************/
void RTC_reg_load_prl(unsigned int uiPrl)
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
 * void RTC_reg_read(struct RTCREG * strRtc);
 * @param	: Pointer to struct into which values are stored
 * @brief	: Read RTC registers and store in struct
 ******************************************************************************/
void RTC_reg_read(struct RTCREG * strRtc)
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
 * unsigned int RTC_reg_read_cnt(void);
 * @brief	: Read RTC CNT (counter that counts TR_CLK)
 * @return	: CNT register as a 32 bit word
 ******************************************************************************/
unsigned int RTC_reg_read_cnt(void)
{
	/* Wait for registers to update	*/
	while (	(RTC_CRL & RTC_CRL_RTOFF) == 0 );	// This can take up to 3 RTC_CLK cycles

	/*  Read high order word and OR with low order word of counter (p 455) */
	return ( (RTC_CNTH << 16) | (RTC_CNTL & 0xffff) );
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

void RTC_IRQHandler(void)
{
	unsigned int temp;

	if ((RTC_CRL & RTC_CRL_SECF) != 0 ) // (see p 452)
	{ // Here, "Seconds" flag, (osc divided by prescalar) TR_CLK (see p 449).

		/* Be sure registers are not in the update process from a previous write */
		while (	(RTC_CRL & RTC_CRL_RTOFF) == 0 );	// We don't expect this to loop	

		/* Since we are only interested in the SECF flag, reset all flags.  Since it take up to 3 RTCLK cycles 
		   for the upate, RTC_CRL writing takes place whilst the fruits of the interrupt are harvested. */
		RTC_CRL = 0;		// Note: the RTOFF bit is read-only, the the others write-zero only

		/* Update the in-memory counter that mirrors the CNT register */
		uiRTCsystemcounter += 1;

		/* Cause TIM6 interrupt which will call subroutines that run at a low priority level */
		NVICISPR(NVIC_TIM6_IRQ);	// Set pending (low priroity) interrupt for TIM6 ('../lib/libusartstm32/nvicdirect.h')

	}
	else
	{
		/* Be sure registers are not in the update process from a previous write */
		while (	(RTC_CRL & RTC_CRL_RTOFF) == 0 );	// We don't expect this to loop	

		/* Since we are only interested in the SECF flag, reset all flags */
		RTC_CRL = 0;		// Note: the PTOFF bit is read-only, the the others write-zero only
	}

	/* Do a in-memory one second tick (instead of using the ALR register) */
	uiOnesecTickctr += 1;
	if (uiOnesecTickctr >= ALR_INCREMENT)
	{
		uiOnesecTickctr = 0;
		uiSecondsFlag += 1;				// A flag for others to observe
		temp = RTC_reg_read_cnt();
		temp += ALR_INCREMENT;			
		RTC_reg_load_alr(temp);
	}
	return;
}
/*#######################################################################################
 * ISR routine for TIM6 (low priority level)
 *####################################################################################### */
void TIM6_IRQHandler(void)
{
/* This interrupt is caused by the RTC interrupt handler when further processing is required */
	/* Call other routines if an address is set up */
	if (rtc_secf_ptr != 0)		// Having no address for the following is bad.
		(*rtc_secf_ptr)();	// Go do something (e.g. poll the AD7799)	
	
	return;
}


/*#######################################################################################
 * ISR routine for ALARM (direct) or WAKEUP pin
 *####################################################################################### */
/* This interrupt comes on when the RTC counter matches the RTC alarm register, when the 
processor is not in STANDBY mode.  In STANDBY mode the the RTC alarm (or Wake up pin) causes
an exit of the STANDBY mode */
void RTC_ALARM_IRQHandler(void)
{
	while (1==1);	// Trap since shouldn't be implemented

	return;
}



