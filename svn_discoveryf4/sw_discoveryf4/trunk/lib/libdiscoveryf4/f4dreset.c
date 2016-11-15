/******************************************************************************
* File Name          : f4dreset.c
* Date First Issued  : 08/15/2014
* Board              : Discovery F4
* Description        : Auto reset for non-cold startup
*******************************************************************************/
/*
When ST-LINK texane completes re-flashing it appears there is not a reset and the
ADC usually does not come up without pressing the RESET button.  This routine checks
if it has executed a software forced reset by checking a RTC backup register.  If bit 0
is zero then it is assumed a system reset had taken place.  If not, then a reset is
forced.
*/

#include "libopencm3/cm3/common.h"
#include "libopencm3/stm32/f4/pwr.h"
#include "libopencm3/stm32/f4/scb.h"
#include "libopencm3/stm32/f4/rtc.h"

/******************************************************************************
 * void f4dreset(void);
 * @brief 	: Reset if we detect that start was no a cold start
*******************************************************************************/
void f4dreset(void)
{
	PWR_CR |= (1 << 8);  // DPB bit: Access to RTC and RTC Backup registers and backup SRAM enabled
	
	/* Unlock Write Protect */
	RTC_WPR = 0xCA;	// 1st key
	RTC_WPR = 0x53;	// 2nd key

	if ((RTC_BKPXR(0) & 0x1) == 0)  // Was there a RESET prior to this?
		RTC_BKPXR(0) |= 0x1;	// Yes. 
	else
	{
		RTC_BKPXR(0) &= ~0x1;	// No.  Execute a reset
		SCB_AIRCR  = (0x5FA << 16) | SCB_AIRCR_SYSRESETREQ;	// Cause a RESET
		while (1==1);		
	}
	return;
}

/******************************************************************************
 * unsigned int f4dreset_getreg(unsigned int x);
 * @brief 	: Unlock and return a backup a register
 * @brief	: After the 1st unlock r/w to the registers can be done directly.
 * @param	: x = Register number 0 - 19
 * @return	: backup register
*******************************************************************************/
unsigned int f4dreset_getreg(unsigned int x)
{
	PWR_CR |= (1 << 8);  // DPB bit: Access to RTC and RTC Backup registers and backup SRAM enabled
	
	/* Unlock Write Protect */
	RTC_WPR = 0xCA;	// 1st key
	RTC_WPR = 0x53;	// 2nd key

	return RTC_BKPXR(x);	// Return backup register x.

}
