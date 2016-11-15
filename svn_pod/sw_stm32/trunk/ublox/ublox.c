/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : ublox.c
* Hackeroos          : deh
* Date First Issued  : 03/05/2013
* Board              : STM32F103VxT6_pod_mm (USART1) or Olimex P103 (USART1)
* Description        : USART1<->UART5 for configuring ublox GPS
*******************************************************************************/
/*
This routine does a "pass-through" between UART5 and USART1.  USART1 connects to
the PC via a RS-232 serial port.  UART5 connects to the u-blox gps at the 3.3v 
ttl level.

If the baud rate is changed the 'u-blox control center' program will see the unit
as unresponsive, in which case this program has to have the baud rate changed to
match the new value.

Note that if the battery of the u-blox drops too low the startup is from the flash.
When a power-down/up restart is done and the battery is holding RAM values are used,
if the 'CFG-CFG' to RAM was done.  However, if the battery is too low, e.g. overnight
unpowered, the startup is from flash.  So "permanent" changes need to be stored with
the 'CFG-CFG' command with I2C eeprom specified.
*/


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

//#define POD	// POD versus sensor boards


/* ----------------- Clocking -------------------------------------------------- */

/* (@2) 'struct CLOCKS clocks' is used to setup the clock source, PLL, dividers, and bus clocks 
See P 84 of Ref Manual for a useful diagram.
../lib/libmiscstm32/clockspecifysetup.h has the 'enum' values that may help for making mistakes(!) */
/* NOTE: Bus for ADC (APB2) must not exceed 14 MHz */
const struct CLOCKS clocks = { \
HSOSELECT_HSE_XTAL,	/* Select high speed osc 			*/ \
PLLMUL_6X,		/* Multiplier PLL: 0 = not used 		*/ \
1,			/* Source for PLLMUL: 0 = HSI, 1 = PLLXTPRE (1 bit predivider)	*/ \
0,			/* PLLXTPRE source: 0 = HSE, 1 = HSE/2 (1 bit predivider on/off)	*/ \
APBX_2,			/* APB1 clock = SYSCLK divided by 1,2,4,8,16; freq <= 36 MHz */ \
APBX_4,			/* APB2 prescalar code = SYSCLK divided by 1,2,4,8,16; freq <= 72 MHz */ \
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
		USART1_txint_putc(c);
	}
/*****************************************************************************************/
/* Setup the gpio for the LED on the Olimex stm32 P103 board */
/*****************************************************************************************/
void gpio_setup(void)
{
//	PODgpiopins_Config();	// Setup the pins for the STM32F103VxT6_pod_mm
	/* Setup GPIO pin for LED (PC12) (See Ref manual, page 157) */
	// 'CRH is high register for bits 8 - 15; bit 12 is therefore the 4th CNF/MODE position (hence 4*4)
	GPIO_CRH(GPIOC) &= ~((0x000f ) << (4*4));	// Clear CNF reset bit 01 = Floating input (reset state)
	GPIO_CRH(GPIOC) |=  (( (GPIO_CNF_OUTPUT_PUSHPULL<<2) | (GPIO_MODE_OUTPUT_2_MHZ) ) << (4*4));
	
}
/*****************************************************************************************/
/* Stupid routine for toggling the gpio pin for the LED */
/*****************************************************************************************/
/* LED identification

|-number on pc board below LEDs
|   |- color
v vvvvvv  macro
3 green   
4 red
5 green
6 yellow
*/


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
/* =============================== MAIN ================================================ */
/*****************************************************************************************/
/* And now for the main routine */
/*****************************************************************************************/
/* =============================== MAIN ================================================ */
int main(void)
{
	char c;

/* ---------- Initializations ------------------------------------------------------ */
	clockspecifysetup((struct CLOCKS *)&clocks);// Get the system clock and bus clocks running
	gpio_setup();		// Need this to make the LED work
	init_printf(0,putc);	// This one-time initialization is needed by the tiny printf routine
	ENCODERGPSPWR_on;	// Turn on power to GPS

	// Initialize USART and setup control blocks and pointer
#define BAUD 57600
//#define BAUD 38400
#define BAUD1 BAUD
//#define BAUD1 115200

#ifdef POD
	UART5_txmin_init(BAUD);
	UART5_rxmin_init(BAUD);
#else
	USART3_txmin_init(BAUD);
	USART3_rxmin_init(BAUD);
#endif
	USART1_txmin_init(BAUD1);
	USART1_rxmin_init(BAUD1);

	/* Announce who we are */
	USART1_txmin_puts("\n\ru-blox 05-06-2014\n\r");


	while (1==1)
	{
		if (USART1_rxmin_rxready() != 0)
		{ // Here, we have a USART1 char
			c = USART1_rxmin_getchar();
		#ifdef POD
			UART5_txmin_putc(c);
		#else
			USART3_txmin_putc(c);
		#endif

		}
	#ifdef POD
		if (UART5_rxmin_rxready() != 0)
		{
			c = UART5_rxmin_getchar();
	#else
		if (USART3_rxmin_rxready() != 0)
		{
			c = USART3_rxmin_getchar();
	#endif
			USART1_txmin_putc(c);
		}
	}

}

