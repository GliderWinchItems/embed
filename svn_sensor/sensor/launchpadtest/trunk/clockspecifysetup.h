/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : clocksetup.h
* Hackor             : deh
* Date First Issued  : 06/14/2011
* Description        : Modifications of 'clocksetup.c' to change on-the-fly
*******************************************************************************/
// (Ref Manual page 121 for RCC registers)
// (Ref Manual page 54 for flash wait states)


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CLOCKSPECIFYSETUP_H
#define __CLOCKSPECIFYSETUP_H


/* Define codes versus multiplier for 1st and 2nd PLLs */	
enum pllmulx
{
	PLLMUL_NO	= 0,	// Not used
	PLLMUL_2X	= 0,
	PLLMUL_3X	= 1,	
	PLLMUL_4X	= 2,	// Input freq multiplied by 4 times
	PLLMUL_5X	= 3,
	PLLMUL_6X	= 4,
	PLLMUL_7X	= 5,
	PLLMUL_8X	= 6,
	PLLMUL_9X	= 7,
	PLLMUL_10X	= 8,
	PLLMUL_11X	= 9,
	PLLMUL_12X	= 10,
	PLLMUL_13X	= 11,
	PLLMUL_14X	= 12,
	PLLMUL_15X	= 13,
	PLLMUL_16X	= 14
};
/* Predivide scalar codes are more straightforward */
enum prediv
{
	PREDIV_1	= 0,	// No division, i.e. divide by 1
	PREDIV_2	= 1,	// Divide by 2
};
/* APBx divider codes */
enum apbx
{
	APBX_1	= 0,	// Not divided, i.e. divide by 1
	APBX_2	= 4,	// Divide by 2
	APBX_4	= 5,	// Divide by 4
	APBX_8	= 6,	// Divide by 8
	APBX_16	= 7	// Divide by 16
};
/* AHB divider codes */
enum ahbdiv
{
	AHB_1	= 0,	// Not divided, i.e. divide by 1
	AHB_2	= 8,	// Divide by 2
	AHB_4	= 9,	// Divide by 4
	AHB_8	= 10,	// Divide by 8
	AHB_16	= 11,	// Divide by 16
	AHB_64	= 12,	// Divide by 64
	AHB_128	= 13,	// Divide by 128
	AHB_256	= 14,	// Divide by 256
	AHB_512	= 15	// Divide by 512
};
/* Codes for high speed oscillator selection */
enum hsoselect
{
	HSOSELECT_HSI			= 0,	// Internal RC oscillator (8 MHz)
	HSOSELECT_HSE_XTAL		= 1,	// External xtal oscillator (2-25MHz)
	HSOSELECT_HSE_INPUT		= 2,	// External input signal
	HSOSELECT_HSE_XTAL_REMAPPED	= 3,	// External xtal oscillator remapped to PD0,1
	HSOSELECT_HSE_XTAL_INPUT	= 4	// External input signal remapped to PD0
};

/* struct for passing setup parameters */
struct CLOCKS
{
enum hsoselect	hso	; // Select high speed osc: 0 = internal 8 MHz rc; 1 = external xtal controlled; 2 = ext input; 3 ext remapped xtal; 4 ext input
enum pllmulx	pllmul	; // Multiplier PLL: 0 = not used
unsigned char	pllsrc	; // Source for PLLMUL: 0 = HSI, 1 = PLLXTPRE (1 bit predivider)
unsigned char	pllxtpre; // PLLXTPRE source: 0 = HSE, 1 = HSE/2 (1 bit predivider on/off)
enum apbx	apb1	; // APB1 clock = SYSCLK divided by 0,2,4,8,16; freq <= 36 MHz
enum apbx	apb2	; // APB2 prescalar code = SYSCLK divided by 0,2,4,8,16; freq <= 72 MHz
enum ahbdiv	ahb	; // AHB prescalar code: SYSCLK/[2,4,8,16,32,64,128,256,512] (drives APB1,2)
unsigned int	freq	; // Oscillator source frequency, e.g. 8000000 for an 8 MHz xtal on the external osc.	
};
/*
(1) When 
*/

/******************************************************************************/
void clockspecifysetup(struct CLOCKS *clocks);
/* @brief	: Setup the clocks & bus freq dividers
 * @param	: chase: 0 = use HSI (internal 8 MHz RC osc), 1 = use HSE (external Xtal osc)
 * @return	: static variables are set from input paramaters passed in struct
 ******************************************************************************/

/* The following variables are used by peripherals in their 'init' routines to set dividers (@1) */
extern unsigned int	hclk_freq;	/* 	SYSCLKX/HPREDIV	 	E.g. 72000000 	*/
extern unsigned int	pclk1_freq;	/*	SYSCLKX/PCLK1DIV	E.g. 72000000 	*/
extern unsigned int	pclk2_freq;	/*	SYSCLKX/PCLK2DIV	E.g. 36000000 	*/
extern unsigned int	sysclk_freq;	/* 	SYSCLK freq		E.g. 72000000	*/


#endif

