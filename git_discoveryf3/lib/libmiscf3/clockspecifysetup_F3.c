/******************************************************************************
* File Name          : clockspecifysetup_F3.c
* Date First Issued  : 03/03/2012
* Description        : Modifications of 'clocksetup.c' to specify clock setup parameters
*******************************************************************************/
#include "../libopencm3/stm32/rcc.h"
#include "../libopencm3/stm32/gpio.h"
#include "../libopencm3/stm32/flash.h"
#include "clockspecifysetup_F3.h"

/*
Note: This routine may not work correctly for I2S2,3 and requires some
review for USB OTG.
*/

/*
Bus Tree--
SYSCLK:
  AHB1 (72 MHz)
    TSCEN
    GPIO
    CRC
    FLITF
    SRAM
    DMA1,2   
    APB1: (max 36 MHz)
      IWDG
      TIM 2-7,12-14,18
      USART 2,3
      UART  4,5
      SPI 2,3
      CAN 1
      I2C 1,2
      DAC 1,2
      PWR
      BKP
      IWDG
      WWDG (window watch dog)
      USB OTG
      RTC
    APB2: (max 72 MHz)
      SDIO
      GPIOA-G
      TIM 15-17,19
      ADC1 (max 36 MHz, Typical 30 MHz)  
      SDADC 1-3
      USART1
      SPI1
      EXTI
      WKUP
      AFIO
      COMP 1,2/SYSCFG CTL
      
*/


/* The following variables are used by peripherals in their 'init' routines to set dividers 	*/
extern unsigned int	hclk_freq;	/* 	SYSCLKX/HPREDIV	 	E.g. 72000000 	*/
extern unsigned int	pclk1_freq;	/*	SYSCLKX/PCLK1DIV	E.g. 72000000 	*/
extern unsigned int	pclk2_freq;	/*	SYSCLKX/PCLK2DIV	E.g. 36000000 	*/
extern unsigned int	sysclk_freq;	/* 	SYSCLK freq		E.g. 72000000	*/

/* Tables for looking up division and multiplication corresponding to the register code */
/*                                     0 1 2 3 4 5 6 7 8  9  10 11 12  13  14  15 */	// Code in clocks struct
const unsigned short ahbtbl[16]     = {1,1,1,1,1,1,1,1,2, 4,  8,16,64,128,256,512};	// AHB PRESCALE code versus ahb division
const unsigned char apbtbl[8]	    = {1,1,1,1,2,4,8,16};	// APBx prescale code versus divisor

/******************************************************************************
 * void clockspecifysetup_F3(struct CLOCKSF3 *clocks)
 * @brief	: Setup the clocks & bus freq dividers
 * @param	: chase: 0 = use HSI (internal 8 MHz RC osc), 1 = use HSE (external Xtal osc)
 * @return	: static variables are set from input paramaters passed in struct
 ******************************************************************************/
void clockspecifysetup_F3(struct CLOCKSF3 *clocks)
{
	unsigned short usSW = 0;		// Oscillator selection code (default = HSI)
	unsigned short ws = 0;			// Wait states (default for 16 MHz HSI)
	unsigned short sw_pll = 0;		// 0 = pll not used; 1 = pll used
	unsigned int pllpre_x;			// Adjust pllpre value to register code
	unsigned int ahb_x;
	unsigned int apb1_x;
	unsigned int apb2_x;
	signed short pllm_x;			// Adjust pllm value to register code	

	/* Note: Out of reset, the HSI (internal rc osc) is selected and ready. */

	/* Setup oscillator that will be used for system. */
	switch (clocks->hso)	// Select high speed oscillator source
	{
	case HSOSELECT_HSI:	// Use internal 8 Mhz rc osc
		/* No action needed needed since HSI is already running and ready.  
			This osc as system clock is what got us to this point! */
		sysclk_freq = 8000000;				// The internal RC osc is always 8MHz
		break;
	case HSOSELECT_HSE_XTAL: // Use external xtal controlled osc
		RCC_CR = RCC_CR_HSEON;			// Start hi-speed external oscillator
		while ((RCC_CR & (RCC_CR_HSERDY)) == 0);// Wait to become ready
		usSW = 0x01;				// SWS = HSE as system clock
//RCC_CR &= ~RCC_CR_HSION;	// Turn off HSI osc
		break;
	case HSOSELECT_HSE_INPUT: // Use external signal input
		RCC_CR = RCC_CR_HSEBYP;			// By-pass external osc and use external signal
		RCC_CR = RCC_CR_HSEON;			// Start hi-speed external clock
		while ((RCC_CR & (RCC_CR_HSERDY)) == 0);// Wait to become ready
		usSW = 0x01;				// SWS = HSE as system clock
		break;
//	case HSOSELECT_HSE_XTAL_REMAPPED: // Use remapped external osc *xtal*
//$$$		AFIO_MAPR |= (1 << 15); 		// Bit 15 PD01_REMAP: Port D0/Port D1 mapping on OSC_IN/OSC_OUT		
//		RCC_CR = RCC_CR_HSEON;			// Start oscillators hi-speed internal and external
//		while ((RCC_CR & (RCC_CR_HSERDY)) == 0);// Wait to become ready
//		usSW = 0x01;				// SWS = HSE as system clock
//		break;
//	case HSOSELECT_HSE_XTAL_INPUT: // Use remapped external osc *signal*
//$$$		AFIO_MAPR |= (1 << 15); 		// Bit 15 PD01_REMAP: Port D0/Port D1 mapping on OSC_IN/OSC_OUT		
//		RCC_CR = RCC_CR_HSEBYP;			// By-pass external osc and use external signal
//		while ((RCC_CR & (RCC_CR_HSERDY)) == 0);// Wait to become ready
//		usSW = 0x01;				// SWS = HSE as system clock
//		break;
	default: // Oh oh. We are so screwed!
		while (1==1);
	}

	/* APB1 divider code */
	apb1_x = apbtbl[(clocks->apb1 & 0x7)];
	RCC_CFGR |= (clocks->apb1 & 0x7) << 8;

	/* APB2 divider code */
	apb2_x = apbtbl[(clocks->apb2 & 0x7)];
	RCC_CFGR |= (clocks->apb2 & 0x7) << 11;

	/* Note: pllm multiplier should be: 2x - 16x. */
	if ( clocks->pllm > 1 ) // Is PLL multiplier specifed?
	{ // Here, yes the PLL will be used
		pllm_x = clocks->pllm;
		if (pllm_x > 16) pllm_x = 16;
		pllm_x -= 2;	// pll register code: 0 to 14 for 2x to 16x multiplier

		/* System clock freq, given predivider and pll multiplier. */
		pllpre_x = clocks->pllpre & 0xf;

		/* Note: Even though freq and pllm_x are designated as integers the division
                   is a "real frequency" and could be fractional; therefore, multiply before
                   the division, otherwise the computed sysclk_freq and the actual freq might
                   be different (and even then it could result in a fraction of a Hz difference. */
		sysclk_freq = (clocks->freq * clocks->pllm)/(clocks->pllpre+1) ;

		/* Set PLL predivider code */

		/* Set PLL source (HSI or HSE) */
		RCC_CFGR |= (clocks->pllsrc & 0x1) << 16;

		/* Pre-division code (0 = not divided, 1 = divide by 2,...15 = divide by 16) */
		RCC_CFGR2 = pllpre_x;

		/* Don't blast ahead if the sysclk freq is too high */
		if (sysclk_freq > 72000000) while(1==1);  // Screwed!

		/* Set the PLL code. */
		RCC_CFGR |= ( (pllm_x & 0xf) << 18); // Set PLL multiplier

		sw_pll = 1;	// Show PLL in use
	}

	/* Determine number of flash wait states. */
/* 
Bits 1:0 LATENCY[2:0]: Latency
These bits represent the ratio of the SYSCLK (system clock) period to the Flash
access time.
000: Zero wait state, if 0 < SYSCLK ≤ 24 MHz
001: One wait state, if 24 MHz < SYSCLK ≤ 48 MHz
010: Two wait sates, if 48 < SYSCLK ≤72 MHz
*/
	ws = 0;
	if (sysclk_freq >= 24000000) ws = 1;
	if (sysclk_freq >= 48000000) ws = 2;

	/* Set wait states in register, plus prefetch buffer enable. */
	FLASH_ACR |= (ws | (1<<4));

	/* AHB divider code */
	/* prefetch should not be switched when AHB divider not same as system clk,
	   so set the AHB divider after setting the prefetch buff enable bit. */
	ahb_x = ahbtbl[(clocks->ahbdiv & 0xf)];	// Get divisor from code
	RCC_CFGR |= ((clocks->ahbdiv & 0xf) << 4);

	// The following two waits may not be needed on the F3
	/* Wait for register to become effective. */
	while ((FLASH_ACR & 0x07) != ws);

	/* Wait for prefetch to show enabled. */
	while ((FLASH_ACR & (1<<5)) == 0);

	if (sw_pll == 1)	// Are we using the PLL?
	{ // Here yes, start PLL
		RCC_CR |= (1<<24);			// PLLON:  Turn PLL ON
		usSW = 0x02;	// Change the usSW code for selecting PLL
		while ((RCC_CR & (1<<25) ) == 0);	// PLLRDY: Wait for PLL to become ready
	}

	/* Switch the system clock source to the one specified */
	RCC_CFGR = (RCC_CFGR & ~0x03) | usSW;		// Set system clock source (it has been running on HSI (internal 8 MHz rc clock)

	/* Compute static variables with bus freqs that other routines will use to for their setup */
	hclk_freq   = sysclk_freq/ahb_x;	// AHB bus freq = sysclck/ ahb bus divider
	pclk1_freq  =   hclk_freq/apb1_x;	// APB1 driven from AHB (must not exceed 36 MHz)
	pclk2_freq  =   hclk_freq/apb2_x;	// APB2 driven from AHB

	/* Wait until system is running on the selected clock source */
	while ( (RCC_CFGR & 0x0c) != (unsigned short)(usSW << 2) );	

 	return;                                           
}

