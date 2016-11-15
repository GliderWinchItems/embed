/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : PPMadjust.c
* Hackeroo           : deh
* Date First Issued  : 07/21/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Pediodic adjusting 32 KHz osc time-base for temp & offset
*******************************************************************************/

/* Holds the RTC CNT register count that is maintained in memory during power up (see ../devices/32KHz.c)	*/
extern volatile unsigned int uiRTCsystemcounter;	// This mirrors the RTC CNT register, and is updated each RTC Secf interrupt

/* Address this routine will go to upon completion */
void 	(*ppmadjust_done_ptr)(void);	// Address of function to call for 'rtctimers_countdown' to go to for further handling

/* This is an in-memory count of the update time.  During STANDBY this count is maintained
in backup registers BKP_DR1|DR2 and the alarm register (RTC_ALR) is loaded with this time
for the next wake-up */
unsigned int uiPPM_next_timect;		// The RTC_CNT register when a time base adjust is due

/* This tracks the error in the time-base.  The adc reading from the thermistor is used with
the table lookup that gives the ppm * 100 error due to temp.  The calibration struct holds the
nominal osc offset error (ppm * 100).  */

int	nPPM_error_accum;		// Accumulated error
/******************************************************************************
 * void PPMadjust(void);
 *  @brief	: Do a time adjustment check
*******************************************************************************/
void PPMadjust(void)
{
	/* Is it time to do an adjustment? */
	if (uiRTCsystemcounter >= uiPPM_next_timect)
	{ // Here, do a time adjustment check 
		uiPPM_next_timect += PPMADJUSTTIMEINCREMENT;	// Compute RTC_CNT for next check
		
	}

	if (ppmadjust_done_ptr != 0)	// Having no address for the following is bad.
		(*ppmadjust_done_ptr)();	// Go do something (e.g. check timers, or poll AD7799)
	return;
}

