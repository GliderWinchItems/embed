/******************************************************************************
* File Name          : clockspecifysetup.h
* Date First Issued  : 03/03/2012
* Description        : Modifications of 'clocksetup.c' to change on-the-fly
*******************************************************************************/
// (Ref Manual page 121 for RCC registers)
// (Ref Manual page 54 for flash wait states)


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CLOCKSPECIFYSETUP_H
#define __CLOCKSPECIFYSETUP_H


/* PLLP divide codes */
enum pllpcode
{
	PLLP_2 = 0,	// Divide by 2
	PLLP_4 = 1,	// Divide by 4
	PLLP_6 = 2,	// Divide by 6
	PLLP_8 = 3,	// Divide by 8
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

/* struct for passing setup parameters p 95 */
struct CLOCKS
{
enum hsoselect	hso	; // Select high speed osc: 0 = internal 8 MHz rc; 1 = external xtal controlled; 2 = ext input; 3 ext remapped xtal; 4 ext input
unsigned char	pllsrc	; // Source for main PLL & audio PLLI2S: 0 = HSI, 1 = HSE selected
enum apbx	apb1	; // APB1 clock = SYSCLK divided by 0,2,4,8,16; freq <= 42 MHz
enum apbx	apb2	; // APB2 prescalar code = SYSCLK divided by 0,2,4,8,16; freq <= 84 MHz
enum ahbdiv	ahb	; // AHB prescalar code: SYSCLK/[2,4,8,16,32,64,128,256,512] (drives APB1,2)
unsigned int	freq	; // Oscillator source frequency, e.g. 8000000 for an 8 MHz xtal on the external osc.
unsigned short	pllq	; // (PLL) divider: USB OTG FS, SDIO, random number gen. USB OTG FS clock freq = VCO freq / PLLQ with 2 ≤ PLLQ ≤ 15
unsigned short	pllp	; // Main PLL divider: PLL output clock frequency = VCO frequency / PLLP with PLLP = 2, 4, 6, or 8
unsigned short	plln	; // Main PLL multiplier: VCO output frequency = VCO input frequency × PLLN with 64 ≤ PLLN ≤ 432	
unsigned short	pllm	; // VCO input frequency = PLL input clock frequency / PLLM with 2 ≤ PLLM ≤ 63
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
int clockI2Ssetup(unsigned int R, unsigned int N);
/* @brief	: Setup PLL for I2S: f(USB OTG FS, SDIO, RNG clock output) = f(VCO clock) / PLLQ
 * @param	: R = valid range: 2-7 (3 bits)
 * @param	: N = valid range: 192-432 (9 bits). 
 * @return	: 0 = OK; 1 = failed
 ******************************************************************************/

/* The following variables are used by peripherals in their 'init' routines to set dividers (@1) */
extern unsigned int	hclk_freq;	/* 	SYSCLKX/HPREDIV	 	E.g. 72000000 	*/
extern unsigned int	pclk1_freq;	/*	SYSCLKX/PCLK1DIV	E.g. 72000000 	*/
extern unsigned int	pclk2_freq;	/*	SYSCLKX/PCLK2DIV	E.g. 36000000 	*/
extern unsigned int	sysclk_freq;	/* 	SYSCLK freq		E.g. 72000000	*/


#endif

