/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : xbloopbk_p
* Person             : deh
* Date First Issued  : 03/17/2012
* Board              : STM32F103VxT6_pod_mm
* Description        : Test program to loopbk two XBee units POD END of link
*******************************************************************************/

/* 

Open minicom on the PC with 115200 baud and 8N1.

*/
/*
XBee pins of interest--
PB XB	XB wire	Signal
5   1	red	3.3v reg enable--XBee : gpio out	(TIM3 CH2*)(SPI3_MOSI)(SPI1_MOSI*)(I2C1_SMBAI)
8   9	white	XBee /DTR/SLEEP_RQ: gpio_out 		(CANRX*)(I2C2_SMBAI)
9		XBee /RESET: gpio_out 			(CANTX*)

PD
3  12	white	Xbee /CTS:	USART2_CTS*
4  16	yellow	Xbee /RTS:	USART2_RTS*
5   3	yellow	Xbee DIN/CONFIG:USART2_TX*
6   2	white	Xbee DOUT: 	USART2_RX*


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
#include "pinconfig.h"
#include "libmiscstm32/clockspecifysetup.h"

/* Parameters for setting up the clocking */
/* PLLMUL_6X = 6 * 8 Mhz = 48 MHz clock */
const struct CLOCKS clocks = { \
HSOSELECT_HSE_XTAL,	/* Select high speed osc 			*/ \
0,		/* Multiplier PLL: 0 = not used 		*/ \
1,			/* Source for PLLMUL: 0 = HSI, 1 = PLLXTPRE (1 bit predivider)	*/ \
0,			/* PLLXTPRE source: 0 = HSE, 1 = HSE/2 (1 bit predivider on/off)	*/ \
APBX_1,			/* APB1 clock = SYSCLK divided by 1,2,4,8,16; freq <= 36 MHz */ \
APBX_1,			/* APB2 prescalar code = SYSCLK divided by 1,2,4,8,16; freq <= 72 MHz */ \
AHB_1,			/* AHB prescalar code: SYSCLK/[2,4,8,16,32,64,128,256,512] (drives APB1,2) */ \
8000000			/* Oscillator source frequency, e.g. 8000000 for an 8 MHz xtal on the external osc. */ \
};


int	nCode;		// Return code for USART2 remapping
char pp[192];		// USART1 -> USART2 buffer


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
/* ************************************************************
void delay_systick(unsigned int nDelay)
XBee command guard time delay
arg = time delay in milliseconds
***************************************************************/
void delay_systick(unsigned int nDelay)
{
	unsigned int systick1;
	systick1 = SYSTICK_getcount32() - (nDelay*(sysclk_freq/1000));	// Compute tick count to assure delay
	while ( ((int)(SYSTICK_getcount32()- systick1)) > 0);	// Time loop
	return;
}
/* ************************************************************
void send_xb_cmd(char * p)
Send command(s) to XB
arg = pointer to string to send
***************************************************************/
void send_xb_cmd(char * p)
{
	USART1_txint_puts(p);		// Echo back the line just received
	USART1_txint_puts ("\n");	// Add line feed to make things look nice
	USART1_txint_send();		// Send to USART1

	/* Send command out USART2* */
	USART2_txint_puts("+++");	// Sequence to enter command mode 
	USART2_txint_send();		// Send Command Mode Sequence
	delay_systick(600);		// No-character guard delay following "+++"
	USART2_txint_puts(p);		// Send command to XBee
	delay_systick(600);		// No-character guard delay following command
	USART2_txint_puts("\r");	// Add <CR> (that triggers response?)
	USART2_txint_send();		// Send to XBee

	return;
}

/***************************************************************************************************
And now for the main routine 
****************************************************************************************************/
int main(void)
{
	int i = 0; 		// Timing loop variable
	int j;			// Another (fixed pt FORTRAN!) variable
	char* p;		// Temp getline pointer


/* --------------------- Begin setting things up -------------------------------------------------- */ 
	clockspecifysetup((struct CLOCKS *)&clocks);	// Get the system clock and bus clocks running

	PODgpiopins_default();	// Set gpio port register bits for low power
	PODgpiopins_Config();	// Now, configure pins
	MAX3232SW_on		// Turn on RS-232 level converter (if doesn't work--no RS-232 chars seen on USART1)

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
	USART1_rxinttxint_init(115200,150,2,150,2); // Initialize USART and setup control blocks and pointers

	/* Announce who we are */
	USART1_txint_puts("\n\rxbloopbk_w XBee loop back POD-END 03-17-2012 \n\r");
	USART1_txint_send();	// Start the line buffer sending


	
	/* Xbee set up */
	/* USART2* connects the the XBee */
	if ( (nCode=USART2_rxinttxint_init(  9600,150,2,150,2) ) != 0) 	// Initialize USART and setup control blocks and pointers
	{
		printf("\n\rUSART2 initialize failed, %d\n\r",nCode); USART1_txint_send();
	}
	if ( (nCode = USARTremap (USART2, 1)) != 0)	// Remap pins
	{
		printf("\n\rUSART2 remap failed code = %d\n\r",nCode);	USART1_txint_send();
	}


	/* Inidividual pins */
	XBEE_RESET_hi;		// Be sure /RESET is de-asserted (to use Digi's terminology)
	XBEESLEEPRQ_hi;		// Be sure /SLEEP is de-asserted
	XBEEREG_on;		// Enable regulator to turn on power to XB module
	configure_pin_input_pull_up_dn ( (volatile u32 *)GPIOD, 3, 1); // Configures PD3, /CTS from XBee, for input w pull up
	configure_pin ( (volatile u32 *)GPIOD, 4); // Configures PD4, /RTS to XBee, for pushpull output
	delay_systick(600);	// Allow time for power to come up and XBee to initialize


	/* Configure */
	USART1_txint_puts("  Configurating--\n\r");	USART1_txint_send();
	send_xb_cmd("ATDL 1,MY 2,DL,MY,CN");	// Factor reset, Destination address (low byte) 0x0001, my address 0x0002

/* -------------------- Send/receive loop -------------------------------------------------------- */
	USART1_txint_puts("  Enter line to send then <enter>\n\r");	USART1_txint_send();
	while (1==1)
	{
		/*LED blink rate timing */
		if ( i++ > 100000)
		{
			toggle_4leds();
			i = 0; j += 1;
		}

		/* Echo PC chars, and send command to XBee */
		if ((p=USART1_rxint_getline()) != 0)	// Check if we have a completed line from the PC USART
		{ // Here, pointer points to line buffer with a zero terminated line

			/* Echo line from PC back to PC */
			printf ("USART1: ");		USART1_txint_send();	// Identify source of printout
			USART1_txint_puts(p);		// Echo back the line just received
			USART1_txint_puts ("\n");	USART1_txint_send();	// Add line feed to make things look nice
			
			/* Send command out USART2* */
			USART2_txint_puts(p);		// Set up line to XBee
			USART2_txint_send();		// Send it
		}

		/* Send XBee chars to PC */
		if ((p=USART2_rxint_getline()) != 0)	// Check if we have a completed line XBee USART
		{ // Here, pointer points to line buffer with a zero terminated line
			USART1_txint_puts("USART2: ");	// Identify source of printout
			USART1_txint_puts(p);		// Echo back the line just received
			USART1_txint_puts ("\n");	// Add line feed to make things look nice
			USART1_txint_send();		// Start the line buffer sending

			/* Echo line back to winch end XBee */
			USART2_txint_puts(p);		// Echo back the line just received
			USART2_txint_send();		// Start the line buffer sending
		
		}
	}

	return 0;	
}


