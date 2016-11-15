/******************************************************************************
* File Name          : uarttest.c
* Date First Issued  : 11/06/2013
* Board              : Discovery F4
* Description        : Test F4 uart routines
*******************************************************************************/
/* 
11-23-2013: svn_discoveryf4, rev 119, appears to work for dma and interrupt.  Tested
            with USART2 (DMA1) and USART6 (DMA2)
*/
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <sys/types.h>
#include <sys/fcntl.h>

#include "libopencm3/stm32/f4/rcc.h"
#include "libopencm3/stm32/f4/gpio.h"
#include "libopencm3/stm32/f4/scb.h"
#include "libopencm3/stm32/f4/usart.h"

#include "systick1.h"
#include "clockspecifysetup.h"

#include "DISCpinconfig.h"	// Pin configuration for STM32 Discovery board

#include "canwinch_ldr.h"
#include "panic_leds.h"
#include "PC_gateway_comm.h"
#include "USB_PC_gateway.h"
#include "CAN_gateway.h"
#include "bsp_uart.h"
#include "default_irq_handler.h"

#include "libopencm3/stm32/systick.h"

//Don't use: #include "usb1.h" because it will bring in ST defines that are conflict with libopencm3.
void usb1_init(void);	// This is the only link to the usb.  The rest is in 'syscalls.c'



/* Why is this needed!? Not needed in Charlie's 'my_main.c' */
int _read(int file, char *ptr, int len);

/* --------------- For debugging...(usb) ------------------------------ */
int Default_HandlerCode = 999;
u32 DH08;
void Default_Handler08(void) {DH08 += 1; return;}

void OTG_FS_IRQHandler(void);
void Default_Handler76(void) {	OTG_FS_IRQHandler(); return; }

void Default_Handler00(void) { Default_HandlerCode =  0; panic_leds(5); }
void Default_Handler01(void) { Default_HandlerCode =  1; panic_leds(5); }
void Default_Handler02(void) { Default_HandlerCode =  2; panic_leds(5); }
void Default_Handler03(void) { Default_HandlerCode =  3; panic_leds(5); }
void Default_Handler04(void) { Default_HandlerCode =  4; panic_leds(5); }
void Default_Handler05(void) { Default_HandlerCode =  5; panic_leds(5); }
void Default_Handler06(void) { Default_HandlerCode =  6; panic_leds(5); }
void Default_Handler07(void) { Default_HandlerCode =  7; panic_leds(5); }
//void Default_Handler08(void) { Default_HandlerCode =  8; panic_leds(5); }
void Default_Handler09(void) { Default_HandlerCode =  9; panic_leds(5); }
void Default_Handler10(void) { Default_HandlerCode = 10; panic_leds(5); }
void Default_Handler11(void) { Default_HandlerCode = 11; panic_leds(5); }
void Default_Handler12(void) { Default_HandlerCode = 12; panic_leds(5); }
void Default_Handler13(void) { Default_HandlerCode = 13; panic_leds(5); }
void Default_Handler14(void) { Default_HandlerCode = 14; panic_leds(5); }
void Default_Handler15(void) { Default_HandlerCode = 15; panic_leds(5); }
void Default_Handler16(void) { Default_HandlerCode = 16; panic_leds(5); }
void Default_Handler17(void) { Default_HandlerCode = 17; panic_leds(5); }
void Default_Handler18(void) { Default_HandlerCode = 18; panic_leds(5); }
void Default_Handler19(void) { Default_HandlerCode = 19; panic_leds(5); }
void Default_Handler20(void) { Default_HandlerCode = 20; panic_leds(5); }
void Default_Handler21(void) { Default_HandlerCode = 21; panic_leds(5); }
void Default_Handler22(void) { Default_HandlerCode = 22; panic_leds(5); }
void Default_Handler23(void) { Default_HandlerCode = 23; panic_leds(5); }
void Default_Handler24(void) { Default_HandlerCode = 24; panic_leds(5); }
void Default_Handler25(void) { Default_HandlerCode = 25; panic_leds(5); }
void Default_Handler26(void) { Default_HandlerCode = 26; panic_leds(5); }
void Default_Handler27(void) { Default_HandlerCode = 27; panic_leds(5); }
void Default_Handler28(void) { Default_HandlerCode = 28; panic_leds(5); }
void Default_Handler29(void) { Default_HandlerCode = 29; panic_leds(5); }
void Default_Handler30(void) { Default_HandlerCode = 30; panic_leds(5); }
void Default_Handler31(void) { Default_HandlerCode = 31; panic_leds(5); }
void Default_Handler32(void) { Default_HandlerCode = 32; panic_leds(5); }
void Default_Handler33(void) { Default_HandlerCode = 33; panic_leds(5); }
void Default_Handler34(void) { Default_HandlerCode = 34; panic_leds(5); }
void Default_Handler35(void) { Default_HandlerCode = 35; panic_leds(5); }
void Default_Handler36(void) { Default_HandlerCode = 36; panic_leds(5); }
void Default_Handler37(void) { Default_HandlerCode = 37; panic_leds(5); }
void Default_Handler38(void) { Default_HandlerCode = 38; panic_leds(5); }
void Default_Handler39(void) { Default_HandlerCode = 39; panic_leds(5); }
void Default_Handler40(void) { Default_HandlerCode = 40; panic_leds(5); }
void Default_Handler41(void) { Default_HandlerCode = 41; panic_leds(5); }
void Default_Handler42(void) { Default_HandlerCode = 42; panic_leds(5); }
void Default_Handler43(void) { Default_HandlerCode = 43; panic_leds(5); }
void Default_Handler44(void) { Default_HandlerCode = 44; panic_leds(5); }
void Default_Handler45(void) { Default_HandlerCode = 45; panic_leds(5); }
void Default_Handler46(void) { Default_HandlerCode = 46; panic_leds(5); }
void Default_Handler47(void) { Default_HandlerCode = 47; panic_leds(5); }
void Default_Handler48(void) { Default_HandlerCode = 48; panic_leds(5); }
void Default_Handler49(void) { Default_HandlerCode = 49; panic_leds(5); }
void Default_Handler50(void) { Default_HandlerCode = 50; panic_leds(5); }
void Default_Handler51(void) { Default_HandlerCode = 51; panic_leds(5); }
void Default_Handler52(void) { Default_HandlerCode = 52; panic_leds(5); }
void Default_Handler53(void) { Default_HandlerCode = 53; panic_leds(5); }
void Default_Handler54(void) { Default_HandlerCode = 54; panic_leds(5); }
void Default_Handler55(void) { Default_HandlerCode = 55; panic_leds(5); }
void Default_Handler56(void) { Default_HandlerCode = 56; panic_leds(5); }
void Default_Handler57(void) { Default_HandlerCode = 57; panic_leds(5); }
void Default_Handler58(void) { Default_HandlerCode = 58; panic_leds(5); }
void Default_Handler59(void) { Default_HandlerCode = 59; panic_leds(5); }
void Default_Handler60(void) { Default_HandlerCode = 60; panic_leds(5); }
void Default_Handler61(void) { Default_HandlerCode = 61; panic_leds(5); }
void Default_Handler62(void) { Default_HandlerCode = 62; panic_leds(5); }
void Default_Handler63(void) { Default_HandlerCode = 63; panic_leds(5); }
void Default_Handler64(void) { Default_HandlerCode = 64; panic_leds(5); }
void Default_Handler65(void) { Default_HandlerCode = 65; panic_leds(5); }
void Default_Handler66(void) { Default_HandlerCode = 66; panic_leds(5); }
void Default_Handler67(void) { Default_HandlerCode = 67; panic_leds(5); }
void Default_Handler68(void) { Default_HandlerCode = 68; panic_leds(5); }
void Default_Handler69(void) { Default_HandlerCode = 69; panic_leds(5); }
void Default_Handler70(void) { Default_HandlerCode = 70; panic_leds(5); }
void Default_Handler71(void) { Default_HandlerCode = 71; panic_leds(5); }
void Default_Handler72(void) { Default_HandlerCode = 72; panic_leds(5); }
void Default_Handler73(void) { Default_HandlerCode = 73; panic_leds(5); }
void Default_Handler74(void) { Default_HandlerCode = 74; panic_leds(5); }
void Default_Handler75(void) { Default_HandlerCode = 75; panic_leds(5); }
//void Default_Handler76(void) { Default_HandlerCode = 76; panic_leds(5); }
void Default_Handler77(void) { Default_HandlerCode = 77; panic_leds(5); }
void Default_Handler78(void) { Default_HandlerCode = 78; panic_leds(5); }
void Default_Handler79(void) { Default_HandlerCode = 79; panic_leds(5); }
void Default_Handler80(void) { Default_HandlerCode = 80; panic_leds(5); }
void Default_Handler81(void) { Default_HandlerCode = 81; panic_leds(5); }
void Default_Handler82(void) { Default_HandlerCode = 82; panic_leds(5); }
void Default_Handler83(void) { Default_HandlerCode = 83; panic_leds(5); }
void Default_Handler84(void) { Default_HandlerCode = 84; panic_leds(5); }
void Default_Handler85(void) { Default_HandlerCode = 85; panic_leds(5); }
void Default_Handler86(void) { Default_HandlerCode = 86; panic_leds(5); }
void Default_Handler87(void) { Default_HandlerCode = 87; panic_leds(5); }
void Default_Handler88(void) { Default_HandlerCode = 88; panic_leds(5); }
void Default_Handler89(void) { Default_HandlerCode = 89; panic_leds(5); }
void Default_Handler90(void) { Default_HandlerCode = 90; panic_leds(5); }


/* The following 'struct CLOCKS' values provide --
External 8 MHz xtal
sysclk  =  168 MHz
PLL48CK =   48 MHz
PLLCLK  =  168 MHz
AHB     =  168 MHz
APB1    =   42 MHz
APB2    =   84 MHz

NOTE: PLL48CK *must* be 48 MHz for the USB to work.
*/

const struct CLOCKS clocks = { \
HSOSELECT_HSE_XTAL,	/* Select high speed osc: 0 = internal 16 MHz rc; 1 = external xtal controlled; 2 = ext input; 3 ext remapped xtal; 4 ext input */ \
1,			/* Source for main PLL & audio PLLI2S: 0 = HSI, 1 = HSE selected */ \
APBX_4,			/* APB1 clock = SYSCLK divided by 0,2,4,8,16; freq <= 42 MHz */ \
APBX_2,			/* APB2 prescalar code = SYSCLK divided by 0,2,4,8,16; freq <= 84 MHz */ \
AHB_1,			/* AHB prescalar code: SYSCLK/[2,4,8,16,32,64,128,256,512] (drives APB1,2 and HCLK) */ \
8000000,		/* External Oscillator source frequency, e.g. 8000000 for an 8 MHz xtal on the external osc. */ \
7,			/* Q (PLL) divider: USB OTG FS, SDIO, random number gen. USB OTG FS clock freq = VCO freq / PLLQ with 2 ≤ PLLQ ≤ 15 */ \
PLLP_2,			/* P Main PLL divider: PLL output clock frequency = VCO frequency / PLLP with PLLP = 2, 4, 6, or 8 */ \
84,			/* N Main PLL multiplier: VCO output frequency = VCO input frequency × PLLN with 64 ≤ PLLN ≤ 432	 */ \
2			/* M VCO input frequency = PLL input clock frequency / PLLM with 2 ≤ PLLM ≤ 63 */
};

/* Keyboard input entry line */
char vv[64];



/* ***** LED identification **********************************************
Discovery F4 LEDs: PD 12, 13, 14, 15

|-number on pc board below LEDs
|  |- color
vv vvvvvv 
12 green   
13 orange
14 red
15 blue
 ************************************************************************/
/* ************************************************************
Turn the LEDs on in sequence, then turn them back off 
***************************************************************/
static int lednum = 12;	// Lowest port bit numbered LED
void toggle_4leds (void)
{
	if ((GPIO_ODR(GPIOD) & (1<<lednum)) == 0)
	{ // Here, LED bit was off
		GPIO_BSRR(GPIOD) = (1<<lednum);	// Set bit
	}
	else
	{ // HEre, LED bit was on
		GPIO_BSRR(GPIOD) = (1<<(lednum+16));	// Reset bit
	}
	lednum += 1;		// Step through all four LEDs
	if (lednum > 15) lednum = 12;

}
/* Pin configuration to use pin as gp output.  For the options see--
'../svn_discoveryf4/sw_discoveryf4/trunk/lib/libopencm3/stm32/f4/gpio.h' */
// LED pins are configured with the following options--
const struct PINCONFIG	outpp = { \
	GPIO_MODE_OUTPUT,	// mode: output 
	GPIO_OTYPE_PP, 		// output type: push-pull 		
	GPIO_OSPEED_100MHZ, 	// speed: highest drive level
	GPIO_PUPD_NONE, 	// pull up/down: none
	0 };		// Alternate function code: not applicable

FILE* Fptty2;	// USART2
FILE* Fptty6;	// USART6

/*#################################################################################################
  main routine 
  #################################################################################################*/
int main(void)
{
	int i;
/* --------------------- Begin setting things up -------------------------------------------------- */ 
	clockspecifysetup((struct CLOCKS*)&clocks);	// Get the system clock and bus clocks running

/* --------------------- LED setup ------------------------------------------------------------------------------------- */
	for (i = 0; i < 4; i++) f4gpiopins_Config ((volatile u32 *)GPIOD, (12+i), (struct PINCONFIG*)&outpp);

/* ---------------------- Initialize for usb ------------------------------------------------------ */
	// usb will be used as STDIN, STDOUT, STDERR
//	usb1_init();	// Initialization for USB (STM32F4_USB_CDC demo package)
//	setbuf(stdout, NULL);
/* --------------------- Initialize USART/UARTs ---------------------------------------------------- */
/* Regarding 'fprintf' and 'fopen'--(11-21-2013) this does not work.  'fprintf' (in 'newlib.c') does not call 
   '_write' in newlib_support.c'.  In the meantime the function of 'fprintf' is accomplished by using 'sprintf'
   followed by a 'puts' to send the string to the uart. 
*/
/*	DMA DRIVEN  */
// int bsp_uart_dma_init_number(u32 iuart, u32 baud, u32 rxbuffsize, u32 txbuffsize, u32 dmastreamrx, u32 dmastreamtx, u32 dma_tx_int_priority);
	bsp_uart_dma_init_number(2, 115200, 96, 128, 5, 6, 0xd0); // Flashing LED's means failed and you are screwed.
	bsp_uart_dma_init_number(6, 115200, 96, 128, 1, 6, 0xd0); // Flashing LED's means failed and you are screwed.

/*	CHAR-BY-CHAR INTERRUPT DRIVEN  */
// int bsp_uart_init_number(u32 iuart, u32 baud, u32 txbuffsize, u32 rxbuffsize,  u32 uart_int_priority);
//	bsp_uart_int_init_number(2, 115200, 96, 128, 0xd0);	// e.g. GPS
//	bsp_uart_int_init_number(6, 115200, 96, 128, 0xc0);	// e.g. LCD	


//	Fptty2 = fopen("tty2", "rw"); // e.g. FDTI to PC
//	Fptty2 = fopen("tty2","r+");
//	Fptty6 = fopen("tty6", "rw");


/* --------------------- Initialze 32b system counter ---------------------------------------------- */

	/* Initialize the DTW_CYCCNT counter if it is used for timing.  This is a Cortex-Mx core function
           sometimes used in a debugger, and not a STM32 peripheral timer.  It is 32b and runs off the
	   system clock (168MHz in this demo). */
/* CYCCNT counter is in the Cortex-M-series core.  See the following for details 
http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0337g/BABJFFGJ.html */
	*(volatile unsigned int*)0xE000EDFC |= 0x01000000; // SCB_DEMCR = 0x01000000;
	*(volatile unsigned int*)0xE0001000 |= 0x1;	// Enable DTW_CYCCNT (Data Watch cycle counter)

/* --------------------- A little output to show the program is alive ------------------------------- */
	/* The following is done 3 times since the PC will take a short period of time to recognize
	   /dev/ttyACM0 is present and chars at the beginning are lost. */
//	for (i = 0; i < 3; i++)
//	{
		/* Announce who we are. */
//		printf(" \n\rDISCOVERY F4 GPIO1: 11-07-2013  v0\n\r");

		/* Display the clock and bus frequencies. */
//		printf ("   hclk_freq (MHz) : %9u\n\r",  hclk_freq/1000000);	
//		printf ("  pclk1_freq (MHz) : %9u\n\r", pclk1_freq/1000000);	
//		printf ("  pclk2_freq (MHz) : %9u\n\r", pclk2_freq/1000000);	
//		printf (" sysclk_freq (MHz) : %9u\n\r",sysclk_freq/1000000);
//	}


	bsp_uart_puts_uartnum(2,"\n\r\n\rUSART2 is alive..................\n\r");
	bsp_uart_puts_uartnum(6,"\n\r\n\rUSART6 is alive..................\n\r");


/* --------------------- Endless loop follows -------------------------------------------------------------------------- */
#define PACECOUNT (168000000/100);	// Pace the output loop
u32	t_led = *(volatile unsigned int *)0xE0001004 + PACECOUNT;
u32 t_0;
u32 t_diff1;
u32 t_diff2;
volatile int freturn = 0;
int greturn = 0;
char c;
char vv[128];
const char *pline = "abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ 01234567889 !@#$%^&*()~<>,.:;?-={}\n\r";

	
	/* with FTDI cables plugged into PC and connected to the F4 for the two ports (e.g. UASRT2, USART6),
           Open two minicom windows e.g. sudo minicom /dev/ttyUSB0, and sudo minicom /dev/ttyUSB1. */

	switch (2)
	{
	/* Simple output on both uarts */
	case 1:

		while(1==1)	// Endless polling loop
		{
			/* Flash the LED's to amuse the hapless Op */
			if ( ( (int)(*(volatile unsigned int *)0xE0001004 - t_led) ) > 0)
			{
				toggle_4leds(); 
				t_led += PACECOUNT;

//			 	 printf("%5i  USB\n\r",i);
//				freturn = fprintf(Fptty2, "%5i  USART2\n\r",i);
				freturn = sprintf(vv, "%5i  USART2 %6u ",i,t_diff1);
				t_0 = *(volatile unsigned int *)0xE0001004;
				bsp_uart_puts_uartnum(2,vv);
				bsp_uart_puts_uartnum(2,pline);
				t_diff1 = *(volatile unsigned int *)0xE0001004 - t_0;

//				fprintf(Fptty6, "%5i  USART6\n\r",i);
				freturn = sprintf(vv, "%5i  USART6 %6u ",i,t_diff2);
				t_0 = *(volatile unsigned int *)0xE0001004;
				bsp_uart_puts_uartnum(6,vv);
				bsp_uart_puts_uartnum(6,pline);
				t_diff2 = *(volatile unsigned int *)0xE0001004 - t_0;

				i += 1;
			}
		}
		break;

	/* Type in on one uart (minicom window), and echo on the other uart (minicom window) */
	case 2:
		while(1==1)
		{
			/* Flash the LED's to amuse the hapless Op */
			if ( ( (int)(*(volatile unsigned int *)0xE0001004 - t_led) ) > 0)
			{
				toggle_4leds(); 
				t_led += PACECOUNT;
			}

			if  ((greturn = bsp_uart_getcount_uartnum(2)) > 0)
			{ // Here, we have one or more incoming chars buffered
				for (i = 0; i < greturn; i++)
				{ 
					c = bsp_uart_getc_uartnum(2);

					/* Output to other terminal. */
					bsp_uart_putc_uartnum(6, c);
					if (c == '\r') bsp_uart_putc_uartnum(6, '\n');

					/* Echo to terminal that we are typing in on. */
					bsp_uart_putc_uartnum(2, c);
					if (c == '\r') bsp_uart_putc_uartnum(2, '\n');

				}
			}
			if  ((greturn = bsp_uart_getcount_uartnum(6)) > 0)
			{ // Here, we have one or more incoming chars buffered
				for (i = 0; i < greturn; i++)
				{ 
					c = bsp_uart_getc_uartnum(6);

					/* Output to other terminal. */
					bsp_uart_putc_uartnum(2, c);
					if (c == '\r') bsp_uart_putc_uartnum(2, '\n');

					/* Echo to terminal that we are typing in on. */
					bsp_uart_putc_uartnum(6, c);
					if (c == '\r') bsp_uart_putc_uartnum(6, '\n');
				}
			}
		}


	}
	return 0;	
}

