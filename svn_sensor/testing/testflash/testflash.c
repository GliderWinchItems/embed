/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : flash.c
* Author             : deh
* Date First Issued  : 07/31/2013
* Board              : sensor board RxT6 w STM32F103RGT6
* Description        : Do some flash work
*******************************************************************************/
/* 
Hack of SE2 routine

This program resides in flash, starting at 0x08000000.  A program from the PC
via 'gateway' unit first loads a loader program into RAM.  Execution out of RAM
then loads flash at a location following this fixed loader.

Open minicom on the PC with 115200 baud and 8N1.

07-29-2013 rev flash test code

*/



#include <math.h>
#include <string.h>

#include "libopenstm32/adc.h"
#include "libopenstm32/can.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/gpio.h"
#include "libopenstm32/usart.h"
#include "libusartstm32/usartallproto.h"
#include "libmiscstm32/printf.h"
#include "libmiscstm32/clockspecifysetup.h"

#include "SENSORpinconfig.h"
#include "panic_leds.h"
#include "flash_write.h"


/* For test with and without XTAL clocking */
#define NOXTAL 
#ifdef NOXTAL

/* Parameters for setting up clock. (See: "libmiscstm32/clockspecifysetup.h" */

/* NOTE: APB2 is set 32 MHz and the ADC set for max divide (divide by 8).  The slower ADC helps the 
   accuracy in the presence of source impedance. */

// INTERNAL RC osc parameters -- 64 MHz
const struct CLOCKS clocks = { \
HSOSELECT_HSI,	/* Select high speed osc 			*/ \
PLLMUL_16X,		/* Multiplier PLL: 0 = not used 		*/ \
0,			/* Source for PLLMUL: 0 = HSI, 1 = PLLXTPRE (1 bit predivider)	*/ \
0,			/* PLLXTPRE source: 0 = HSE, 1 = HSE/2 (1 bit predivider on/off)	*/ \
APBX_2,			/* APB1 clock = SYSCLK divided by 1,2,4,8,16; freq <= 36 MHz */ \
APBX_4,			/* APB2 prescalar code = SYSCLK divided by 1,2,4,8,16; freq <= 72 MHz */ \
AHB_1,			/* AHB prescalar code: SYSCLK/[2,4,8,16,32,64,128,256,512] (drives APB1,2) */ \
8000000			/* Oscillator source frequency, e.g. 8000000 for an 8 MHz xtal on the external osc. */ \
};

#else

/* Parameters for setting up clock. (See: "libmiscstm32/clockspecifysetup.h" */
const struct CLOCKS clocks = { \
HSOSELECT_HSE_XTAL,	/* Select high speed osc 			*/ \
PLLMUL_8X,		/* Multiplier PLL: 0 = not used 		*/ \
1,			/* Source for PLLMUL: 0 = HSI, 1 = PLLXTPRE (1 bit predivider)	*/ \
0,			/* PLLXTPRE source: 0 = HSE, 1 = HSI/2 (1 bit predivider on/off)	*/ \
APBX_2,			/* APB1 clock = SYSCLK divided by 1,2,4,8,16; freq <= 36 MHz */ \
APBX_4,			/* APB2 prescalar code = SYSCLK divided by 1,2,4,8,16; freq <= 72 MHz */ \
AHB_1,			/* AHB prescalar code: SYSCLK/[2,4,8,16,32,64,128,256,512] (drives APB1,2) */ \
8000000			/* Oscillator source frequency, e.g. 8000000 for an 8 MHz xtal on the external osc. */ \
};

#endif




/* ************************************************************
Step through the LEDs
***************************************************************/
static int ledct;

void walk_LEDs(void)
{
	switch (ledct)
	{
	case 0: LED19RED_off;		LED20RED_off;		LED21GREEN_on;		break;
	case 1: LED19RED_off;		LED20RED_on;		LED21GREEN_off;		break;
	case 2: LED19RED_on;		LED20RED_off;		LED21GREEN_off;		break;
	default: ledct = 0; break;
	}
	ledct += 1;		// Step through all four LEDs
	if (ledct > 2) ledct = 0;
	return;
}
/* **************************************************************************************
 * void putc ( void* p, char c); // This is for the tiny printf
 * ************************************************************************************** */
// Note: the compiler will give a warning about conflicting types
// for the built in function 'putc'.  Use ' -fno-builtin-putc' to eliminate compile warning.
void putc ( void* p, char c)
	{
		p=p;	// Get rid of the unused variable compiler warning
		USART1_txint_putc(c);
	}

/*#################################################################################################
And now for the main routine 
  #################################################################################################*/
int main(void)
{
	int i = 0; 

/* --------------------- Begin setting things up -------------------------------------------------- */ 

	clockspecifysetup((struct CLOCKS*)&clocks);		// Get the system clock and bus clocks running

/* ---------------------- Set up pins ------------------------------------------------------------- */
	SENSORgpiopins_Config();	// Now, configure pins

	/* Use DTW_CYCCNT counter for timing */
/* CYCCNT counter is in the Cortex-M-series core.  See the following for details 
http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0337g/BABJFFGJ.html */
	*(volatile unsigned int*)0xE000EDFC |= 0x01000000; // SCB_DEMCR = 0x01000000;
	*(volatile unsigned int*)0xE0001000 |= 0x1;	// Enable DTW_CYCCNT (Data Watch cycle counter)

/* --------------------- Initialize usart --------------------------------------------------------- */
/*	USARTx_rxinttxint_init(...,...,...,...);
	Receive:  rxint	rx into line buffers
	Transmit: txint	tx with line buffers
	ARGS: 
		baud rate, e.g. 9600, 38400, 57600, 115200, 230400, 460800, 921600
		rx line buffer size, (long enough for the longest line)
		number of rx line buffers, (must be > 1)
		tx line buffer size, (long enough for the longest line)
		number of tx line buffers, (must be > 1)
*/
	USART1_rxinttxint_init(115200,32,2,96,4); // Initialize USART and setup control blocks and pointers
//	USART1_rxinttxint_init(  9600,32,2,32,3); // Initialize USART and setup control blocks and pointers

	/* Announce who we are */
	USART1_txint_puts("\n\rldr: 07-24-2013 \n\r");
	USART1_txint_send();	// Start the line buffer sending

	/* Display things for to entertain the hapless op */
	init_printf(0,putc);	// This one-time initialization is needed by the tiny printf routine

 	printf ("NO  XTAL\n\r");

	printf ("  hclk_freq (MHz) : %9u\n\r",  hclk_freq/1000000);	USART1_txint_send();
	printf (" pclk1_freq (MHz) : %9u\n\r", pclk1_freq/1000000);	USART1_txint_send();
	printf (" pclk2_freq (MHz) : %9u\n\r", pclk2_freq/1000000);	USART1_txint_send();
	printf ("sysclk_freq (MHz) : %9u\n\r",sysclk_freq/1000000);	USART1_txint_send();

/* --------------------- Program is ready, so do program-specific startup ---------------------------- */


int delayctr = 0;
#define FLASHINCRMENT	16000000	// System ticks for toggling LED

#define COPYFROM ((u16*)0x08000000)
//#define COPYFROM (&x[0])
#define FLASHTO  ((u16*)0x08083000) // Upper bank
//#define FLASHTO  ((u16*)0x08061400) // Lower bank
int flash_ret;
void phex(u16* p);
extern u32 flash_err;
u32 diff;
u16* copyfrom;
u16* flashto;
#define COPYCOUNT	1024	// Number of half-words
u16 x[1024];
for (i = 0; i < 1024; i++) x[i] = 0xffff;

printf("Flash size (Kbytes): %d\n\r",*(u16*)0x1FFFF7E0);USART1_txint_send();

/* Green LED flashing */
/* --------------------- Endless Stuff ----------------------------------------------- */
u32 dwt = (*(volatile unsigned int *)0xE0001004) + FLASHINCRMENT; // DWT_CYCNT
	while (1==1)
	{
		/* Wait for time to expire. */
		if (  ((int)dwt - (int)(*(volatile unsigned int *)0xE0001004)) < 0 )
		{
			dwt += FLASHINCRMENT;	// Set next LED toggle time
			TOGGLE_GREEN;
			if (delayctr++ > 8)
			{
//				flash_moveandwrite((u16*)0x0800f000, (u16*)0x08000000, 0x100);
//				(*(  (void (**)(void))0x08004004)  )();

				/* Test flash */
dwt = (*(volatile unsigned int *)0xE0001004); // DWT_CYCNT
				 flash_ret = flash_erase(FLASHTO);
diff = (*(volatile unsigned int *)0xE0001004); // DWT_CYCN
printf ("Tick diff: %u\n\r",((int)diff - (int)dwt));USART1_txint_send();
				if (flash_ret != 0)
				{
					printf("Erase err: %d\n\r",flash_ret);USART1_txint_send();
				}
				printf("Erase\n\r");USART1_txint_send();
				phex((u16*)FLASHTO);

				printf("\n\rWrite");USART1_txint_send();
dwt = (*(volatile unsigned int *)0xE0001004); // DWT_CYCNT
				flash_ret = flash_write( FLASHTO, COPYFROM, COPYCOUNT);
diff = (*(volatile unsigned int *)0xE0001004); // DWT_CYCN
printf ("Tick diff: %u\n\r",((int)diff - (int)dwt));USART1_txint_send();
				if (flash_ret != 0)
				{
					printf("Write err: %d %08 flash_err: %08x\n\r",flash_ret, flash_err);USART1_txint_send();
					
				}
				phex((u16*)FLASHTO);
				flashto = FLASHTO; copyfrom = COPYFROM;
				for (i = 0; i < COPYCOUNT; i++)
				{
					if (*flashto++ != *copyfrom++) break;
				}
				if (i < COPYCOUNT)
				{
					flashto--; copyfrom--; i--;
					printf("Verfiy err:  bytes: to %04x from %04x at: %u\n\r",*flashto, *copyfrom,i);USART1_txint_send();
				}
				while (1==1);
				
			}
		}
	}

	

	return 0;	
}
void phex(u16* p)
{
	int i;
	int j = 0;
	printf("\n\r%3u: ",j);USART1_txint_send();
	for (j = 0; j < COPYCOUNT/16; j++)
	{
		for (i = 0; i < 16; i++)
			printf("%04x ",*p++);
		printf("\n\r%3u: ",j+1);USART1_txint_send();
	}
	return;
}


