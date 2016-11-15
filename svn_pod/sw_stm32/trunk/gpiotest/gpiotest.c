/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : gpiotest
* Hackeroo           : deh
* Date First Issued  : 05/22/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Test program for gpio pins
*******************************************************************************/

/* 
This routine first blinks the four on-board LEDs, then waits for "commands" from
the PC.
Command: 'Xnb'<CR>
Where: X is port designator = a, b, c, d, e
n is bit number of part = 0 - 15
b is 0 or 1.  zero for off (low), one is for on (high)

This routine will then set/reset the bit.

Open minicom on the PC with 115200 baud and 8N1.

*/
#include <math.h>
#include <string.h>

#include "libopenstm32/rcc.h"
#include "libopenstm32/gpio.h"
#include "libopenstm32/usart.h"
#include "libusartstm32/usartallproto.h"
#include "libmiscstm32/systick1.h"
#include "libmiscstm32/printf.h"
#include "PODpinconfig.h"


/* The following conditional compiling is for testing/debugging */
#define XCLK 1	// Select routine for setting up clocks: 1 = clockspecifysetup; 0 = clocksetup
#if XCLK 
#include "libmiscstm32/clockspecifysetup.h"
struct CLOCKS clocks = { \
HSOSELECT_HSE_XTAL,	/* Select high speed osc 			*/ \
PLLMUL_6X,		/* Multiplier PLL: 0 = not used 		*/ \
1,			/* Source for PLLMUL: 0 = HSI, 1 = PLLXTPRE (1 bit predivider)	*/ \
0,			/* PLLXTPRE source: 0 = HSE, 1 = HSE/2 (1 bit predivider on/off)	*/ \
APBX_2,			/* APB1 clock = SYSCLK divided by 0,2,4,8,16; freq <= 36 MHz */ \
APBX_1,			/* APB2 prescalar code = SYSCLK divided by 0,2,4,8,16; freq <= 72 MHz */ \
AHB_1,			/* AHB prescalar code: SYSCLK/[2,4,8,16,32,64,128,256,512] (drives APB1,2) */ \
8000000			/* Oscillator source frequency, e.g. 8000000 for an 8 MHz xtal on the external osc. */ \
};
#else
#include "libmiscstm32/clocksetup.h"
#endif


/* This if for test sprintf */
char vv[180];	// sprintf buffer

/* This is for the tiny printf */
// Note: the compiler will give a warning about conflicting types
// for the built in function 'putc'.
void putc ( void* p, char c)
	{
		p=p;	// Get rid of the unused variable compiler warning
		USART1_txint_putc(c);
	}

/* LED identification

|-number on pc board below LEDs
|   |- color
v vvvvvv  macro
3 green   
4 red
5 green
6 yellow
*/

/* ************************************************************
Turn the LEDs on in sequence, then turn them back off 
***************************************************************/
static int lednum = LED3;	// Lowest port bit numbered LED
void toggle_4leds (void)
{
	if ((GPIO_ODR(GPIOE) & (1<<lednum)) == 0)
	{ // Here, LED bit was off
		GPIO_BSRR(GPIOE) = (1<<lednum);	// Set bits = all four LEDs off
	}
	else
	{ // HEre, LED bit was on
		GPIO_BRR(GPIOE) = (1<<lednum);	// Reset bits = all four LEDs on
	}
	lednum += 1;		// Step through all four LEDs
	if (lednum > LED6) lednum = LED3;
}
/***************************************************************************************************
And now for the main routine 
****************************************************************************************************/
int main(void)
{
	int i = 0; 		// Timing loop variable
	int j;			// Another (fixed pt FORTRAN!) variable
	char* p;		// Temp getline pointer
	int b,n1,n2,n;


/* --------------------- Begin setting things up -------------------------------------------------- */ 
#if XCLK 
	clockspecifysetup(&clocks);		// Get the system clock and bus clocks running
#else
	clocksetup();		// Get the system clock and bus clocks running
#endif

	PODgpiopins_default();	// Set gpio port register bits for low power
	PODgpiopins_Config();	// Now, configure pins
	MAX3232SW_on		// Turn on RS-232 level converter (if doesn't work--no RS-232 chars seen)

	init_printf(0,putc);	// This one-time initialization is needed by the tiny printf routine

	SYSTICK_init(0);	// Set SYSTICK for interrupting and to max count (24 bit counter)

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
	USART1_rxinttxint_init(115200,32,2,8,3); // Initialize USART and setup control blocks and pointers
//	USART1_rxinttxint_init(  9600,32,2,8,3); // Initialize USART and setup control blocks and pointers

	/* Announce who we are */
	USART1_txint_puts("\n\rgpioteset 06-10-2011\n\r");
	USART1_txint_send();	// Start the line buffer sending

	USART1_txint_puts("The four leds will 'walk' on, then off, in the sequence LED3,4,5,6\n\rWhile LEDs are flash enter chars on keyboard and hit return to echo\n (test USART)\r");
	USART1_txint_send();	// Start the line buffer sending
	USART1_txint_puts("'x' as the first char will break this loop and enter loop for macros\n\r");
	USART1_txint_send();	// Start the line buffer sending
	
/* --------------------- Flash LEDs a number of times ----------------------------------------------- */
//	for (j = 0; j < 40000; i++)
	while (1==1)
	{
		/*LED blink rate timing */
		if ( i++ > 900000)
		{
			toggle_4leds();
			i = 0; j += 1;
		}

		/* Echo incoming chars back to the sender */
		if ((p=USART1_rxint_getline()) != 0)	// Check if we have a completed line
		{ // Here, pointer points to line buffer with a zero terminated line
//			printf ("Enter something and hit ENTER\n\r");	
//			USART1_txint_send();	// Start the line buffer sending
//			printf ("Char ct = %u: ", strlen(p)); // 'strlen()' for demo only; don't do this
//			USART1_txint_send();	// Start the line buffer sending
			if (*p == 'x') break;	// Quit when x is the first char of the line
			USART1_txint_puts(p);		// Echo back the line just received
			USART1_txint_puts ("\n");	// Add line feed to make things look nice
			USART1_txint_send();	// Start the line buffer sending
		}

	}
	LEDSALL_off	// Turn off all 4 LEDs




/* --------------------- Do Port bit commands ----------------------------------------------------- */

	USART1_txint_puts("Enter port char (p) a-e, bit (bb) 00-15, (n) 0 or 1 for set low or high as\n\rpbbn\n\ra040 [example: AD7799_2_CS_low]; (chars do not echo) 'x' to break loop---\n\r");
	USART1_txint_send();	// Start the line buffer sending
	*p = 'a';

	while (*(p+0) != 'x')	// Loop until 'x' is hit
	{


		if ((p=USART1_rxint_getline()) != 0)	// Check if we have a completed line
		{ // Here, pointer points to line buffer with a zero terminated line
			USART1_txint_puts(p);		// Echo back the line just received
			USART1_txint_puts ("\n");	// Add line feed to make things look nice
			USART1_txint_send();	// Start the line buffer sending

			n1 = *(p+1) - '0';	// Simple ASCII to binary
			n2 = *(p+2) - '0';
			b  = *(p+3) - '0';
			n = n1*10+n2;		// Port bit runs 0 - 15
			if ( (n < 0) | (n > 15) | (b < 0) | (b > 1) )	// Check for bogus numbers
			{
				USART1_txint_puts("Bogus numbers\n\r");	USART1_txint_send();
			}				
			else
			{
				switch (*(p+0))	// Case based on port designation a-e
				{
				case 'a':
					configure_pin ((volatile u32 *)GPIOA, n);
					if (b == 0)GPIO_BRR(GPIOA)  = (1<<n);// Reset bit
					if (b == 1)GPIO_BSRR(GPIOA) = (1<<n);// Set bit
					break;
				case 'b':
					configure_pin ((volatile u32 *)GPIOB, n);
					if (b == 0)GPIO_BRR(GPIOB)  = (1<<n);// Reset bit
					if (b == 1)GPIO_BSRR(GPIOB) = (1<<n);// Set bit
					break;
				case 'c':
					configure_pin ((volatile u32 *)GPIOC, n);
					if (b == 0)GPIO_BRR(GPIOC)  = (1<<n);// Reset bit
					if (b == 1)GPIO_BSRR(GPIOC) = (1<<n);// Set bit
					break;
				case 'd':
					configure_pin ((volatile u32 *)GPIOD, n);
					if (b == 0)GPIO_BRR(GPIOD)  = (1<<n);// Reset bit
					if (b == 1)GPIO_BSRR(GPIOD) = (1<<n);// Set bit
					break;
				case 'e':
					configure_pin ((volatile u32 *)GPIOE, n);
					if (b == 0)GPIO_BRR(GPIOE)  = (1<<n);// Reset bit
					if (b == 1)GPIO_BSRR(GPIOE) = (1<<n);// Set bit
					break;
				default:	
					USART1_txint_puts("Bogus port\n\r");
					USART1_txint_send();
					break;
				USART1_txint_puts("Enter port cmd: Pnnb (P:a-e, x to quit; nn = 00-15; b = 0-1)\n\r");USART1_txint_send();
				}
			}
		}
	}
	return 0;	
}


