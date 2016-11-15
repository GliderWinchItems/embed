/******************************************************************************
* File Name          : ledcontrol.c
* Date First Issued  : 05/11/2014
* Board              : Sensor
* Description        : Flashing the leds
*******************************************************************************/

#include "gpio.h"
#include "Tim9.h"
#include "SENSORpinconfig.h"


static void REDled_left(void);
static void REDled_right(void);

// Right RED LED on sensor board, ties to LED RED/GREEN pair on flashgenie module
struct COUNTDOWNTIMER RightED = {\
	0,    /* u32	LeftLED.ctr  = 0;			 Countdown counter */ \
	0,    /* u32	LeftLED.flag = 0;			 Flag != 0--count time hit zero */\
	200,  /* u32	LeftLED.rep  = 200;			 Countdown repetition (1/2 ms ticks) */ \
	0,    /* u32	LeftLED.lod  = 0;			 Load value */ \
	(void*)REDled_right,    /* void 	LeftLED.(*func)(void) =(void*)REDled_left;	 Callback pointer */ \
	0,    /* struct COUNTDOWNTIMER*	LeftLED.next = 0;	 Pointer to next struct */ \
};





static int lred_m; // Mode
static int lred_t; // Rep rate count
/* **************************************************************************************
 * Callbacks for countdown timers
 * ************************************************************************************** */

static void REDled_right(void)
{
	return;
}
/******************************************************************************
 * void GRNsetmode(int m, int t);
 * @brief 	: Set mode and time for GREEN led
 * @param	: m: 0 = off, 1 = on, 2 = flash with time period t (ms)
 * @param	: t = time ticks in ms
*******************************************************************************/
void LeftREDsetmode(int m, int t)
{ // Save where it gets picked up on the next timeout
	lred_t = t; lred_m = m; return;
}
/******************************************************************************
 * static void Leftled(struct COUNTDOWNTIMER* p);
 * @brief 	: Callback for left RED led
*******************************************************************************/
static struct COUNTDOWNTIMER LeftLED;
static void REDled_left(void)
{
//if ((GPIO_ODR(GPIOC) & (1<<5)) == 0)
//{LED20RED_on;}	// Set bit
//else
//{LED20RED_off;}	// Reset bit
//return;
	switch (lred_m)
	{
 	case 0:	// LED off 
		LED20RED_off;	// Reset bit
		LeftLED.rep = 100;	// Rep count to keep it alive
		break;	
	case 1:	// LED on
		LED20RED_on;	// Set bit
		LeftLED.rep = 100;	// Rep count to keep it alive
		break;	
	case 2: // LED flashing with 't' ms period
		LeftLED.rep = lred_t;	// Update rep rate (if changed)
		if ((GPIO_ODR(GPIOC) & (1<<5)) == 0)
			{LED20RED_on;}	// Set bit
		else
			{LED20RED_off;}	// Reset bit
		break;
	}
	return;
}

/* **************************************************************************************
 * void LEDsetup(void);
 * @brief	:Add structs for coundown timing
 * ************************************************************************************** */


void LEDsetup(void)
{
//	countdowntimer_add(&SD_sw);
// Left RED LED on sensor board
LeftLED.ctr  = 0;		// Countdown counter
LeftLED.flag = 0; 		// Flag != 0--count time hit zero
LeftLED.rep  = 500;		//	 Countdown repetition (1/2 ms ticks)
LeftLED.lod  = 0;		// Load value
LeftLED.func = (void*)&REDled_left;	// Callback pointer
LeftLED.pnext = 0;		// Pointer to next struct 

	countdowntimer_add(&LeftLED);
	return;
}

