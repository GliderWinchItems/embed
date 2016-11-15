/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : panic_leds.c
* Author             : deh
* Date First Issued  : 05/24/2013
* Board              : STM32F103RxT6 sensor board
* Description        : Panic flash LED's
*******************************************************************************/
/*
Flash LED's fast for the hard fault count, then pause, and repeat

NOTE--THIS IS BOARD-SPECIFIC.
NOTE--THIS IS BOARD-SPECIFIC.
NOTE--THIS IS BOARD-SPECIFIC.

*/ 
#include "../libopenstm32/rcc.h"
#include "libopenstm32/gpio.h"


extern unsigned int	sysclk_freq;	/* 	SYSCLK freq		E.g. 72000000	*/

static void loop(volatile int ct)
{
	while (ct > 0) ct -= 1; 
	return;
}

/******************************************************************************
void panic_leds_Olimex(unsigned int count);
 * @param	: Number of fast flashes
 * @brief	: Configure gpio pins
 ******************************************************************************/
#define LEDBIT	12	// Olimex board LED bit
void panic_leds_Olimex(unsigned int count)
{
	unsigned int i;
	volatile int x;		// Loop ct for timing pause
	volatile int xon;	// Loop ct for timing LED on duration
	volatile int xoff;	// Loop ct for timing LED off duration
	int sw = 0;

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
	if (count > 6)  count = 7; // Bogus count

	/* Be sure we have the ports and pins setup for the LEDs */
	RCC_APB2ENR |= 0x08;	// Enable IO ports C
		
	/* Setup GPIO pin for LED (PC12) (See Ref manual, page 157) */
	// 'CRH is high register for bits 8 - 15; bit 12 is therefore the 4th CNF/MODE position (hence 4*4)
	GPIO_CRH(GPIOC) &= ~((0x000f ) << (4*4));	// Clear CNF reset bit 01 = Floating input (reset state)
	GPIO_CRH(GPIOC) |=  (( (GPIO_CNF_OUTPUT_PUSHPULL<<2) | (GPIO_MODE_OUTPUT_2_MHZ) ) << (4*4));

	/* Now flash, flash, flash away, ye' rummies.  Batten the hatches, 'er there be trouble. */
	if (sw == 0)
	{
		while (1==1)
		{
			for (i = 0; i < count; i++)
			{
				GPIO_BRR(GPIOC) = (1<<LEDBIT);		loop(xon);
				GPIO_BSRR(GPIOC) = (1<<LEDBIT);		loop(xoff);			
			}
			loop(x);
		}
	}
}
