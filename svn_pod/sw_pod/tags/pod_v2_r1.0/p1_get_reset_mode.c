/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : p1_get_reset_mode.c
* Hackeroos          : deh
* Date First Issued  : 08/30/2011
* Board              : STM32F103VxT6_pod_mm (USART1)
* Description        : Sort out type of reset and some other related things ;)
*******************************************************************************/
/*
08-30-2011

This routine takes care of some of the RTC initialization out of which the 
determination of which type of reset we are dealing with (i.e. power up, or
wakeup (which is caused by either rtc alarm, or pushbutton).

Subroutine call references shown as "@n"--
@1 = svn_pod/sw_stm32/trunk/devices/32KHz_p1.c


*/

#include "p1_get_reset_mode.h"
#include "32KHz_p1.h"
#include "p1_common.h"
#include "rtctimers.h"
#include "tickadjust.h"


/* RTC registers: PRL, CNT, ALR initial values (see ../devices/32KHz.h, also p 448 Ref Manual) (see ../devices/32KHz.c) */
struct RTCREG1 strRtc_reg_init = {PRL_DIVIDE_PRE,0,ALR_INCREMENT}; // PRL(+1), CNT, ALR register default values

unsigned int	 uiRCC_CSR_init;	// RCC_CSR saved at beginning of RTC initialization (see 32KHz.c)

/******************************************************************************
 * int p1_get_reset_mode(void);
 * @brief 	: Get started with the sequence when reset is comes out of STANDBY
 * @return	: Reset mode type
*******************************************************************************/
int p1_get_reset_mode(void)
{
	int mode;

	struct RTCREG1 	strRtc_reg_read;	// Readback of registers

	/* Setup addresses for additional RTC (Secf) interrupt handling (under interrupt)--
		p1_RTC_ISRHandler (../devices/32KHz.c) goes to 'rtctimers_countdown' (../devices/rtctimers.c).
		'rtctimers_countdown', then goes to 'rtc_tickadjust'.  The consecutive 'returns'
		then unwind back to the RTC ISR routine and return from interrupt.  */

	p1_rtc_secf_ptr  = &rtctimers_countdown;	// RTC_ISRHandler goes to 'rtctimers_countdown'

	/* Setup the RTC osc and bus interface */	
	p1_Reset_and_RTC_init();	// Save register settings (mostly for debugging) (@1)

	/* See what we have in the RTC registers after 'init routine  */	
	p1_RTC_reg_read(&strRtc_reg_read);		// Read-back of register settings (@1)

	if (p1_cResetFlag == 1)	// After reset was the 32KHz osc up & running? (in 32KHz.c see Reset_and_RTC_init)
	{ // Here, no.  It must have been a power down/up reset, so we need to re-establish the back domain
		/* Load the RTC registers */
	/* ========================================================== */	
 		mode = RESET_MODE_COLD;
	/* ========================================================== */	
		/* Load the RTC registers */
		p1_RTC_reg_load (&strRtc_reg_init);	// Setup RTC registers with default (see above)
	}
	else
	{
	/* ========================================================== */	
 		mode = RESET_MODE_NORMAL;
	/* ========================================================== */

		/* Restore time-keeping values from backup registers and synchronize to RTC_CNT */
		RTC_tickadjust_init_exiting_standby();
	}

	return mode;
}
