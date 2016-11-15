/******************************************************************************
* File Name          : panic_ledsDf3.c
* Date First Issued  : 01/27/2015
* Board              : Discovery F3
* Description        : Panic flash LED's
*******************************************************************************/
/*
Flash LED's fast for the hard fault count, then pause, and repeat

NOTE--THIS IS BOARD-SPECIFIC.

*/ 
#include "../libopencm3/stm32/rcc.h"
#include "f3Discovery_led_pinconfig.h"

extern unsigned int sysclk_freq;	/* 	SYSCLK freq		E.g. 72000000	*/

static void loop(volatile int ct)
{
	while (ct > 0) ct -= 1; 
	return;
}

/******************************************************************************
void panic_ledsDf3(unsigned int count);
 * @brief	: Configure gpio pins for Discovery F4 board
 * @param	: Number of fast flashes
 ******************************************************************************/
int PanicCount;

void panic_ledsDf3(unsigned int count)
{
	unsigned int i;
	volatile int x;		// Loop ct for timing pause
	volatile int xon;	// Loop ct for timing LED on duration
	volatile int xoff;	// Loop ct for timing LED off duration
	int sw = 0;
PanicCount = count; // Useful for debugging this
	/* Have clocks been setup? */
	if (sysclk_freq < 1000000)
	{ // Here, no.  We are running on the 8 MHz HSI oscillator
		sysclk_freq = 8000000;	// Set default
		sw = 8000000;	// Not zero
	}

	/* Setup timing for pause */
	x = sysclk_freq/10;

	/* Setup timing for fast flash */
	xon =  sysclk_freq/250;
	xoff = sysclk_freq/40;

	/* Limit max count */
	if (count == 0) count = 6; // Bogus count

	/* Be sure we have the ports and pins setup for the LEDs */
	f3Discovery_led_pinconfig_init();	// Configure pins

	/* Now flash, flash, flash away, ye' rummies.  Batten the hatches, 'er there be trouble. */
	if (sw == 0)
	{
		while (1==1)
		{
			for (i = 0; i < count; i++)
			{
				DF3LEDSALL_on;	loop(xon);
				DF3LEDSALL_off;	loop(xoff);			
			}
			loop(x);
		}
	}
}
