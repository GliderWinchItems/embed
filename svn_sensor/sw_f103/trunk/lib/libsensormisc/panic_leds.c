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
#include "SENSORpinconfig.h"
#include "../libopenstm32/rcc.h"
#include "libopenstm32/scb.h"

extern unsigned int	sysclk_freq;	/* 	SYSCLK freq		E.g. 72000000	*/
static void allon(void)
{
	LED21GREEN_on;
	LED19RED_on;
	LED20RED_on;
	return;
}
static void alloff(void)
{
	LED21GREEN_off;
	LED19RED_off;
	LED20RED_off;
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
 * @brief	: Configure gpio pins
 ******************************************************************************/
void panic_leds(unsigned int count)
{
	unsigned int i;
	volatile int x;		// Loop ct for timing pause
	volatile int xon;	// Loop ct for timing LED on duration
	volatile int xoff;	// Loop ct for timing LED off duration
	int sw = 0;
	int ledct = 0;

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
	RCC_APB2ENR |= 0x18;	// Enable IO ports C & D		
	SENSORgpiopins_Config();
	
	unsigned int rst = 0;
	
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
		rst += 1; if (rst >= 10)
			SCB_AIRCR = (0x5FA << 16) | SCB_AIRCR_SYSRESETREQ;	// Cause a RESET
	}
	
	
	while (1==1)
	{
		for (i = 0; i < count; i++)
		{
	
		switch (ledct)
			{
			case 0: LED19RED_off;		LED20RED_off;		LED21GREEN_on;		loop(xon);	LED21GREEN_off;	loop(xoff);	break;
			case 1: LED19RED_on;		LED20RED_off;		LED21GREEN_off;		loop(xon);	LED19RED_off;	loop(xoff);	break;
			case 2: LED19RED_off;		LED20RED_on;		LED21GREEN_off;		loop(xon);	LED20RED_off;	loop(xoff);	break;
			default: ledct = 0; break;
			}
			
			ledct += 1;		// Step through all four LEDs
			if (ledct > 2) ledct = 0;
		}
		loop(x);
	}

}
