/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : Utest2.c.usart2.rxinttxcir
* Hackor             : deh
* Date First Issued  : 10/12/2010
* Description        : Test program for tx dma, rx int (char-by-char interrupt)
*******************************************************************************/

/* 
Open minicom on the PC with 115200 baud and 8N1 and this routine
will (should!) echo back chars sent by minicom when "Enter" is typed.

This is setup for the Olimex stm32 board.  If a different board is used
then here are some of the changes that might be required.
- LED port and pin
- USART port

*/
#include <math.h>
#include <string.h>

#include "libopenstm32/rcc.h"
#include "libopenstm32/gpio.h"
#include "libopenstm32/usart.h"

#include "libusartstm32/usartallproto.h"


#include "libmiscstm32/printf.h"
#include "libmiscstm32/clockspecifysetup.h"


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

/* The following is for checking that static variables are properly intialized */
char cC[15]={0x0a,0x0d,'M','y','x','w','v','u','t','s','r','q',0x0a,0x0d,0};

/* Some things to check floating point */
//pi            3.1415926535897932384626433832795028841971 (41)
double pi = 3.14159265358979323846; // (21 digits)
double dF = 6.0;
int nScale = 1000000000;

/* This is for the tiny printf */
// Note: the compiler will give a warning about conflicting types
// for the built in function 'putc'.
void putc ( void* p, char c)
	{
		p=p;	// Get rid of the unused variable compiler warning
		USART1_txcir_putc(c);
	}
/* Setup the gpio for the LED on the Olimex stm32 P103 board */
void gpio_setup(void)
{
	/* Setup GPIO pin for LED (PC12) (See Ref manual, page 157) */
	// 'CRH is high register for bits 8 - 15; bit 12 is therefore the 4th CNF/MODE position (hence 4*4)
	GPIO_CRH(GPIOC) &= ~((0x000f ) << (4*4));	// Clear CNF reset bit 01 = Floating input (reset state)
	GPIO_CRH(GPIOC) |=  (( (GPIO_CNF_OUTPUT_PUSHPULL<<2) | (GPIO_MODE_OUTPUT_2_MHZ) ) << (4*4));
	
}
/* Stupid routine for toggling the gpio pin for the LED */
#define LEDBIT	12


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
/* And now for the main routine */
int main(void)
{
	int i = 0; 		// Timing loop variable
	int j;			// Another (fixed pt FORTRAN!) variable
	double dS;		// Some floating point mischief
	int n1,n2;		// Float-to-fix for printing floating pt
	char* p;		// Temp getlne pointer
	char alt = 0;		// Step between types of getline and getcount
	struct USARTLB lb;	// Holds the return from 'getlineboth' of char count & pointer
 
	clockspecifysetup((struct CLOCKS*)&clocks);		// Get the system clock and bus clocks running.
	gpio_setup();		// Need this to make the LED work


	/* Initialize usart ('_rxinttxcir_init)
	Receive:  rxint	rx into line buffers
	Transmit: txcir	tx with one circular buffer
	ARGS: 
		baud rate, e.g. 9600, 38400, 57600, 115200, 230400, 460800, 921600
		rx line buffer size, (long enough for the longest line)
		number of rx line buffers, (must be > 1)
		tx circular buffer size ("about" long enough for longest line sent)
	*/
	USART1_rxinttxcir_init(115200,48,4,128);

	init_printf(0,putc);	// This needed by the tiny printf routine

	/* Announce who we are */
	USART1_txcir_puts("\n\rUtest2 USART1 txcir rxint 10-15-2010..........1234567890..........12345Z\n\r");

	/* Check that printf works  */
	printf ("\n\rThis is printf - the number is %u\n\r",314159265);

	/* Test that busy works and doesn't overrun the output line buffers.
	If the busy doesn't work, then either it hangs, or doesn't prevent 'send'
        from stepping ahead into line buffers that have not as yet been sent.
	*/
	USART1_txcir_puts("1 The quick brown fox jumped over a lazy dog's back\n\r");

	USART1_txcir_puts("2 The quick brown fox jumped over a lazy dog's back\n\r");

	USART1_txcir_puts("3 The quick brown fox jumped over a lazy dog's back\n\r");
 
	USART1_txcir_puts("4 The quick brown fox jumped over a lazy dog's back\n\r");

	USART1_txcir_puts("5 The quick brown fox jumped over a lazy dog's back\n\r");

	USART1_txcir_puts("6 The quick brown fox jumped over a lazy dog's back\n\r");

	/* Check that 'putc doesn't lose chars */
	printf ("\n\rX The quick brown fox jumped over a lazy dog's back %10u\n\r",314159265);
	
	/* Testing that static variables got initialized OK in 'startup_deh' */
	USART1_txcir_puts(cC);
	
	/* A little experimenting with floating point */
	dS = sin(pi/dF);
	n1 = ( dS + (0.5/nScale) ) * nScale; // Round and scale 
	n2 = ((pi/dF) + (0.5/nScale) )*nScale;

	printf ("sine of %u is %u, scaled upwards by %u\n\r", n2, n1, nScale);

	printf ("\n\n\rEnter something and hit ENTER\n\r");

	/* Blink the LED on the board so that we can see it is alive */
	while (1==1) 
	{
		/*LED blink rate timing */
		if ( i++ > 100000)
		{
			toggle_led();
			i = 0;
		}
		/* Echo incoming chars back to the sender */
		switch (alt)	// Demonstrate different methods of using 'rxint' 
		{
		case 0:
			if ((p=USART1_rxint_getline()) != 0)	// Check if we have a completed line
			{ // Here, pointer points to line buffer with a zero terminated line
				printf ("Case %u.  Enter something and hit ENTER\n\r", alt);	
				printf ("Char ct = %u: ", strlen(p)); // 'strlen()' for demo only; don't do this
				USART1_txcir_puts(p);		// Echo back the line just received
				USART1_txcir_puts ("\n");	// Add line feed to make things look nice
				alt = 1;			// Do it differently next time
			}
			break;
		case 1:
			lb = USART1_rxint_getlineboth();	// Get both char count and pointer
			/* Check if a line is ready.  Either 'if' could be used */
//			if (lb.ct > 0)			// Check if we have a completed line
			if (lb.p > (char*)0)		// Check if we have a completed line
			{ // Here we have a pointer to the line and a char count
				printf ("Case %u.  Enter something and hit ENTER\n\r", alt);
				printf ("Char ct = %u: ", lb.ct);
				USART1_txcir_puts(lb.p);	// Echo back the line just received
				USART1_txcir_puts ("\n");	// Add line feed to make things look nice
				alt = 2;			// Do it differently next time
			
			}
			break;
		case 2:
			lb = USART1_rxint_getlineboth();	// Get both char count and pointer
			/* Check if a line is ready.  Either 'if' could be used */
//			if (lb.ct > 0)			// Check if we have a completed line
			if (lb.p > (char*)0)		// Check if we have a completed line
			{ // Here we have a pointer to the line and a char count
				printf ("Case %u.  Enter something and hit ENTER\n\r", alt);
				printf ("Char ct = %u: ", lb.ct);
				/* Do something without using the line as a zero terminated string */
				for (j = 0; j < lb.ct; j++)
					USART1_txcir_putc(*lb.p++);	// Echo back the line just received
				USART1_txcir_puts ("\n");		// Add line feed to make things look nice
				alt = 0;				// Do it differently next time
			}
			break;
		}
	}
	return 0;

}

