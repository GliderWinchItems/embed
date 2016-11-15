/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : 32KHztest.c
* Hackeroos          : caw, deh
* Date First Issued  : 06/24/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Test program for 32KHz clocking
*******************************************************************************/

/* NOTE: 
Open minicom on the PC with 115200 baud and 8N1 and this routine
*/



#include <math.h>
#include <string.h>

#include "libopenstm32/rcc.h"
#include "libopenstm32/gpio.h"
#include "libopenstm32/usart.h"
#include "libopenstm32/bkp.h"	// Used for #defines of backup register addresses

#include "libopenstm32/systick.h"


#include "libusartstm32/usartallproto.h"
#include "libmiscstm32/systick1.h"



#include "libmiscstm32/printf.h"// Tiny printf
#include "libmiscstm32/clockspecifysetup.h"

#include "PODpinconfig.h"	// gpio configuration for board

#include "spi1ad7799.h"		// spi1 routines 
#include "ad7799_comm.h"	// ad7799 routines (which use spi1 routines)
#include "32KHz.h"		// RTC & BKP routines
#include "pwrctl.h"		// Low power routines

#include "libopenstm32/bkp.h"	// #defines for addresses of backup registers

#define STANDBYTIME	100	// Time for next wakeup in tenth seconds
#define SLEEPDEEPTICKS	(ALR_INCREMENT*STANDBYTIME)/10	// TR_CLK tick count added to current RTC CNT counter for wakeup

/* These are debugging and checking things */
extern unsigned int RTC_debug0;	// Debugging bogus interrupt
extern unsigned int RTC_debug1;	// Debugging 32 KHz osc not setup as expected
extern unsigned int RTC_debug2;	// Type of reset "we think we had"
unsigned int RTC_debug0x;	// Previous value
unsigned int tempb;



/* 'struct CLOCKS clocks' is used to setup the clock source, PLL, dividers, and bus clocks 
See P 84 of Ref Manual for a useful diagram.
../lib/libmiscstm32/clockspecifysetup.h has the 'enum' values that may help for making mistakes(!) */

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


/* RTC registers: PRL, CNT, ALR initial values (see ../devices/32KHz.h, also p 448 Ref Manual) (see ../devices/32KHz.c) */
struct RTCREG strRtc_reg_init = {PRL_DIVIDER,0,ALR_INCREMENT}; // SECF = 1/2 sec, CNT
struct RTCREG 	strRtc_reg_read;	// Readback of registers
unsigned int	 uiRCC_CSR_init;	// RCC_CSR saved at beginning of RTC initialization (see 32KHz.c)
extern void 	(*rtc_secf_ptr)(void);	// Address of function to call during RTC_IRQHandler of SECF (Seconds flag)
extern char	cResetFlag;		// Out of a reset: 1 = 32 KHz osc was not setup; 2 = osc setup OK, backup domain was powered
extern unsigned int	uiSecondsFlag;	// 1 second tick: 0 = not ready, + = seconds count (see 32KHz.c)


/* Variables set by SPI1 operations.  See ../devices/ad7799_comm.c */
extern unsigned char 		ad7799_8bit_reg;	// Byte with latest status register from ad7799_1
extern unsigned char 		ad7799_8bit_wrt;	// Byte to be written
extern volatile unsigned char 	ad7799_comm_busy;	// 0 = not busy with a sequence of steps, not zero = busy
extern unsigned short 		ad7799_16bit_wrt;	// 16 bits to be written
extern union SHORTCHAR 		ad7799_16bit;		// 16 bits read
extern union INTCHAR		ad7799_24bit;		// 24 bits read (high ord byte is set to zero)

		
/* The following variables are used by peripherals in their 'init' routines to set dividers 	*/
/* We might use them here just to display how the clock was set up */
extern unsigned int	hclk_freq;  	/* 	SYSCLKX/HPREDIV	 	E.g. 72000000 	*/
extern unsigned int	pclk1_freq;  	/*	SYSCLKX/PCLK1DIV	E.g. 72000000 	*/
extern unsigned int	pclk2_freq;	/*	SYSCLKX/PCLK2DIV	E.g. 36000000 	*/
extern unsigned int	sysclk_freq;	/* 	SYSCLK freq		E.g. 72000000	*/

/* This if for test sprintf */
char vv[180];	// sprintf buffer

long long llX;	// Big integer for slowing pause loop


/* For tinkering with averaging ad7799 readings */
#define AVESIZE	16		// Number of readings in sum
#define DECIMATECT	16	// Number of readings between decimation
int ad7799_data[AVESIZE];	// Incoming data register readings
int ad7799sum;			// Running summation
int ad7799ave;			// Summation divide by AVESIZE
int ad7799bipolar;		// +/- correction for bipolar mode
unsigned short ixD,uiI;		// Index, and decimation count

/*-------------------------------------------------------------------------------*/
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

/* ****************** POD *************************************
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
//	lednum += 1;		// Step through all four LEDs (comment out for single LED)
	if (lednum > LED6) lednum = LED3;
}
/* ****************** Olimex *********************************/
/* Stupid routine for toggling the gpio pin for the LED	     */
/* ***********************************************************/
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
/* ***********************************************************/
/* Setup the gpio for the LED on the Olimex stm32 P103 board */
/* ***********************************************************/
void gpio_setup(void)
{
	/* Setup GPIO pin for LED (PC12) (See Ref manual, page 157) */
	// 'CRH is high register for bits 8 - 15; bit 12 is therefore the 4th CNF/MODE position (hence 4*4)
	GPIO_CRH(GPIOC) &= ~((0x000f ) << (4*4));	// Clear CNF reset bit 01 = Floating input (reset state)
	GPIO_CRH(GPIOC) |=  (( (GPIO_CNF_OUTPUT_PUSHPULL<<2) | (GPIO_MODE_OUTPUT_2_MHZ) ) << (4*4));
	
}
/* *******************************************************************************/
/* RTC, Seconds, interrupt routine comes here from isr: RTC_IRQHandler (32KHz.c) */
/* *******************************************************************************/
unsigned short usTick;		// Counter of RTCCLK (32768 Hz)divided by prescalar (PRL)
unsigned char ucTflag;		// Flag to mainline for 1/2 sec tick

void RTC_secf_stuff(void)
{
	usTick += 1;		// 32768 Hz divided by 16
	if (usTick > 1024)	// Count 1/2 seconds worth then toggle
	{
		usTick = 0;	
		ucTflag = 1;	// Signal mainline program
	}
	return;
}
/***********************************************************************************************************/
/* And now for the main routine */
/***********************************************************************************************************/
int main(void)
{
	int i;
	u16 temp;
	char* p;		// Temp getline pointer
	u32 ui32;
 
/* ---------- Initializations ------------------------------------------------------ */
	clockspecifysetup(&clocks);		// Get the system clock and bus clocks running

	PODgpiopins_default();	// Set gpio port register bits for low power
	PODgpiopins_Config();	// Now, configure pins

//	gpio_setup();	// Olimex gpio setup for LED

	MAX3232SW_on		// Turn on RS-232 level converter (if doesn't work--no RS-232 chars seen)

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
	if ( (temp = USART1_rxinttxint_init(115200,32,2,48,4)) != 0 )
	{ // Here, something failed 
		return temp;
	}
	


	/* Announce who we are */
	USART1_txint_puts("\n\r32KHztest 07-02-2011 r1\n\r");  USART1_txint_send();// Start the line buffer sending

	/* See what we have in the RTC registers before we do anything with them */	
	RTC_reg_read(&strRtc_reg_read);		// Read-back of register settings
	printf ("RTC reg before   : PRL 0x%08x, CNT 0x%08x, ALR 0x%08x\n\r",strRtc_reg_read.prl,strRtc_reg_read.cnt,strRtc_reg_read.alr);	USART1_txint_send();

	/* Setup the RTC osc and bus interface */
	rtc_secf_ptr = &RTC_secf_stuff;		// RTC_ISRHandler goes to this address for further work
	uiRCC_CSR_init = Reset_and_RTC_init();	// Save register settings

	/* Show what the power up initialization saw as the cause of the reset */
	ui32 = cResetFlag;	// tiny printf only does unsigned ints
	printf ("debug2: 0x%08x  cFlag: %02x\n\r", RTC_debug2,ui32);	USART1_txint_send();
	printf ("RCC_CSR: 0x%08x\n\r",uiRCC_CSR_init);	USART1_txint_send();	// RCC_CSR before routine did anything

	/* See what we have in the RTC registers after 'init routine  */	
	RTC_reg_read(&strRtc_reg_read);		// Read-back of register settings
	printf ("RTC reg after init: PRL 0x%08x, CNT 0x%08x, ALR 0x%08x\n\r",strRtc_reg_read.prl,strRtc_reg_read.cnt,strRtc_reg_read.alr);	USART1_txint_send();

	if (cResetFlag == 1)	// After reset was the 32KHz osc up & running? (in 32KHz.c see Reset_and_RTC_init)
	{ // Here, no.  It must have been a power down/up reset, so we need to re-establish the back domain
		/* Load the RTC registers */
		RTC_reg_load (&strRtc_reg_init);	// Setup RTC registers
		USART1_txint_puts("Re-establishing backup domain\n\r");	USART1_txint_send();	// Something for the hapless op
		BKP_DR1 = 0;	// Extended seconds counter
	}
	else
	{
		/* Compute and set the next alarm register */
		strRtc_reg_read.alr =	((strRtc_reg_read.cnt & ~((1<<11)-1)) +(1<<11));
		RTC_reg_load_alr(strRtc_reg_read.alr);
		USART1_txint_puts("Backup domain was good\n\r");	USART1_txint_send();	// Something for the hapless op
	}
	/* Compute elapsed seconds */
	ui32 = ((strRtc_reg_read.cnt>>ALR_INC_ORDER) | (BKP_DR1>>PRL_DIVIDE_ORDER));	// Combine high order bits with CNT register
	printf ("Elapsed seconds: %9u\n\r",ui32);

	/* Check if the RTC register setup OK */	
	RTC_reg_read(&strRtc_reg_read);		// Read-back of register settings
	printf ("RTC reg after lod:PRL 0x%08x, CNT 0x%08x, ALR 0x%08x\n\r",strRtc_reg_read.prl,strRtc_reg_read.cnt,strRtc_reg_read.alr);	USART1_txint_send();

	/* Direct backup register test */
	BKP_DR3 = 0x1234;
	BKP_DR4 = 0x5678;
	tempb = (BKP_DR4 & 0xffff) | (BKP_DR3 << 16);
	printf ("TEST:BKP_DR3 DR4 reads as %08x and should be 0x12345678\n\r",tempb);	USART1_txint_send();
	

	

/* ---------- End Initializations -------------------------------------------------- */

/* --------------------- Flash LEDs a number of times ----------------------------------------------- */
	i = 0;
	while (1==1)
	{
		/*LED blink rate based on RTC flag */
//		if ( ucTflag == 1)
		if ( uiSecondsFlag != 0)
		{
//			ucTflag = 0;
			uiSecondsFlag -= 1;
//			toggle_led();	// Olimex single LED
			toggle_4leds();	// POD board LEDs

			/* Check if the RTC register setup OK */	
			RTC_reg_read(&strRtc_reg_read);		// Read-back of register settings
			ui32 = BKP_DR1;	// Alarm extension
			i +=1;
//			if (i >= 4)
//			{
//				Powerdown_to_standby( SLEEPDEEPTICKS  );	// Set ALR wakeup, setup STANDBY mode.
//				USART1_txint_puts("WFE should not come here. Z\n\r");	USART1_txint_send();	// Something for the hapless op
//				while (1==1);	// Should not come here
//			}
			printf ("%3u RTC reg: CNT %8u, ALR 0x%08x, DR1 0x%04x\n\r",i,strRtc_reg_read.cnt,strRtc_reg_read.alr, ui32);USART1_txint_send();

		}
		
		if (RTC_debug0x != RTC_debug0)
		{
			printf ("RTC_debug0: %9u\n\r",RTC_debug0); 	USART1_txint_send();
			RTC_debug0x = RTC_debug0;
		}

		/* Echo incoming chars back to the sender */
		if ((p=USART1_rxint_getline()) != 0)	// Check if we have a completed line
		{ // Here, pointer points to line buffer with a zero terminated line
			if (*p == 'x') break;		// Quit when x is the first char of the line
//			USART1_txint_puts(p);		// Echo back the line just received
//			USART1_txint_puts ("\n");	// Add line feed to make things look nice
//			USART1_txint_send();		// Start the line buffer sending
		}

	}
	Powerdown_to_standby( SLEEPDEEPTICKS  );	// Set ALR wakeup, setup STANDBY mode.
	USART1_txint_puts("WFE should not come here\n\r");	USART1_txint_send();	// Something for the hapless op
	while (1==1);	// Should not come here
	return 0;	

}

