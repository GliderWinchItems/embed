/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : LED_ctl.c
* Author             : deh
* Date First Issued  : 10/21/2011
* Board              : STM32F103VxT6_pod_mm (USART1)
* Description        : Pulse external LED (use systick and polling)
*******************************************************************************/
/*
*/


/*
Subroutine call references shown as "@n"--
@1 = svn_pod/stm32/trunk/devices/Podpinconfig.h
@2 = svn_pod/stm32/trunk/lib/libmiscstm32/SYSTICK_getcount32.c

*/


/* The following are in 'svn_pod/stm32/trunk/lib' */

#include "p1_common.h"		// Variables in common

static char state;
static unsigned int uiLED_systick;	// Working count
static unsigned int uiLED_systickCt;	// Pulse ON width
static unsigned int uiLED_systickSpace;	// Pulse OFF width
static int nLED_systickNum;		// Number of pulses
/******************************************************************************
 * void LED_ctl_turnon(unsigned int uiCt, unsigned int uiSpace, int nNum);
 * @param	: uiCt: count of ON time (ms) (limit to about 40,000)
 * @param	: uiSpace: count of space time (ms) between pulses 
 * @param	: uiNum: number of pulses 
 * @brief 	: Configure pin for output, turn LED on, configure back to input
*******************************************************************************/
void LED_ctl_turnon(unsigned int uiCt, unsigned int uiSpace, int nNum)
{
	/* Set PA0 for output (since dual-use pin) */
	PA0_reconfig(0);	

	/* Save pulse width, space width, and number of pulses */
	uiLED_systickCt = uiCt;	
	uiLED_systickSpace = uiSpace;
	nLED_systickNum = nNum;

	state = 3;

	return;
}
/******************************************************************************
 * void LED_ctl(void);
 * @brief 	: Polling loop checks to see if time to turn LED off
*******************************************************************************/
void LED_ctl(void)
{
	switch (state)
	{
	case 0:	// Idle
		break;

	case 1:
		/* Wait for ON time to expire */
		/* NOTE: Systick counter counts DOWN (not up).  Get  time switch turned on, and computed the count
			that will assure at least the minimum delay required for voltages to build up.  */
		if (  ((int)((SYSTICK_getcount32() - uiLED_systick))) > 0 )//  Time expired? (@2)		
			return;			// No.
		
		/* Turn LED OFF */
		PA0_WKUP_low;	// Set PA0 low (@1)

		/* Have sent the requested number of pulses? */
		nLED_systickNum -= 1;
		if (nLED_systickNum > 0)
		{ // Here, there is more to do.
			uiLED_systick = SYSTICK_getcount32() - (uiLED_systickSpace*(sysclk_freq/1000));
			state = 2;
		}
		else
		{ // Here, it is time to quit
			/* Set PA0 for input (since dual-use pin) */
			PA0_reconfig(1);	// Reconfigure so pushbutton can be seen
			state = 0;	// Idle state for polling
		}

		break;
	
	case 2:	
		/* Wait (with LED OFF) for space time to expire */				
		if (  ((int)((SYSTICK_getcount32() - uiLED_systick))) > 0 )//  Time expired? (@2)		
			return;			// No.

	case 3:
		/* Turn LED ON */
		PA0_WKUP_hi;	// Set PA0 high (@1)

		/* Compute systick count when LED should turn off */
		uiLED_systick = SYSTICK_getcount32() - (uiLED_systickCt*(sysclk_freq/1000));

		state = 1;

		break;
	}
	return;
}


/******************************************************************************
 * void LEDonboard_ctl_turnon_ct(unsigned int uiCtob, unsigned int uiSpaceob, short sCt);
 * @param	: uiCobt: count of ON time (ms) (limit to about 40,000)
 * @param	: uiSpaceob: count of space time (ms) between pulses 
 * @param	: uiCt: Number of quick flashes (1 = one flash; 2 = two, etc.)
 * @brief 	: Flash onboard LED & LED on interface (not LED on BOX)
*******************************************************************************/
static unsigned short stateob;
static short sCount;
static unsigned int uiSpace;		
static unsigned int uiOn;
static unsigned int uiLEDob_systick;	// Working count for timing


void LEDonboard_ctl_turnon_ct(unsigned int uiCtob, unsigned int uiSpaceob, short sCt)
{	

	switch (stateob)
	{
	case 0:
		sCount = sCt;	// Save count of flashes
		uiSpace = uiSpaceob;
		uiOn = uiCtob;

	case 1:	// Initial state
		/* Compute systick count when LED should turn off */
		uiLEDob_systick = SYSTICK_getcount32() - (uiOn*(sysclk_freq/1000));

		/* Turn LED ON */
		LED4_on;	// Macro (@1)
		EXTERNALLED_on;	// PE2 high (FET driver for LED)

		stateob = 2;
		return;

	case 2: // Waiting to turn LED OFF

		/* LED is ON.  Wait for time to expire */
		if (  ((int)((SYSTICK_getcount32() - uiLEDob_systick))) > 0 )//  Time expired? (@2)		
			return;	// Return time not expired.

		/* Turn LED OFF */
		LED4_off;	// Macro (@1)
		EXTERNALLED_off;// PE2 low (FET driver for LED)

		/* Compute systick count when LED should turn ON */
		uiLEDob_systick = SYSTICK_getcount32() - (uiSpace*(sysclk_freq/1000));

		stateob = 3;
		break;

	case 3:	// LED is off, wait turn it back on
		if (  ((int)((SYSTICK_getcount32() - uiLEDob_systick))) > 0 )//  Time expired? (@2)		
			return;	// Return time not expired.

		sCount -= 1;
		if (sCount <= 0)
			stateob = 0;
		else
		{
			stateob = 1;	// Repeat flash
			uiOn = 10;	// Use shorter ON
			uiSpace = 250;	// User shorter OFF
		}
		break;
	}

	return;
}

