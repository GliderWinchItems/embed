/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : clockspecifysetup.c
* Hackor             : deh
* Date First Issued  : 06/14/2011
* Description        : Modifications of 'clocksetup.c' to specify clock setup parameters
*******************************************************************************/
#include "clockspecifysetup.h"
#include "../libopenstm32/rcc.h"
#include "../libopenstm32/flash.h"
#include "../libopenstm32/gpio.h"
/*
Note: This routine may not work correctly for I2S2,3 and requires some
review for USB OTG.
*/

/*
Peripherals driven by bus clocks--
SYSCLK:
  IS2,3SCLK	
  AHB: (this is driven by SYSCLK)
    DMA1,2   
    SDIO        	
    APB1: (max 36 MHz)
      TIM 2-7 (if APB1 prescale = 0, 1x; else x2)
      USART4,5
      USART2,3
      SPI2,3
      CAN1,2
      I2C1,2
      DAC
      PWR
      BKP
      IWDG
      WWDG (window watch dog)
      USB OTG
      RTC
    APB2: 
      GPIOA-G
      TIM 1,8 (if APB2 prescale = 0, 1x; else x2)
      ADC1-3 (max 14 MHz)      
      USART1
      SPI1
      EXTI
      AFIO

(USB prescaler driven directly from PLLCLK)
*/


/* The following variables are used by peripherals in their 'init' routines to set dividers 	*/
extern unsigned int	hclk_freq;	/* 	SYSCLKX/HPREDIV	 	E.g. 72000000 	*/
extern unsigned int	pclk1_freq;	/*	SYSCLKX/PCLK1DIV	E.g. 72000000 	*/
extern unsigned int	pclk2_freq;	/*	SYSCLKX/PCLK2DIV	E.g. 36000000 	*/
extern unsigned int	sysclk_freq;	/* 	SYSCLK freq		E.g. 72000000	*/

unsigned short pmul1,pmul2,pdev1,pdev2;	// Debugging

unsigned int convert(unsigned int ucC);

/* Tables for looking up PLL multiplication corresponding to the code */

/*  not allowed codes                                                           */	
/*                                     0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15  */	// Code in clocks struct p 93
const unsigned char  pllmulxtbl[16] = {2, 2, 8,10,12,14,16,18,20,22,24,26,28,30,32,32}; 	// PLLMUL  code versus multiplication x 2

/*  not allowed codes                  v v v v v v v                  	  	*/	
/*                                     0 1 2 3 4 5 6 7 8 9 10 11 12  13  14  15 */	// Code in clocks struct
const unsigned short ahbtbl[16]     = {1,1,1,1,1,1,1,1,2,4, 8,16,64,128,256,512};	// AHB PRESCALE code versus ahb division


/* CONECTIVITY LINE */
///*  not allowed codes                    v                   v  v  v  v  v	*/	
///*                                     0 1 2  3  4  5  6  7  8  9 10 11 12 13   */	// Code in clocks struct p 124
//onst unsigned char  pllmulxtbl[14] = {2,1,8,10,12,14,16,18, 1, 1, 1, 1, 1,13}; 	// PLLMUL  code versus multiplication x 2

/*  not allowed codes                    v v v v v                     v 	*/	
/*                                     0 1 2 3 4 5 6 7  8  9 10 11 12 13 14 15  */	// Code in clocks struct
//const unsigned char pllmulx2tbl[16] = {1,1,1,1,1,1,8,9,10,11,12,13,14, 1,16,20};	// PLLMUL2 code versus multiplication x 1



/******************************************************************************
 * void clockspecifysetup(struct CLOCKS *clocks)
 * @brief	: Setup the clocks & bus freq dividers
 * @param	: chase: 0 = use HSI (internal 8 MHz RC osc), 1 = use HSE (external Xtal osc)
 * @return	: static variables are set from input paramaters passed in struct
 ******************************************************************************/
void clockspecifysetup(struct CLOCKS *clocks)
{
	unsigned short usSW;

	RCC_CFGR = 0; 	// JIC we entered with clocks all ready set 

	/* See Ref manual page 84 (115 for connecitivity line) for diagram of clocks, PLLs, etc. */
	/* Note: Out of reset, the HSI (internal rc osc) is selected and ready. */
	/* Clock control register (Ref manual, page 121)	*/
	switch (clocks->hso)	// Select high speed oscillator source
	{
	case 0:	// Use internal 8 Mhz rc osc
		/* No action needed needed since HSI is already running and ready.  
			This osc as system clock is what got us to this point! */
		usSW = 0x00;				// SWS = HSI as system clock
		break;
	case 1: // Use external xtal controlled osc
		RCC_CR = RCC_CR_HSEON;			// Start hi-speed external oscillator
		while ((RCC_CR & (RCC_CR_HSERDY)) == 0);// Wait to become ready
		usSW = 0x01;				// SWS = HSE as system clock
//RCC_CR &= ~RCC_CR_HSION;	// Turn off HSI osc
		break;
	case 2: // Use external signal input
		RCC_CR = RCC_CR_HSEBYP;			// By-pass external osc and use external signal
		RCC_CR = RCC_CR_HSEON;			// Start hi-speed external clock
		while ((RCC_CR & (RCC_CR_HSERDY)) == 0);// Wait to become ready
		usSW = 0x01;				// SWS = HSE as system clock
		break;
	case 3: // Use remapped external osc xtal
		AFIO_MAPR |= (1 << 15); 		// Bit 15 PD01_REMAP: Port D0/Port D1 mapping on OSC_IN/OSC_OUT		
		RCC_CR = RCC_CR_HSEON;			// Start oscillators hi-speed internal and external
		while ((RCC_CR & (RCC_CR_HSERDY)) == 0);// Wait to become ready
		usSW = 0x01;				// SWS = HSE as system clock
		break;
	case 4: // Use remapped external osc signal
		AFIO_MAPR |= (1 << 15); 		// Bit 15 PD01_REMAP: Port D0/Port D1 mapping on OSC_IN/OSC_OUT		
		RCC_CR = RCC_CR_HSEBYP;			// By-pass external osc and use external signal
		RCC_CR = RCC_CR_HSEON;			// Start hi-speed external clock
		while ((RCC_CR & (RCC_CR_HSERDY)) == 0);// Wait to become ready
		usSW = 0x01;				// SWS = HSE as system clock
		break;
	default: // Oh oh. We are screwed!
		while (1==1);
	}

	/* Compute the SYSCLK freq */
	sysclk_freq = (clocks->freq *  pllmulxtbl[clocks->pllmul] ) / /* Numerator = multiplies of freq */\
		      (2 * (clocks->pllxtpre + 1) );	// Note: '2 *' is because 6.5x comes in as 13
	if ( (clocks->hso == 0) && (clocks->pllmul > 0) )	
		sysclk_freq /= 2;	/* PLL with the HSI source gets an extra divide by 2 */
	

	/* Flash access control register (Ref manual, page 54) */
	// Determine flash wait states based on SYSCLKX frequency
	// Less 0 - 24 MHz use default of 0 wait 
	if (sysclk_freq > 24000000)		// 0-24 MHz
	{
		if (sysclk_freq <= 48000000)	// 24-48MHz
			FLASH_ACR = 0x31;	// One wait + defaults
		else
			FLASH_ACR = 0x32;	// 48-72MHz, two waits + defaults
	}

	/* Clock configuraton register (Ref manual, page 123)	*/
	// Setup up bus dividers
	RCC_CFGR = (clocks->apb2  << 11)  |	/* ABP2 prescaler bit code (max of 36 MHz)	*/ 
		   (clocks->apb1  <<  8)  |	/* APB1 prescaler bit code			*/ 
		   (clocks->ahb   <<  4)  ;	/* AHB  prescaler bit code			*/	

	/* PLL(s) setup, if used */
	if (clocks->pllmul > 0)	// If PLL is zero, then that 
	{	// See page 84, 93. Ref Manual
		RCC_CFGR |=     (clocks->pllmul   << 18)  | /* Multiplier code 	(0-15)		*/ 
				(clocks->pllxtpre << 17)  | /* HSE divider for PL source 	*/
			        (clocks->pllsrc   << 16)  ; /* PLL source: HSI or HSE	 	*/

		// Enable PLL
		RCC_CR |= (1<<24);			// PLLON:  Turn PLL on
		while ((RCC_CR & (1<<25) ) == 0);	// PLLRDY: Wait for PLL to become ready

		// Select PLL as system clock source 
		usSW = 0x02;				// SWS = PLL for system clock
	}

	/* Switch the system clock source to the one specified */
	RCC_CFGR = (RCC_CFGR & ~0x03) | usSW;		// Set system clock source (it has been running on HSI (internal 8 MHz rc clock)
	while ( (RCC_CFGR & 0x0c) != (unsigned short)(usSW << 2) );	// Wait until system is running on the clock source

	/* Enable all present GPIOx clocks.  These will be needed to turn on power regulators & switches (see p 103) */
	RCC_APB2ENR |= 0x7c;	// Enable IO ports A thru E

	/* Compute static variables that other routines will use to for their setup */
	hclk_freq   = sysclk_freq/ahbtbl[(clocks->ahb)];	// AHB bus freq = sysclck/ ahb bus divider
	pclk1_freq  =   hclk_freq/convert(clocks->apb1);	// APB1 driven from AHB (must not exceed 36 MHz)
	pclk2_freq  =   hclk_freq/convert(clocks->apb2);	// APB2 driven from AHB
 	return;                                           
}
/* Convert 0,4,5,6,7 input to 1,2,4,8,16 */
unsigned int convert(unsigned int ucX)
{	
	return (ucX == 0) ? 1 :(1 << (ucX-3) );
//	if (ucX == 0) return 1;
//	return (1 << (ucX-3));	
}

