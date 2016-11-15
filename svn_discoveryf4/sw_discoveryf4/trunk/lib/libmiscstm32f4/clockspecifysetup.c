/******************************************************************************
* File Name          : clockspecifysetup.c
* Date First Issued  : 03/03/2012
* Description        : Modifications of 'clocksetup.c' to specify clock setup parameters
*******************************************************************************/
#include "../libopencm3/stm32/f4/rcc.h"
#include "../libopencm3/stm32/f4/gpio.h"
#include "../libopencm3/stm32/f4/flash.h"
#include "clockspecifysetup.h"

/*
Note: This routine may not work correctly for I2S2,3 and requires some
review for USB OTG.
*/

/*
Peripherals driven by bus clocks--
SYSCLK:
  AHB1
    DMA1,2   
    SDIO        	
    APB1: (max 42 MHz)
      TIM 2-7,12-14
      USART 2,3
      UART  4,5
      SPI 2,3
      CAN 1,2
      I2C 1,2
      DAC 1,2
      PWR
      BKP
      IWDG
      WWDG (window watch dog)
      USB OTG
      RTC
    APB2: (max 84 MHz)
      WKUP
      SDIO
      GPIOA-G
      TIM 1,8-11
      ADC1-3 (max 36 MHz, Typical 30 MHz)      
      USART1,6
      SPI1
      EXTI
      AFIO
  AHB2
    USB OTG
    Camera
    RNG

(USB prescaler driven directly from PLLCLK)
*/


/* The following variables are used by peripherals in their 'init' routines to set dividers 	*/
extern unsigned int	hclk_freq;	/* 	SYSCLKX/HPREDIV	 	E.g. 72000000 	*/
extern unsigned int	pclk1_freq;	/*	SYSCLKX/PCLK1DIV	E.g. 72000000 	*/
extern unsigned int	pclk2_freq;	/*	SYSCLKX/PCLK2DIV	E.g. 36000000 	*/
extern unsigned int	sysclk_freq;	/* 	SYSCLK freq		E.g. 72000000	*/

unsigned int convert(unsigned int ucC);

/* Tables for looking up PLL multiplication corresponding to the code */


/*  not allowed codes                  v v v v v v v                  	  	*/	
/*                                     0 1 2 3 4 5 6 7 8 9 10 11 12  13  14  15 */	// Code in clocks struct
const unsigned short ahbtbl[16]     = {1,1,1,1,1,1,1,1,2,4, 8,16,64,128,256,512};	// AHB PRESCALE code versus ahb division


/******************************************************************************
 * void clockspecifysetup(struct CLOCKS *clocks)
 * @brief	: Setup the clocks & bus freq dividers
 * @param	: chase: 0 = use HSI (internal 8 MHz RC osc), 1 = use HSE (external Xtal osc)
 * @return	: static variables are set from input paramaters passed in struct
 ******************************************************************************/
void clockspecifysetup(struct CLOCKS *clocks)
{
	unsigned short usSW = 0;		// Oscillator selection code (default = HSI)
	unsigned short ws = 0;			// Wait states (default for 16 MHz HSI)
	unsigned short sw_pll = 0;		// 0 = pll not used; 1 = pll used
	unsigned short pllp = clocks->pllp;	// Likely the clocks struct was a "const" and we may have to override the value given
	unsigned short pllm = clocks->pllm;
	unsigned short plln = clocks->plln;
	unsigned short pllq = clocks->pllq;
	unsigned int	freq = clocks->freq;	
	unsigned int	vco_freq;		// Main VCO freq

	/* See Ref manual page 85 figure 9 */
	/* Note: Out of reset, the HSI (internal rc osc) is selected and ready. */


	switch (clocks->hso)	// Select high speed oscillator source
	{
	case 0:	// Use internal 16 Mhz rc osc
		/* No action needed needed since HSI is already running and ready.  
			This osc as system clock is what got us to this point! */
		freq = 16000000;				// The internal RC osc is always 16MHz
		break;
	case 1: // Use external xtal controlled osc
		RCC_CR = RCC_CR_HSEON;			// Start hi-speed external oscillator
		while ((RCC_CR & (RCC_CR_HSERDY)) == 0);// Wait to become ready
		usSW = 0x01;				// SWS = HSE as system clock
RCC_CR &= ~RCC_CR_HSION;	// Turn off HSI osc
		break;
	case 2: // Use external signal input
		RCC_CR = RCC_CR_HSEBYP;			// By-pass external osc and use external signal
		RCC_CR = RCC_CR_HSEON;			// Start hi-speed external clock
		while ((RCC_CR & (RCC_CR_HSERDY)) == 0);// Wait to become ready
		usSW = 0x01;				// SWS = HSE as system clock
		break;
	case 3: // Use remapped external osc *xtal*
//$$$		AFIO_MAPR |= (1 << 15); 		// Bit 15 PD01_REMAP: Port D0/Port D1 mapping on OSC_IN/OSC_OUT		
		RCC_CR = RCC_CR_HSEON;			// Start oscillators hi-speed internal and external
		while ((RCC_CR & (RCC_CR_HSERDY)) == 0);// Wait to become ready
		usSW = 0x01;				// SWS = HSE as system clock
		break;
	case 4: // Use remapped external osc *signal*
//$$$		AFIO_MAPR |= (1 << 15); 		// Bit 15 PD01_REMAP: Port D0/Port D1 mapping on OSC_IN/OSC_OUT		
		RCC_CR = RCC_CR_HSEBYP;			// By-pass external osc and use external signal
		while ((RCC_CR & (RCC_CR_HSERDY)) == 0);// Wait to become ready
		usSW = 0x01;				// SWS = HSE as system clock
		break;
	default: // Oh oh. We are so screwed!
		while (1==1);
	}

/* p 95
This register is used to configure the PLL clock outputs according to the formulas:
●    f(VCO clock) = f(PLL clock input) × (PLLN / PLLM)
●    f(PLL general clock output) = f(VCO clock) / PLLP
●    f(USB OTG FS, SDIO, RNG clock output) = f(VCO clock) / PLLQ
*/
	if ( (plln != 0) && (pllm != 0) ) // Is PLL multiplier/divider specifed?
	{ // Here, yes the PLL is to be used

		/* Make sure pllp is valid */
		if ((clocks->pllp >> 1) == 0 ) pllp = 2;
		if ((clocks->pllp >> 1) >  4 ) pllp = 8;

		/* Make sure plln, pllm is within range */
		if (plln > 432 ) while (1==1);
		if (plln <   2 ) while (1==1);
		if (pllm >  64 ) while (1==1);
		if (pllm <   2 ) while (1==1);	

		/* Divider for 48 MHz peripherals */
		if ((pllq == 0) || (pllq > 15)) pllq = 15;	

		vco_freq = (freq * clocks->plln) / clocks->pllm; 	// VCO freq

		/* Don't blast ahead if the VCO freq is out of range */
		if (vco_freq > 432000000) while(1==1); 	// SCREWED
		if (vco_freq < 64) 	  while(1==1); 	// SCREWED
		
		sysclk_freq = vco_freq / pllp;

		/* Don't blast ahead if the sysclk freq is too high */
		if (sysclk_freq > 168000000)	while(1==1); // SCREWED

		/* Setup PLL counts */
		RCC_PLLCFGR = (pllq << 24) | ( (clocks->pllsrc & 0x01) << 22) | ( pllp << 16) | ( (plln & 0x01ff) << 6) | (pllm);

		sw_pll = 1;
	}

	/* Determine number of flash wait states p 55.  2.7-3.6v ONLY */
	if (sysclk_freq >  30) ws = 1;
	if (sysclk_freq >  60) ws = 2;
	if (sysclk_freq >  90) ws = 3;
	if (sysclk_freq > 120) ws = 4;
	if (sysclk_freq > 150) ws = 5;

	/* Set wait states in register */
	FLASH_ACR |= ws;

	/* Wait for register to become effective p 55 */
	while ((FLASH_ACR & 0x07) != ws);


	/* Clock configuraton register (Ref manual, page 97)	*/
	// Setup up bus dividers
	RCC_CFGR = (clocks->apb2  << 13)  |	/* (3b) PPRE2: ABP2 prescaler bit code	(4-7)		*/ 
		   (clocks->apb1  << 10)  |	/* (3b) PPRE1: APB1 prescaler bit code	(4-7)		*/ 
		   (clocks->ahb   <<  4)  ;	/* (4b) HPRE:  AHB  prescaler bit code	(8-15)		*/


	if (sw_pll == 1)	// Are we using the PLL?
	{ // Here yes, start PLL
		// Enable PLL
		RCC_CR |= (1<<24);			// PLLON:  Turn PLL on
		while ((RCC_CR & (1<<25) ) == 0);	// PLLRDY: Wait for PLL to become ready

		// Select PLL as system clock source 
		usSW = 0x02;				// Change the usSW code for PLL
	}

	

	/* Switch the system clock source to the one specified */
	RCC_CFGR = (RCC_CFGR & ~0x03) | usSW;		// Set system clock source (it has been running on HSI (internal 8 MHz rc clock)
	while ( (RCC_CFGR & 0x0c) != (unsigned short)(usSW << 2) );	// Wait until system is running on the clock source


	/* Compute static variables with bus freqs that other routines will use to for their setup */
	hclk_freq   = sysclk_freq/ahbtbl[(clocks->ahb)];	// AHB bus freq = sysclck/ ahb bus divider
	pclk1_freq  =   hclk_freq/convert(clocks->apb1);	// APB1 driven from AHB (must not exceed 36 MHz)
	pclk2_freq  =   hclk_freq/convert(clocks->apb2);	// APB2 driven from AHB
 	return;                                           
}
/* Convert 0,4,5,6,7 input to 1,2,4,8,16 */
unsigned int convert(unsigned int ucX)
{	
	return (ucX == 0)?:(1 << (ucX-3) );
}
/******************************************************************************
 * int clockI2Ssetup(unsigned int R, unsigned int N);
 * @brief	: Setup PLL for I2S: f(USB OTG FS, SDIO, RNG clock output) = f(VCO clock) / PLLQ
 * @param	: R = valid range: 2-7 (3 bits)
 * @param	: N = valid range: 192-432 (9 bits). 
 * @return	: 0 = OK; 1 = failed
 ******************************************************************************/
int clockI2Ssetup(unsigned int R, unsigned int N)
{
	int i = 10000;	// Anti-hang loop counter
	
	/* Check input value ranges */
	if (R <   2) return 1;
	if (R >   7) return 1;
	if (N < 192) return 1;
	if (N > 432) return 1;

	/* Turn PLL off and reset counters */
	RCC_CR &= ~(1<<26);			// Disable PLL2I2S if previously enabled (p 92)
	while ( (RCC_CR & (1<<27)) == 1 );	// Be sure it is not ready (p 93)
	RCC_PLLI2SCFGR &= ~(0x70007fc0) ;	// Reset current R & N setttings (p 132)

	/* Set up new PLL counters (p 132) */
	RCC_PLLI2SCFGR |= (R << 28) | (N << 6) ;

	/* Enable PLL */
	RCC_CR |= (1<<26);	// Enable bit

	/* Wait for it to come ready, but don't allow to hang forever. (p 93) */
	while ( ((RCC_CR & (1<<27)) == 1) && (--i > 0) );
	if ( i < 0 ) return 1;		// Timed out

	return 0;
}
