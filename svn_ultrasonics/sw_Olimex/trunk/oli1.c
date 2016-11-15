/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : sdcardtest2.c
* Author             : 
* Date First Issued  : 09/01/2012
* Board              : Olimex P103 (USART2)
* Description        : Test program for Olimex, leading to ultrosonic tension sensing
*******************************************************************************/

/* NOTE: 
Open minicom on the PC with 115200 baud and 8N1 and this routine.
...
*/

/* Subroutine protoypes */

#include <math.h>
#include <string.h>

#include "libopenstm32/rcc.h"
#include "libopenstm32/gpio.h"
#include "libopenstm32/usart.h"

#include "libopenstm32/systick.h"


#include "libusartstm32/usartallproto.h"
#include "libmiscstm32/systick1.h"



#include "libmiscstm32/printf.h"
//#include "libmiscstm32/clocksetup.h"
#include "libmiscstm32/clockspecifysetup.h"

#include "PODpinconfig.h"

#include "spi2sdcard.h"
#include "sdcard.h"
#include "libsupportstm32/sdcard_csd.h"
#include "sdcard_csd_print.h"
#include "sdcard_cid_print.h"

/* Simpleton routines to convert input ascii */
int dumbasctoint(char *p);
int dumb1st(char *p);
int dumb2nd(char *p);


/* ----------------- Clocking -------------------------------------------------- */

/* 'struct CLOCKS clocks' is used to setup the clock source, PLL, dividers, and bus clocks 
See P 84 of Ref Manual for a useful diagram.
../lib/libmiscstm32/clockspecifysetup.h has the 'enum' values that may help
in making mistakes(!) */
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
extern unsigned int	sysclk_freq;	/* 	SYSCLK freq		E.g. 72000000	*/
extern unsigned int	pclk1_freq;	/*	SYSCLKX/PCLK1DIV	E.g. 72000000 	*/

/* ----------------- Clocking -------------------------------------------------- */


/* This if for test sprintf */
char vv[180];	// sprintf buffer

/* This is for the tiny printf */
// Note: the compiler will give a warning about conflicting types
// for the built in function 'putc'.
void putc ( void* p, char c)
	{
		p=p;	// Get rid of the unused variable compiler warning
		USART2_txint_putc(c);
	}
/*****************************************************************************************/
/* Setup the gpio for the LED on the Olimex stm32 P103 board */
/*****************************************************************************************/
void gpio_setup(void)
{
	PODgpiopins_Config();	// Setup the pins for the STM32F103VxT6_pod_mm
	/* Setup GPIO pin for LED (PC12) (See Ref manual, page 157) */
	// 'CRH is high register for bits 8 - 15; bit 12 is therefore the 4th CNF/MODE position (hence 4*4)
	GPIO_CRH(GPIOC) &= ~((0x000f ) << (4*4));	// Clear CNF reset bit 01 = Floating input (reset state)
	GPIO_CRH(GPIOC) |=  (( (GPIO_CNF_OUTPUT_PUSHPULL<<2) | (GPIO_MODE_OUTPUT_2_MHZ) ) << (4*4));
	
}
/*****************************************************************************************/
/* Stupid routine for toggling the gpio pin for the LED */
/*****************************************************************************************/
#define LEDBIT	12	// Olimex board LED bit 
void toggle_led (void)
{
	if ((GPIO_ODR(GPIOC) & (1<<LEDBIT)) == 0)
	{
		GPIO_BSRR(GPIOC) = (1<<LEDBIT);	// Set bit
	}
	else
	{
		GPIO_BRR(GPIOC) = (1<<LEDBIT);	// Reset bit
	}
}
/* =============================== MAIN ================================================ */
/*****************************************************************************************/
/* And now for the main routine */
/*****************************************************************************************/
/* =============================== MAIN ================================================ */
int main(void)
{
	u16 temp;
	int i = 0; 		// Timing loop variable

	struct USARTLB lb;	// Holds the return from 'getlineboth' of char count & pointer
 
/* ========================== Initializations ============================================== */

	/* Clock setup */
	clockspecifysetup(&clocks);// Get the system clock and bus clocks running
	gpio_setup();		// Need this to make the LED work
	init_printf(0,putc);	// This one-time initialization is needed by the tiny printf routine
	SYSTICK_init(0);	// Set SYSTICK for interrupting and to max count (24 bit counter)

	/* Initialize usart 
	USARTx_rxinttxint_init(...,...,...,...);
	Receive:  rxint	rx into line buffers
	Transmit: txint	tx with line buffers
	ARGS: 
		baud rate, e.g. 9600, 38400, 57600, 115200, 230400, 460800, 921600
		rx line buffer size, (long enough for the longest line)
		number of rx line buffers, (must be > 1)
		tx line buffer size, (long enough for the longest line)
		number of tx line buffers, (must be > 1)
	*/
	// Initialize USART and setup control blocks and pointer
	if ( (temp = USART2_rxinttxint_init(115200,32,2,48,4)) != 0 )
	{ // Here, something failed 
		return temp;
	}

	/* Announce who we are */
	USART2_txint_puts("\n\roli1.c: USART2 txint rxint 09-01-2012\n\r");
	USART2_txint_send();	// Start the line buffer sending

	int nTogglerate = 500000;	// LED flashing at moderate speed

	/* Display things for to entertain the hapless op */
	printf ("pclk1_freq  (MHz)        : %9u\n\r", pclk1_freq/1000000);		USART2_txint_send();
	printf ("sysclk_freq (MHz)        : %9u\n\r",sysclk_freq/1000000);		USART2_txint_send();

/* ========================== Endless loop ================================================ */
	/* Blink the LED on the board so that we can see it is alive */
	while (1==1) 
	{
		/*LED blink rate timing */
		if ( i++ > nTogglerate)
		{
			toggle_led();
			i = 0;
		}
		
		/* The following is partly a demonstration of the different ways to handle USART input */
		lb = USART2_rxint_getlineboth();		// Get both char count and pointer
			/* Check if a line is ready.  Either 'if' could be used */
//			if (lb.ct > 0)				// Check if we have a completed line
		if (lb.p > (char*)0)				// Check if we have a completed line
		{ // Here we have a pointer to the line and a char count
			USART2_txint_puts(lb.p);		// Echo back the line just received
			USART2_txint_puts ("\n");		// Add line feed to make things look nice
			USART2_txint_send();			// Start the line buffer sending
		}
	}
	return 0;
}

