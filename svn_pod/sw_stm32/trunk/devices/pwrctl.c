/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : pwrctl.c
* Generator          : deh
* Date First Issued  : 07/03/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Low power management
*******************************************************************************/
/* NOTE:
Page 59 Ref Manual.
*/

#include "spi1ad7799.h"
#include "ad7799_comm.h"
#include "PODpinconfig.h"
#include "libopenstm32/gpio.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/rtc.h"
#include "libopenstm32/pwr.h"
#include "libopenstm32/bkp.h"
#include "libopenstm32/exti.h"
#include "libopenstm32/scb.h"
#include "libusartstm32/nvicdirect.h" 

extern volatile unsigned char ad7799_comm_busy;// 0 = not busy with a sequence of steps, not zero = busy

#include "32KHz.h"

/* Debugging */
extern struct RTCREG 	strRtc_reg_read;	// Readback of registers

/******************************************************************************
 * void Powerdown_to_standby(unsigned int uiInc);
 * @brief	: Setup and then go to STANDBY low power mode (see p 68 Ref Manual)
 * @param	: Set ALR = CNT + uiInc (uiInc in TR_CLK tick time counts, e.g. 32768Hz/16)
 ******************************************************************************/
void Powerdown_to_standby(unsigned int uiInc)
{
	unsigned int temp;


	/* Everybody that might be doing things under interrupt (should!) have a flag on when it is busy
	   so we wait for all these processes to complete */
/* Note: unless the main program calls the routines, the busy flags don't get loaded.  Maybe the 
busy flag should be a bit in one common word */
//	while (	(ad7799_comm_busy != 0) );	// AD7799 communication
//	(cSDcardFlag != 0);			// SD card in-progress to complete
		
	/* Disable RTC interrupts so we aren't interrupted in the middle of updating the ALR register */
	NVICICER(NVIC_RTC_IRQ);		// Disable RTC_IRQ (see ../lib/libusartstm32/nvicdirect.h, and p 122 Programming Manual)

	/* PA0 external LED drive must not be high or else the unit will immediately come out of STANDBY*/
	GPIO_BRR(GPIOA) = 0x01;	// Reset bit	

	/* Be sure PA0 is configured as input (is this necessary?) */
	PA0_reconfig(1);	// Reconfigure so pushbutton can be seen

	/* Set ALR with wakeup time: ALR = CNT + Time increment */	
	RTC_CRL = RTC_CRL_CNF;				// Set bit to enter register configuration mode (p 452)
	while (	(RTC_CRL & RTC_CRL_RTOFF) == 0 );	// We don't expect this to loop	
	temp = ((RTC_CNTL & 0xffff) | (RTC_CNTH << 16));// Get current alarm register as 32 bit unsigned int
	temp += uiInc;					// Advance to set next alarm flag
	RTC_ALRH = (temp >> 16);			// Set high order word of alarm register (p 456)
	RTC_ALRL = (temp & 0xffff);			// Set low order word of alarm register (p 456)
	RTC_CRL = 0;					// Exit configuration mod
	while (	(RTC_CRL & RTC_CRL_RTOFF) == 0 );	// Wait for registers to update


	/* Enter Standby mode when the CPU enters Deepsleep. */
	PWR_CR =  (PWR_CR_PDDS | PWR_CR_CWUF);	// Clear the WUF Wakeup Flag after 2 System clock cycles. (write) (p70)
						// Enter Standby mode when the CPU enters Deepsleep. (p 69)
						// DBP: Write access to RTC & BKP registers disable

	//  PA0- - POD_box (external) LED: gpio out
//	GPIO_CRL(GPIOA) &= ~((0x000f ) << (4*0));	// Clear CNF reset bit 01 = Floating input (reset state)
//	GPIO_CRL(GPIOA) |=  (( (GPIO_CNF_INPUT_FLOAT << 2) | (GPIO_MODE_INPUT) ) << (4*0));

	/* Enable Wake-up so that PA0 pin will cause exit of STANDBY mode */
	 PWR_CSR |= PWR_CSR_EWUP;	// Enable EWUP (p 71)

	/* System control register ( p 233 The Definitive Guide to the ARM Cortex-M3) */
	SCB_SCR |= (1<<2);		// Enable SLEEPDEEP when entering sleep mode

	__asm__("WFE");			// Wait for event

	return;
}
/******************************************************************************
 * void Power_extern_miminum(void);
 * @param	: None as yet
 * @brief	: Set pins for minimum power consumption
 ******************************************************************************/
void Power_extern_miminum(void)
{

	/* Turn off all the external power sucking */
	// (Some of the following probably are not necessary)
	LEDSALL_off	
	ADC7799VCCSW_off
	ENCODERGPSPWR_off
	STRAINGAUGEPWR_off
	MAX3232SW_off
	TXCOSW_off
	BOTTMCELLADC_off
	TOPCELLADC_off
	SDCARD_CS_hi
	AD7799_1_CS_low
	SDCARDREG_off
	return;
}


