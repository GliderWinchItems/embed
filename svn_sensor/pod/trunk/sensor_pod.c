/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : sensor_pod.c
* Hackeroo           : deh
* Date First Issued  : 09/30/2012
* Board              : STM32F103VxT6_pod_mm
* Description        : Use pod board to prototype sensor program & hardware
*******************************************************************************/
/* 

Open minicom on the PC with 115200 baud and 8N1.

*/
#include <math.h>
#include <string.h>

#include "PODpinconfig.h"
#include "libopenstm32/adc.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/gpio.h"
#include "libopenstm32/usart.h"
#include "libusartstm32/usartallproto.h"
#include "libmiscstm32/systick1.h"
#include "libmiscstm32/printf.h"
#include "libmiscstm32/clockspecifysetup.h"

#include "adcsensor_pod.h"
#include "canwinch_pod.h"
#include "sensor_threshold.h"
#include "common.h"
#include "scb.h"  // See systick1.h

#define IAMUNITNUMBER	CAN_UNITID_POD1	// Each node on the CAN bus gets a unit number

unsigned int Q = 0xabcd1234;

/* ------- LED identification ----------- 
|-number on pc board below LEDs
| color   ADCx codewheel  bit
3 green   ADC2   black    0
4 red	  ADC2   white    1
5 green   ADC1   black    0
6 yellow  ADC1   white    1
  --------------------------------------*/


/* (@2) 'struct CLOCKS clocks' is used to setup the clock source, PLL, dividers, and bus clocks 
See P 84 of Ref Manual for a useful diagram.
../lib/libmiscstm32/clockspecifysetup.h has the 'enum' values that may help for making mistakes(!) */
/* NOTE: Bus for ADC (APB2) must not exceed 14 MHz */
/* 48 MHz sys clock; APB1 24 MHz; APB2 48 MHz; */
const struct CLOCKS clocks = { \
HSOSELECT_HSI,		/* Select high speed internal osc 							*/ \
PLLMUL_12X,		/* Multiplier PLL: 0 = not used 							*/ \
0,			/* Source for PLLMUL: 0 = HSI, 1 = HSE							*/ \
0,			/* PLLXTPRE source: 0 = HSI, 1 = HSE/2 (1 bit predivider on/off)			*/ \
APBX_1,			/* APB1 clock = SYSCLK divided by 1,2,4,8,16; freq <= 36 MHz 				*/ \
APBX_2,			/* APB2 prescalar code = SYSCLK divided by 1,2,4,8,16; freq <= 72 MHz 			*/ \
AHB_1,			/* AHB prescalar code: SYSCLK/[2,4,8,16,32,64,128,256,512] (drives APB1,2) 		*/ \
8000000			/* Oscillator source frequency, e.g. 8000000 for an 8 MHz xtal on the external osc. 	*/ \
};



/* **************************************************************************************
 * void system_reset(void);
 * @brief	: Software caused RESET
 * ************************************************************************************** */
#include "scb.h"
void system_reset(void)
{
/* PM 0056 p 134 (April 2010 Doc ID 15491 Rev 3 1/154)
4.4.4 Application interrupt and reset control register (SCB_AIRCR)
      Address offset: 0x0C
      Reset value: 0xFA05 0000
      Required privilege: Privileged
      The AIRCR provides priority grouping control for the exception model, endian status for data
      accesses, and reset control of the system.
      To write to this register, you must write 0x5FA to the VECTKEY field, otherwise the
      processor ignores the write.
*/

/* Bit 2 SYSRESETREQ System reset request
      This is intended to force a large system reset of all major components except for debug.
      This bit reads as 0.
      0: No system reset request
      1: Asserts a signal to the outer system that requests a reset.
*/
	SCB_AIRCR  |= (0x5FA << 16) | (1 << 2);	// Cause a RESET
	while (1==1);
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







/***************************************************************************************************
And now for the main routine 
****************************************************************************************************/
int main(void)
{
	int i = 0; 		// Timing loop variable
	char* p;		// Temp getline pointer

/* --------------------- Begin setting things up -------------------------------------------------- */ 

	clockspecifysetup((struct CLOCKS*)&clocks);		// Get the system clock and bus clocks running

/* ---------------------- Set up pins ------------------------------------------------------------- */
	PODgpiopins_default();	// Set gpio port register bits for low power
	PODgpiopins_Config();	// Now, configure pins
	MAX3232SW_on;		// Turn on RS-232 level converter (if doesn't work--no RS-232 chars seen)
	ANALOGREG_on;		// Turn on 3.2v analog regulator

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
	USART1_rxinttxint_init(115200,32,2,96,3); // Initialize USART and setup control blocks and pointers
//	USART1_rxinttxint_init(  9600,32,2,32,3); // Initialize USART and setup control blocks and pointers

	/* Announce who we are */
	USART1_txint_puts("\n\rSENSOR_POD TEST 12-08-2012\n\r");
	USART1_txint_send();	// Start the line buffer sending

unsigned int* xx =   (unsigned int *)0x1FFFf000;	
	while (xx < ((unsigned int *)0x20000010) )
{
	printf("%08x %08x\n\r",(unsigned int)xx,*xx);
	USART1_txint_send();	// Start the line buffer sending
	xx ++;
}
while(1==1);



	USART1_txint_puts("'\n\r x>enter>' will reset counters\n\r");
	USART1_txint_send();	// Start the line buffer sending



/* --------------------- Program is ready, so do program-specific startup ---------------------------- */

/* ------- Histogram --------- */
#define ADCHISTO_MAX	4000
#define NUMBERBINS	200
#define NUMBERDATAPTS	2000000
unsigned int bin[NUMBERBINS];
struct ADCHISTO strH = { NUMBERBINS, ADCHISTO_MAX, NUMBERDATAPTS, &bin[0] };

/* Do we run without GPS?  
    If so, then do we find the latest time on the SD and start with 
    that?

  Have the polling unit always send out 1 sec time sync msgs?  If so,
    then there is an issue of readjusting the start of second to the gps.

*/

/* Timing sync  */
   // Initialize tim3_su
   // Enable CAN FIFO 1 to receive and handle time sync msgs
   // Sync to 1 sec time sync msgs




/* --------------------- ADC setup and initialization ----------------------------------------------- */
	adc_init_sequence_se();	// Time delay + calibration, then start conversion




/* --------------------- Endless Stuff ----------------------------------------------- */
	while (1==1)
	{


		/* printf rate throttling */
		if ( i++ > 200000)
		{
			i = 0;
			printf("%7u%7u%6d%9d%9u%9u\n\r",ADC1_DR,ADC2_DR,encoder_ctr,encoder_error,encoder_Actr,encoder_Bctr);USART1_txint_send();

		}

		/* Echo incoming chars back to the sender */
		if ((p=USART1_rxint_getline()) != 0)	// Check if we have a completed line
		{ // Here, pointer points to line buffer with a zero terminated line
			switch (*p)			
			{
			case 'x': // Reset counters
				encoder_ctr = 0;	// Shaft encoder running count (+/-)
				encoder_error = 0;// Cumulative count--shaft encoding errors
				encoder_Actr = 0;	// Cumulative count--photocell A
				encoder_Bctr = 0;	// Cumulative count--photocell B
				USART1_txint_puts("RESET COUNTERS\n\r");USART1_txint_send();
				break;
			case 'h': // Histogram
				adc_histogram(&strH,(u32) ADC1 );
				adc_histogram_print(&strH);
				while ((p=USART1_rxint_getline()) == 0);	// Pause until a key is hit

				break;
			}
		}

	}

	return 0;	
}

