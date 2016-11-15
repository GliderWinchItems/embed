/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : p1_shutdown.c
* Generator          : deh
* Date First Issued  : 09/11/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Power down sequence
*******************************************************************************/

#include "p1_common.h"

/*
Key for finding routines, definitions, variables, and other clap-trap
@1 = svn_pod/sw_stm32/trunk/devices/adcpod.c
@2 = svn_pod/sw_pod/trunk/pod_v1/p1_initialization.c
@3 = svn_pod/sw_stm32/trunk/devices/32KHz_p1.c
@4 = svn_pod/sw_pod/trunk/pod_v1/p1_common.c
@5 = svn_pod/sw_stm32/trunk/devices/pwrctl.c
@6 = svn_pod/sw_stm32/trunk/lib/libsupportstm32/tickadjust.c
@7 = svn_pod/sw_pod/trunk/pod_v1/p1_common.h
@8 = svn_pod/sw_stm32/trunk/lib/libsupportstm32/calibration_init.c
@9 = svn_pod/sw_stm32/trunk/lib/libsupportstm32/calibration.h

*/


static void output_calibrated_adc (unsigned int uiT);

/******************************************************************************
 * void p1_shutdown_normal_run(void);
 * @brief	: Sequence for shutting down from the normal_run mode
 ******************************************************************************/
static short state;
static short sTensionPktCtr;
static	struct PKT_BATTMP *pp_batt;
static unsigned int deepsleepticks = DEEPSLEEPTICKCOUNT1;// Normal shutdown sleep time (@7)

int nDebug_shutdown;

void p1_shutdown_normal_run(void)
{
	int i;
	short sTensionEntryCtr;
	struct PKT_PTR pp;
	struct PKT_AD7799 *pt;
	unsigned int uiBotCellCal;
	unsigned int uiTopCellCal;
	unsigned int uiTopCellDif;

	switch (state)
	{
	case 0:	
		/* Check if new battery|temperature data ready */
		pp = adc_packetize_get_battmp_shutdown();	//
		if (pp.ct != 0 )	// New data ready?
		{ // Here, yes.  Check voltages of both cells.
			pp_batt = (struct PKT_BATTMP*)pp.ptr;	// Convert to packet ptr

			/* Compute cell voltages (@8) (@9) */
			uiBotCellCal = ( strDefaultCalib.adcbot * pp_batt->adc[1] )>>16;// Bottom cell
			uiTopCellCal = ( strDefaultCalib.adctop * pp_batt->adc[2] )>>16;// Top of battery (both cells)
			uiTopCellDif = uiTopCellCal - uiBotCellCal; // Top cell = (Top of battery volts - bottom cell volts)

			/* Check if either cell is too low (@8) (@9) */
			if (uiBotCellCal < strDefaultCalib.celllow) // Bottom cell voltage too low?(@8) (@9)
			{ // Here, yes.
				shutdownflag = 2;	// Shutdown until pushbutton wakes up
				USART1_txint_puts("Shutdown caused by low voltage BotCell: ");// Message for the hapless of
			}
			if (uiTopCellDif < strDefaultCalib.celllow) // Top cell voltage too low? (@8) (@9)
			{ // Here yes.
				shutdownflag = 2;
				USART1_txint_puts("Shutdown caused by low voltage TopCell: ");
			}
			/* Setup voltages for output if either cell was too low */
			if (shutdownflag == 2)
			{
				output_calibrated_adc(  uiBotCellCal );	// Bottom cell
				output_calibrated_adc(  uiTopCellCal );	// Top of battery
				output_calibrated_adc(  uiTopCellDif );	// Top cell
				USART1_txint_puts("\n\r");	USART1_txint_send();
				while (USART1_txint_busy() == 1);	// Busy wait (@1)
			}
		}


		/* Check if tension has exceeded "active" threshold and reset timeout if it has. */		
		pp = ad7799_packetize_get_shutdown();
		if (pp.ct > 0)	// New data?
		{ // Here, we have new data.
		/* Let's check for a continuous duration above the threshold */
			sTensionEntryCtr = 0;	// Counter for number of entries greater than threshold
			pt = (struct PKT_AD7799*) pp.ptr;
			/* Check if all entries in packet are above the threshold */
			for ( i = 0; i < PKT_AD7799_TENSION_SIZE; i++)
			{
				if (pt->tension[i] < TENSION_ACTIVE_THRESHOLD) // Tension too low to be an active launch?
				{ // Here yes, below threshold, so break the consecutive packet count
					sTensionPktCtr = 0;	// Reset count of packets
					break;
				}
				else
				{ // Here, count entries above threshold in this packet
					sTensionEntryCtr += 1;
				}
			}

			/* If all the entries were above the threshold, then count the number of such packets */
			if (sTensionEntryCtr == PKT_AD7799_TENSION_SIZE)
			{
				sTensionPktCtr += 1;
				if (sTensionPktCtr > TENSION_ACTIVE_DURATION) // Enough consecutive packets above threshold?
				{ // Here, yes.  we are definitely active.
				/* Reset the time out counter */
					p1_shutdown_timer_reset();	// Reset timeout counter
					sTensionPktCtr -= 1;	// Hold at max value until drops below threshold
				}
			}
		}

		/* If timer has expired, or battery is low, start shutdown process. */
		if  ( ( nTimercounter[1] == 0 ) || (shutdownflag > 0) )
		{
			state = 1;

//char vv[80]; // Char array for debugging sprintf
//if (shutdownflag > 2) 
//{
//	state = 0;
//	sprintf(vv,"shutdownflag: %2u\n\r",shutdownflag);
//	USART1_txint_puts(vv);
//	USART1_txint_send();
//	shutdownflag = 0;
//}
//shuttest('$');

			/* Write SD card with updated calibration, if necessary */
			p1_calibration_close0();
		}
		break;

	case 1:	// shutdown flag has been turned on.

		/* Make sure last is written */
		sdlog_flush();

		/* Check if sd is still has work  */
		if (sdlog_keepalive() == (1==1)) 	// Work to do?
			break;	// Yes.

		/* Kill RTC tick processing beyond the 'tickadjust' routine */
		rtc_tickadjustdone_ptr = 0;	// Cancel subroutine chaining address

		/* Don't shutdown in the middle of a tick adjust */
		while (sPreTickReset != 0);	// Wait if there is a tick adjust in-progress

		/* Save the tickajdust error accumulation and next time adjust (@6) */
		RTC_tickadjust_entering_standby();

		/* Set time (in ticks) for deepsleep according to type of shutdown */
		if (shutdownflag == 2) deepsleepticks = DEEPSLEEPTICKCOUNT2;
		
		/* Load RTC alarm register with wake up time count */
		p1_RTC_reg_load_alr(deepsleepticks);	// This is redundant, but it won't hurt...

		/* Tidy up and go to STANDBY */
		Powerdown_to_standby(deepsleepticks);	// Final wrapup and go into STANDBY (@5). (@7)
		// It never comes back to here: Like the Roach Hotel, they check in, but they don't check out.
		break;
	}
	return;
}

/******************************************************************************
 * void p1_shutdown_deepsleep_run(void);
 * @brief	: Sequence for shutting down from the deepsleep_run mode
 ******************************************************************************/
void p1_shutdown_deepsleep_run(void)
{


	return;
}
/******************************************************************************
 * void p1_shutdown_timer_reset(void);
 * @brief	: Reset the timeout timer that puts unit into deepsleep mode
 ******************************************************************************/
void p1_shutdown_timer_reset(void)
{
	nTimercounter[1] = INACTIVELOGGINGTICKCOUNT;	// Reset timeout counter (@1)

	return;
}
/*****************************************************************************************
Setup an int for a floating pt type output
*****************************************************************************************/
static void output_calibrated_adc (unsigned int uiT)
{
	unsigned int uiX = uiT/1000;	// Whole part
	unsigned int uiR = uiT % 1000;	// Fractional part

	printf (" %8u.%03u ",uiX,uiR);	// Setup the output

	return;
}

