/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : clocksetup.c
* Hackor             : deh
* Date First Issued  : 09/12/2010
* Description        : A mess hacked from the STM and Libopenstm32 junk
*******************************************************************************/
#include "clocksetup.h"
#include "../libopenstm32/rcc.h"
#include "../libopenstm32/flash.h"

/* The following variables are used by peripherals in their 'init' routines to set dividers 
This routine doesn't change them.  In 'clockspecifysetup' these are computed from parameters
passed to it.

These are not declared constant since there may be routines that change the bus prescalar values.
*/
/* The following variables are used by peripherals in their 'init' routines to set dividers 	*/
extern unsigned int	hclk_freq;  	/* 	SYSCLKX/HPREDIV	 	E.g. 72000000 	*/
extern unsigned int	pclk1_freq;  	/*	SYSCLKX/PCLK1DIV	E.g. 72000000 	*/
extern unsigned int	pclk2_freq;	/*	SYSCLKX/PCLK2DIV	E.g. 36000000 	*/
extern unsigned int	sysclk_freq;	/* 	SYSCLK freq		E.g. 72000000	*/

/******************************************************************************
 * void clocksetup(void)
 * @brief	: Setup most of the clocks & bus freq dividers
 * @brief	: Setup external osc, with PLL, and minimum bus dividers
 ******************************************************************************/
void clocksetup(void)
{
	hclk_freq   = HCLK_FREQ;	/* 	SYSCLKX/HPREDIV	 	E.g. 72000000 	*/
	pclk1_freq  = PCLK1_FREQ;	/*	SYSCLKX/PCLK1DIV	E.g. 72000000 	*/
	pclk2_freq  = PCLK2_FREQ;	/*	SYSCLKX/PCLK2DIV	E.g. 36000000 	*/
	sysclk_freq = SYSCLKX;		/* 	SYSCLK freq		E.g. 72000000	*/
	/* See Ref manual page 115 for diagram of clocks, PLLs, etc. */

	/* Clock control register (Ref manual, page 121)	*/
	RCC_CR = RCC_CR_HSEON;	// Start oscillators hi-speed internal and external

	/* Wait for oscillators to stablize and show as ready */
	while ((RCC_CR & (RCC_CR_HSERDY)) == 0);	// Wait to become ready

	/* Flash access control register (Ref manual, page 54) */
	FLASH_ACR |= 2;	// Two wait states (for clock freq > 48 MHz)

	/* Clock configuraton register (Ref manual, page 123)	*/
	RCC_CFGR = ((PLLMUL-2)<<18) |	/* PLL multiplier code */	\
			    (1<<16) |	/* PLLSRC */			\
		       (ADCDIV<<14) | 	/* ADC divider code */		\
		   (PPPRE2CODE<<11) |	/* */				\
		   ( PPPRE1CODE<<8) |	/* */				\
		     (HPRECODE<< 4) ;	/* */	
						
	/* Enable PLL */
	RCC_CR |= (1<<24);			// PLLON:  Turn PLL on
	while ((RCC_CR & (1<<25) ) == 0);	// PLLRDY: Wait for PLL to become ready
	
	/* Select PLL as system clock source */
	RCC_CFGR |= 0x02;		// PLLSRC | SW sysclk from PLL

	while ( (RCC_CFGR & 0x0f) != 0x0a);	// Wait SYSCLK from PLL ready
	
	/* Enable all present GPIOx clocks. (whats with GPIO F and G?)*/
	RCC_APB2ENR |= 0x7c;	// Enable A thru E

	/* DMA1 is used with all three buffered USART routines */
//	RCC_AHBENR |= RCC_AHBENR_DMA1EN;	// 06-13-2011: this code placed in USARTx_rx(tx)dma.c routines

 	return;                                           
}

