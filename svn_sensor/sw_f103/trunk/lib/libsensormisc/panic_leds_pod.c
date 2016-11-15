/******************************************************************************
* File Name          : panic_leds_pod.c
* Date First Issued  : 05/24/2013
* Board              : stm32F103VCT6 POD board
* Description        : Panic flash LED's
*******************************************************************************/
/*
Flash LED's fast for the hard fault count, then pause, and repeat

NOTE--THIS IS BOARD-SPECIFIC.
NOTE--THIS IS BOARD-SPECIFIC.
NOTE--THIS IS BOARD-SPECIFIC.

*/ 
#include "PODpinconfig.h"
#include "../libopenstm32/rcc.h"
#include "libopenstm32/scb.h"

/* ------- LED identification ----------- 
|-number on pc board below LEDs
| color   ADCx codewheel  bit number
3 green   ADC2   black    0   LED3
4 red	  ADC2   white    1   LED4
5 green   ADC1   black    0   LED5
6 yellow  ADC1   white    1   LED6
  --------------------------------------*/

extern unsigned int	sysclk_freq;	/* 	SYSCLK freq		E.g. 72000000	*/

static void allon(void)
{
	GPIO_BRR(GPIOE) = 0x38;
	return;
}
static void alloff(void)
{
	GPIO_BSRR(GPIOE) = 0x38;
	return;
}	
static void loop(volatile int ct)
{
	while (ct > 0) ct -= 1; 
	return;
}

/******************************************************************************
void panic_leds_pod(unsigned int count);
 * @param	: Number of fast flashes
 * @brief	: Configure gpio pins
 ******************************************************************************/
void panic_leds_pod(unsigned int count)
{
	unsigned int i;
	volatile int x;		// Loop ct for timing pause
	volatile int xon;	// Loop ct for timing LED on duration
	volatile int xoff;	// Loop ct for timing LED off duration

	/* Have clocks been setup? */
	if (sysclk_freq < 1000000)
	{ // Here, no.  We are running on the 8 MHz HSI oscillator
		sysclk_freq = 8000000;	// Set default
	}

	/* Setup timing for pause */
	x = sysclk_freq/10;

	/* Setup timing for fast flash */
	xon =  sysclk_freq/250;
	xoff = sysclk_freq/40;

	/* Limit max count */
	if (count == 0) count = 6; // Bogus count
	if (count > 6)  count = 7; // Bogus count

	/* Be sure we have the ports and pins setup for the LEDs */
	PODgpiopins_default();	// Set gpio port register bits for low power
	PODgpiopins_Config();	// Now, configure pins
	
	/* Now flash, flash, flash away, ye' rummies.  Batten the hatches, 'er there be trouble. */
	while (1==1)
	{
		for (i = 0; i < count; i++)
		{
			allon();	loop(xon);
			alloff();	loop(xoff);			
		}
		loop(x);
	}	
}
