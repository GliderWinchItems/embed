/******************************************************************************
* File Name          : panic_leds.c
* Date First Issued  : 05/24/2013
* Board              : Discovery F4
* Description        : Panic flash LED's
*******************************************************************************/
/*
Flash LED's fast for the hard fault count, then pause, and repeat

NOTE--THIS IS BOARD-SPECIFIC.

*/ 
#include "DISCpinconfig.h"
#include "libopencm3/stm32/f4/rcc.h"

extern unsigned int	sysclk_freq;	/* 	SYSCLK freq		E.g. 72000000	*/
static void allon(void)
{
	LEDSALL_on;
	return;
}
static void alloff(void)
{
	LEDSALL_off;
	return;
}	
static void loop(volatile int ct)
{
	while (ct > 0) ct -= 1; 
	return;
}

/******************************************************************************
void panic_leds(unsigned int count);
 * @param	: Number of fast flashes
 * @brief	: Configure gpio pins for Discovery F4 board
 ******************************************************************************/
int PanicCount;

void panic_leds(unsigned int count)
{
	unsigned int i;
	volatile int x;		// Loop ct for timing pause
	volatile int xon;	// Loop ct for timing LED on duration
	volatile int xoff;	// Loop ct for timing LED off duration
	int sw = 0;
PanicCount = count; // Usefull for debugging this
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
	RCC_AHB1ENR |= 0x08;	// Enable IO port D
	DISCgpiopins_Config();	// Configure pins

	/* Now flash, flash, flash away, ye' rummies.  Batten the hatches, 'er there be trouble. */
	if (sw == 0)
	{
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
}
