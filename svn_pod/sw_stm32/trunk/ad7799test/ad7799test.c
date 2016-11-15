/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : spi1ad7799test.c
* Hackeroos          : caw, deh
* Date First Issued  : 06/17/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Test program for AD7799 sigma-delta ADC, driven via spi1
*******************************************************************************/

/* NOTE: 
Open minicom on the PC with 115200 baud and 8N1 and this routine
*/



#include <math.h>
#include <string.h>

#include "libopenstm32/rcc.h"
#include "libopenstm32/gpio.h"
#include "libopenstm32/usart.h"

#include "libopenstm32/systick.h"


#include "libusartstm32/usartallproto.h"
#include "libmiscstm32/systick1.h"



#include "libmiscstm32/printf.h"
#include "libmiscstm32/clockspecifysetup.h"

#include "PODpinconfig.h"

#include "spi1ad7799.h"
#include "ad7799_comm.h"

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
APBX_4,			/* APB2 prescalar code = SYSCLK divided by 0,2,4,8,16; freq <= 72 MHz */ \
AHB_1,			/* AHB prescalar code: SYSCLK/[2,4,8,16,32,64,128,256,512] (drives APB1,2) */ \
8000000			/* Oscillator source frequency, e.g. 8000000 for an 8 MHz xtal on the external osc. */ \
};

/* Variables set by SPI1 operations.  See ../devices/ad7799_comm.c */
extern unsigned char 	ad7799_8bit_reg;		// Byte with latest status register from ad7799_1
extern unsigned char 	ad7799_8bit_wrt;		// Byte to be written
extern volatile unsigned char 	ad7799_comm_busy;	// 0 = not busy with a sequence of steps, not zero = busy
extern unsigned short 	ad7799_16bit_wrt;		// 16 bits to be written
extern union SHORTCHAR 	ad7799_16bit;			// 16 bits read
extern union INTCHAR	ad7799_24bit;			// 24 bits read (high ord byte is set to zero)
		
/* The following variables are used by peripherals in their 'init' routines to set dividers 	*/
extern unsigned int	hclk_freq;  	/* 	SYSCLKX/HPREDIV	 	E.g. 72000000 	*/
extern unsigned int	pclk1_freq;  	/*	SYSCLKX/PCLK1DIV	E.g. 72000000 	*/
extern unsigned int	pclk2_freq;	/*	SYSCLKX/PCLK2DIV	E.g. 36000000 	*/
extern unsigned int	sysclk_freq;	/* 	SYSCLK freq		E.g. 72000000	*/

/* This if for test sprintf */
char vv[180];	// sprintf buffer

long long llX;	// Big integer for slowing pause loop


/* For tinkering with averaging ad7799 readings */
#define SAVESIZE	470*2	// Number of readings to save
int ad7799_data[SAVESIZE];	// Incoming data register readings
int ad7799bipolar;		// +/- correction for bipolar mode
unsigned short uiI;		// Index, and decimation count
double dSum,dAverage,dStdDev,dDiffsquared;
void printffloat (double dX);
extern int		ad7799_last_good_reading;	// Last good 24 bit reading, bipolar, zero offset adjusted



/*-------------------------------------------------------------------------------*/
/* This is for the tiny printf */
// Note: the compiler will give a warning about conflicting types
// for the built in function 'putc'.
void putc ( void* p, char c)
	{
		p=p;	// Get rid of the unused variable compiler warning
		USART1_txint_putc(c);
	}
/* Setup the gpio for the LED on the Olimex stm32 P103 board */
void gpio_setup(void)
{
	PODgpiopins_Config();	// Setup the pins for the STM32F103VxT6_pod_mm	
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
/* And now for the main routine */
int main(void)
{
	u16 temp;
	int i = 0; 		// Timing loop variable
	int j;			// Another (fixed pt FORTRAN!) variable
	char* p;		// Temp getline pointer
	unsigned int ui;
 
/* ---------- Initializations ------------------------------------------------------ */
	clockspecifysetup(&clocks);		// Get the system clock and bus clocks running

	PODgpiopins_default();	// Set gpio port register bits for low power
	PODgpiopins_Config();	// Now, configure pins

	/* Initialize SPI1 AD7799_1 */
	spi1ad7799_init();

	MAX3232SW_on		// Turn on RS-232 level converter (if doesn't work--no RS-232 chars seen)

	init_printf(0,putc);	// This one-time initialization is needed by the tiny printf routine

	SYSTICK_init(0);	// Set SYSTICK for interrupting and to max count (24 bit coutner)

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
	if ( (temp = USART1_rxinttxint_init(115200,32,2,48,4)) != 0 )
	{ // Here, something failed 
		return temp;
	}
	

/* ---------- End Initializations -------------------------------------------------- */
xx:
	/* Announce who we are */
	USART1_txint_puts("\n\rad7799test 06-17-2011\n\r");
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
	LED43_2_on	// Turn red LED on


/*----------------------------------- Tinker with AD7799 ---------------------------------------------- */
	llX = 0;	// Long long is for slowing down pause loop

	USART1_txint_puts("AD7799_1 register tinkering BEGIN\n\rRegisters follow\n\r");	USART1_txint_send();// Start the line buffer sending

	printf ("hclk %u pclk1 %u pclk2 %u sysclk %u\n\r",hclk_freq,pclk1_freq,pclk2_freq,sysclk_freq);USART1_txint_send();


	AD7799_RD_ID_REG;		// Read ID register (macro in ad7799_comm.h)
	while (ad7799_comm_busy != 0);	// Wait for write/read sequence to complete
	ui = ad7799_8bit_reg;
	printf ("ID    :  %6x \n\r",ui);
	USART1_txint_send();	// Start the line buffer sending

	/* Setup: Zero calibration, sampling rate, and continuous conversions */
	ad7799_1_initA(AD7799_4p17SPS);	// Setup for strain gauge, (Arg = sampling rate)
//	ad7799_1_initB(AD7799_4p17SPS);	// Setup for thermistor bridge, (Arg = sampling rate)
	while (ad7799_comm_busy != 0);	// Wait for write/read sequence to complete

	AD7799_RD_CONFIGURATION_REG;	// Read configuration register (macro in ad7799_comm.h)
	while (ad7799_comm_busy != 0);	// Wait for write/read sequence to complete
	ui = ad7799_16bit.us;
	printf ("Config:%8x \n\r",ui); USART1_txint_send();

	AD7799_RD_MODE_REG;		// Read mode register (macro in ad7799_comm.h)
	while (ad7799_comm_busy != 0);	// Wait for write/read sequence to complete
	ui = ad7799_16bit.us;
	printf ("Mode  :%8x \n\r",ui); USART1_txint_send();

	AD7799_RD_OFFSET_REG;		// Read offset register (macro in ad7799_comm.h)
	while (ad7799_comm_busy != 0);	// Wait for write/read sequence to complete
	printf ("Offset:%8x \n\r",ad7799_24bit.n); USART1_txint_send();

	AD7799_RD_FULLSCALE_REG;	// Read full scale register (macro in ad7799_comm.h)
	while (ad7799_comm_busy != 0);	// Wait for write/read sequence to complete
	printf ("FScale:%8x \n\r",ad7799_24bit.n); USART1_txint_send();

	ad7799_wr_mode_reg(AD7799_CONT | AD7799_470SPS );	// Continuous conversion (see ad7799_comm.h)
//	ad7799_wr_mode_reg(AD7799_CONT | AD7799_242SPS );	// Continuous conversion (see ad7799_comm.h)
//	ad7799_wr_mode_reg(AD7799_CONT | AD7799_10SPS );	// Continuous conversion (see ad7799_comm.h)


	for ( uiI = 0; uiI < SAVESIZE; uiI++)
	{
//		ad7799_wr_mode_reg(AD7799_SINGLE | AD7799_4p17SPS );	// Single conversion
//		while ( ((ui=ad7799_rd_status_reg ()) & 0x80) != 0);
//		printf ("%4u status%3x  ",i++,ui); 

		while (  (GPIO_IDR(GPIOA) & (1<<6))  != 0 );	// Wait for ad7799 /RDY line to go low
		AD7799_RD_DATA_REG;				// Read 24bit data register (macro in ad7799_comm.h)
		while (ad7799_comm_busy != 0);			// Wait for write/read sequence to complete
		ad7799bipolar = 0x800000 - ad7799_24bit.n;	// Zero = 0x800000 when in bipolar mode
		ad7799bipolar += 15300;				// Adjust for zero load offset of bridge
//		ad7799_data[uiI] = ad7799bipolar;		// Save in an array
		/* The following checks the ad7799_comm, 24 bit reg read that does the above two lines */
		ad7799_data[uiI] = ad7799_last_good_reading;	// Save in an array

//		while (  (GPIO_IDR(GPIOA) & (1<<6))  == 0 );	// Wait for ad7799 /RDY line to go high
		
	}
	dSum = 0; dDiffsquared = 0;
	for ( uiI = 0; uiI < SAVESIZE; uiI++)
	{
		if (ad7799_data[uiI] < 0) 			// Take care of tiny printf not handling negative ints
		{
			dSum += ad7799_data[uiI];
			ad7799_data[uiI]= -ad7799_data[uiI];
			printf ("--%9u\n\r", ad7799_data[uiI]); USART1_txint_send();	// Start the line buffer sending
		}
		else
		{
			dSum += ad7799_data[uiI];
			printf ("++%9u\n\r", ad7799_data[uiI]); USART1_txint_send();	// Start the line buffer sending
		}
	}
	dAverage = dSum/(SAVESIZE);
	for ( uiI = 0; uiI < SAVESIZE; uiI++)
	{
		dDiffsquared += (ad7799_data[uiI]-dAverage)*(ad7799_data[uiI]-dAverage);
	}
	dStdDev = sqrt(dDiffsquared)/SAVESIZE;
	USART1_txint_puts("Average:");	printffloat(dAverage);
	USART1_txint_puts("StdDev:");	printffloat(dStdDev);
	USART1_txint_send();
	printf ("\n\rEND.......\n\r"); USART1_txint_send();
	goto xx;
	while (1==1);
	return 0;	

}
void printffloat (double dX)
{
	unsigned int uiX = dX;
	printf("%6u.",uiX);
	uiX = dX*100 - uiX*100;
	printf("%02u ",uiX);	
	return;
}
