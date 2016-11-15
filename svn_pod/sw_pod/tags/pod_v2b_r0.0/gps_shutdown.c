/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : gps_shutdown.c
* Author             : deh
* Date First Issued  : 10/19/2012
* Board              : STM32F103VxT6_pod_mm (USART1)
* Description        : Shutdown gps based on load-cell acitivity
*******************************************************************************/

/*
This routine is called from the polling loop.  It takes the smallest and largest
load-cell readings and determines if there is load-cell acitvity.  If any 
load-cell reading is above a larger threshold it immediate powers up the gps
if it was down.

When there is no load-cell activity a timer timeout will shutdown the gps power.

*/
/*
Subroutine call references shown as "@n"--
@1 = svn_pod/sw_stm32/trunk/lib/libsupport/gps_packetize.h


*/

#include "p1_common.h"	// Just about everything is in this one

#define LOADCELLACTIVE 	800	// Difference in highest and lowest readings that indicates activity
#define LOADCELLALLOUT	10000	// At this level of higher we want the gps on
#define TIMERDURATION	300	// Number of seconds to keep GPS powered when triggered by "activity" 

/* Subroutine prototypes */
int converttograms(int x);

static int nHi = 0x80000000;	// Highest value  = Max negative initially
static int nLo = 0x7fffffff;	// Lowest value = Max positive initially
static int nCt;			// Count number of packets examined
static unsigned long long ullTimer; // Holds the SYS end time for the timeout
static short state;		// Spread the packet examination over many polls
static short usPktIdx;		// Index to array of tensions within a packet
static struct PKT_AD7799 * pp_ad7799;	// Pointer to tension packet being outputted

/******************************************************************************
 * void gps_powerdown(void);
 * @brief 	: Turn off GPS power
*******************************************************************************/
void gps_powerdown(void)
{
	ENCODERGPSPWR_off;	// Diable 5v power regulator
	ullTimer = strAlltime.SYS.ull; // Clear timeout time count

	return;
}
/******************************************************************************
 * void triggerGPSon(int x);
 * @brief 	: Set the timer and turn the GPS on
 *@param	: x = ON time duration in 
*******************************************************************************/
void triggerGPSon(int x)
{
		/* Enable +5 supply to gps (if not already on) */
		ENCODERGPSPWR_on;	// Power up
		/* Reset the timeout time */
		ullTimer = strAlltime.SYS.ull + (x << 11); // Set time out time.

		return;
}
/******************************************************************************
 * void gps_shutdown_poll(void);
 * @brief 	: Check for activity and shutdown/power-up gps
*******************************************************************************/
void gps_shutdown_poll(void)
{
	struct PKT_PTR pp_mon;		// Ptr & ct to current


	switch (state)
	{
	case 0:
		pp_mon = ad7799_packetize_get_monitor();	// Check if packet ready
		if (pp_mon.ct == 0) break;			// Skip if no bytes

		usPktIdx = 0;	// Index for the reading entries within the packet
		pp_ad7799 = (struct PKT_AD7799 *)pp_mon.ptr;	// Convert Char pointer to struct pointer
		state = 1;
	case 1:
		
		if (pp_ad7799->tension[usPktIdx] > nHi ) nHi = pp_ad7799->tension[usPktIdx];			
		if (pp_ad7799->tension[usPktIdx] < nLo ) nLo = pp_ad7799->tension[usPktIdx];

		usPktIdx++;
		if (usPktIdx >= PKT_AD7799_TENSION_SIZE)
		{
			nCt += 1;		// Count number of packets examined
			if (nCt > 2)		// Have we done enough?
			{ // Here yes.  Now see if we should turn the GPS on (if not already on)
				state = 2;
				break;
			}
			usPktIdx = 0;	// Index for the reading entries within the packet
			state = 0;
		}
		break;
		
	case 2:
		/* Convert load-cell reading in packet to grams (makes life easier) */
		nHi = converttograms(nHi);
		nLo = converttograms(nLo);

		if ( ( (nHi-nLo) > LOADCELLACTIVE ) || (nHi > LOADCELLALLOUT) )
		{ // Here, either we have high tension, or the high-low spread indicates activity
			triggerGPSon(TIMERDURATION);
		}
		else
		{
			if ( (signed long long)(strAlltime.SYS.ull - ullTimer) > 0 )
			{
				ENCODERGPSPWR_off;	// Diable 5v power regulator
			}
		}
		/* Reset for next round of checking for load-cell activity */
		state = 0;	nCt = 0;	nHi = 0x80000000;	nLo = 0x7fffffff;
	}
	return;

}
/******************************************************************************
 * static int converttograms(int x);
 * @brief 	: Convert packet reading to grams (same scheme as 'p1_PC_monitor.c')
 * @param	: x = tension reading from packet
*******************************************************************************/
int converttograms(int x)
{
	long long lltemp;
	int ntemp;

	/* Slight adjustment of the load_cell and ad7799 offset for zero */
	lltemp = x + strDefaultCalib.load_cell_zero;

	/* Convert to kgs * 1000 (which I supposed one might call "grams"!) */
	lltemp = lltemp*10000 / strDefaultCalib.load_cell;	// Gain scaling
	ntemp = (int)lltemp;	// Convert back to signed 32 bits

	return ntemp;
}

