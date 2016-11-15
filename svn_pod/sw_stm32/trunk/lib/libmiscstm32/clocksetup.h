/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : clocksetup.h
* Hackor             : deh
* Date First Issued  : 09/12/2010
* Description        : A mess hacked from the STM and Libopenstm32 junk
*******************************************************************************/
// (Ref Manual page 121 for RCC registers)
// (Ref Manual page 54 for flash wait states)


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CLOCKLSETUP_H
#define __CLOCKLSETUP_H

/* Crystal frequency for high speed external oscillator, and PLL multiplier---*/
#define	XTAL	8000000	// HSE osc freq (Hz)

/*
000x: Reserved
0010: PLL input clock x 4
0011: PLL input clock x 5
0100: PLL input clock x 6
0101: PLL input clock x 7
0110: PLL input clock x 8
0111: PLL input clock x 9
10xx: Reserved
1100: Reserved
1101: PLL input clock x 6.5
111x: Reserved
Caution: The PLL output frequency must not exceed 72 MHz.
*/
#define PLLMUL	9	// PLL Multiplier (2 - 16)
#define PLLMULF	(PLLMUL-2)	// PLL multiplier 
#define SYSCLKX	(PLLMUL*XTAL)	// SYSCLK freq used here

// ADC clock = SYSCLK divided by 2,4,6,or 8 only
// ADC clock limit is 14000000 (Hz)
#define A_DIV1		0
#define A_DIV2		1
#define A_DIV6		2
#define A_DIV8		3

#define ADCDIV	A_DIV6	// If SYSCLKX = 72MHz then divide by 6 -> 12 MHz adc


// APB2 clock = SYSCLK divided by 0, 2, 4, 8, or 16 only
#define P_DIV1		0
#define P_DIV2		4
#define P_DIV4		5
#define P_DIV8		6
#define P_DIV16		7

#define PPPRE2CODE	P_DIV1

// APB1 clock = SYSCLK divided by 0, 2, 4, 8, or 16 only,
// and not exceed 36 MHz
#define PPPRE1CODE	P_DIV2

// AHB prescaler clock selection
#define H_DIV1		0	// SYSCLK not divided
#define H_DIV2		8	//0x1000: SYSCLK divided by 2
#define H_DIV4		9	//0x1001: SYSCLK divided by 4
#define H_DIV8		10	//0x1010: SYSCLK divided by 8
#define H_DIV16		11	//0x1011: SYSCLK divided by 16
#define H_DIV64 	12	//0x1100: SYSCLK divided by 64
#define H_DIV128	13	//0x1101: SYSCLK divided by 128
#define H_DIV256	14	//0x1110: SYSCLK divided by 256
#define H_DIV512	15	//0x1111: SYSCLK divided by 512

#define HPRECODE	H_DIV1	// Divide code for AHB prescaler

// Select system clock source
#define SWCODE	0x10		// PLL selected as system clock

// Convert divisor code to divisor
#if ((PPPRE2CODE)>0)
#define PCLK2DIV	(1<<(PPPRE2CODE-3))
#else
#define PCLK2DIV	1
#endif

// Convert divisor code to divisor
#if ((PPPRE1CODE)>0)
#define PCLK1DIV	(1<<(PPPRE1CODE-3))
#else
#define PCLK1DIV	1
#endif

// Convert divisor code to divisor
#if ((HPRECODE)>7)
#define HPREDIV	(1<<(HPRECODE-7))
#else
#define HPREDIV		1
#endif

// Determine flash wait states based on SYSCLKX frequency
#if (SYSCLKX <= 240000000)		// 0-24 MHz
	#define FLASHWAIT	0	// No wait states
#else
	#if (SYSCLKX <= 48000000)		// 24-48MHz
		#define FLASHWAIT	1	// One wait
	#else
		#define FLASHWAIT	2	// 48-72MHz, two
#endif
#endif
		


// The following frequencies are used by various programs

/* Clock freqs in Hz */
#define SYSCLK_FREQ	SYSCLKX			// E.g. 72000000
#define HCLK_FREQ	SYSCLKX/HPREDIV		// E.g. 72000000
#define PCLK1_FREQ	SYSCLKX/PCLK1DIV	// E.g. 72000000
#define PCLK2_FREQ	SYSCLKX/PCLK2DIV	// E.g. 36000000
#define ADCCLK_FREQ	SYSCLK_FREQ/ADCDIV	// E.g. 12000000


void clocksetup(void);


#endif

