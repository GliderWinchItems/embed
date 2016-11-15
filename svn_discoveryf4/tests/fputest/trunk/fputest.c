/******************************************************************************
* File Name          : adctest.c
* Date First Issued  : 01/14/2014
* Board              : Discovery F4
* Description        : Test adc_mc routine
*******************************************************************************/
/* 
This routine tests the adc_mc routine, which reads three ADC channels, and via
DMA stores the values in a three dimensional array.  The first dimension is 2 which
is used for double buffering.  An index for this dimension points to the 1/2 of the
array that is not currently being filled.  The third dimension is the number of
ADC channels in a regular conversion sequence--3.  The 2nd dimension is the number
of sequences buffered during one DMA interrupt--16.

The DMA interrupt switches the buffer index and triggers a lower level interrupt that
executes a cic filter on the buffered data, leaving three resulting values in an array
that is accessed by 'main'.

Note: After the compile and make flash the program usually starts, but usually requires
a push of the reset button to reset ADC/DMA hardware.



*/
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>

#include "xprintf.h"
#include "libopencm3/stm32/f4/adc.h"

#include "libopencm3/stm32/f4/rcc.h"
#include "libopencm3/stm32/f4/gpio.h"
#include "libopencm3/stm32/f4/scb.h"

#include "clockspecifysetup.h"
#include "DISCpinconfig.h"	// Pin configuration for STM32 Discovery board
#include "common_can.h"
#include "panic_leds.h"
#include "bsp_uart.h"
#include "libopencm3/stm32/systick.h"
#include "adc_mc.h"
#include "countdowntimer.h"
#include "adc_internal.h"
#include "f4dreset.h"


#define DTWTIME (*(volatile unsigned int *)0xE0001004)

/* USART|UART assignment for xprintf and read/write */
#define UXPRT	6	// Uart number for 'xprintf' messages
#define USTDO	2	// Uart number for gateway (STDOUT_FILE, STDIIN_FILE)

/* file descriptor */
int fd;


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
sysclk  =  168 MHz.................
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
	lednum -= 1;		// Step through all four LEDs
	if (lednum < 12) lednum = 15;

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
FILE* Fptty6;	// USART6Default_Handler11

/*#################################################################################################
  main routine 
  #################################################################################################*/
int main(void)
{
	int i;

/* --------------------- Auto reset --------------------------------------------------------------- */
//	f4dreset();
/* --------------------- Begin setting things up -------------------------------------------------- */ 
	clockspecifysetup((struct CLOCKS*)&clocks);		// Get the system clock and bus clocks running
/* ---------------------- Set up pins ------------------------------------------------------------- */
	/* Configure pins */
	DISCgpiopins_Config();	// Configure pins
/* ---------------------- Set usb ----------------------------------------------------------------- */
//	usb1_init();	// Initialization for USB (STM32F4_USB_CDC demo package)
	setbuf(stdout, NULL);
/* --------------------- Initialize USART/UARTs ---------------------------------------------------- */
/* Regarding 'fprintf' and 'fopen'--(11-21-2013) this does not work.  'fprintf' (in 'newlib.c') does not call 
   '_write' in newlib_support.c'.  In the meantime the function of 'fprintf' is accomplished by using 'sprintf'
   followed by a 'puts' to send the string to the uart. 

   The strategy is to setup the USART/UART so that it will handle STDOUT, and STDIN, makeing 'printf' etc. work
   directly.  Bulk calls are made to _write, _read in subroutines, and these routines will work with the correct
   usart/uart via the 'fd' that relates the fd to uart control block during the call to _open.  Normally one would
   use 'fprintf' etc., but that isn't working, and this shameful sequence is an interim solution that allows easily
   changing the STDOUT, STDIN uart.

   USART2 and USART6 are shown below.  Select one, or make up one for the USART/UART this will be used.  
   Either DMA or CHAR-BY-CHAR interrupt driven can be used.  DMA for faster high volume loads.

*/
/*	DMA DRIVEN  */
// int bsp_uart_dma_init_number(u32 iuart, u32 baud, u32 rxbuffsize, u32 txbuffsize, u32 dmastreamrx, u32 dmastreamtx, u32 dma_tx_int_priority);
//	bsp_uart_dma_init_number(USTDO, 921600, 256, 256, 5, 6, 0xd0); // Flashing LED's means failed and you are screwed.
//	bsp_uart_dma_init_number(USTDO, 460800, 256, 256, 5, 6, 0xd0); // Flashing LED's means failed and you are screwed.
//	bsp_uart_dma_init_number(USTDO, 230400, 1024, 256, 5, 6, 0x10); // Flashing LED's means failed and you are screwed.
//	bsp_uart_dma_init_number(USTDO, 1Default_Handler1115200, 256, 256, 5, 6, 0xd0); // Flashing LED's means failed and you are screwed.
//	bsp_uart_dma_init_number(UXPRT, 115200, 256, 256, 1, 6, 0xd0); // Flashing LED's means failed and you are screwed.

/*	CHAR-BY-CHAR INTERRUPT DRIVEN  */
// int bsp_uart_init_number(u32 iuart, u32 baud, u32 txbuffsize, u32 rxbuffsize,  u32 uart_int_priority);
//	bsp_uart_int_init_number(USTDO, 460800, 256, 256, 0x30);
//	bsp_uart_int_init_number(USTDO, 230400, 256, 256, 0x40);
//	bsp_uart_int_init_number(USTDO, 115200, 256, 256, 0x10);
	bsp_uart_int_init_number(UXPRT, 115200, 256, 256, 0x30);

/* Setup STDOUT, STDIN (a shameful sequence until we sort out 'newlib' and 'fopen'.)  The following 'open' sets up 
   the USART/UART that will be used as STDOUT_FILENO, and STDIN_FILENO.  Don't call 'open' again!  */
//	fd = open("tty2", 0,0); // This sets up the uart control block pointer versus file descriptor ('fd')
//	fd = open("tty6", 0,0); // This sets up the uart control block pointer versus file descriptor ('fd')
/* ---------------------- DTW sys counter -------------------------------------------------------- */

	/* Use DTW_CYCCNT counter (driven by sysclk) for polling type timing */
/* CYCCNT counter is in the Cortex-M-series core.  See the following for details 
http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0337g/BABJFFGJ.html */
	*(volatile unsigned int*)0xE000EDFC |= 0x01000000; // SCB_DEMCR = 0x01000000;
	*(volatile unsigned int*)0xE0001000 |= 0x1;	// Enable DTW_CYCCNT (Data Watch cycle counter)

/* ---------------------- Let the hapless Op know it is alive ------------------------------------ */
	/* Do this several times because it takes the PC a while to recognize and start 'ttyACM0' and some of
           the chars are missed.  No such problem with ttyUSBx, however. */
	for (i = 0; i < 1; i++) 
	{
		/* Announce who we are. ('xprintf' uses uart number to deliver the output.) */
		xprintf(UXPRT,  " \n\rFPUTEST: 08-24-2014  v0.....................................\n\r");
		/* Make sure we have the correct bus frequencies */
		xprintf (UXPRT, "   hclk_freq (Hz) : %9u...............................\n\r",  hclk_freq);	
		xprintf (UXPRT, "  pclk1_freq (Hz) : %9u...............................\n\r", pclk1_freq);	
		xprintf (UXPRT, "  pclk2_freq (Hz) : %9u...............................\n\r", pclk2_freq);	
		xprintf (UXPRT, " sysclk_freq (Hz) : %9u...............................\n\r",sysclk_freq);
	}
/* --------------------- Timing tests ---------------------------------------------------------------------------------- */

volatile  double x = 0.125;
  xprintf (6,"\natan (0.125 ):%.20f\n\r", atan (x));
  xprintf (6,  "atanf(0.125f):%.20f\n\r\n", atanf (0.125f));



volatile unsigned int t0,t1,t2,t3;
volatile unsigned int tdly;
#define NLOOP	10
float xf = 0;
double xx = 0;
float r;
double rr;

	xprintf(UXPRT,"ct        arg       atanf     ticks         atan      ticks\n\r");
	for (i = 0; i < NLOOP+1; i++) 
	{
		for (tdly = 0; tdly < 1000000; tdly++);
		t0 = DTWTIME;
		r = atanf(xf);
		t1 = DTWTIME;

		t2 = DTWTIME;
xx = xf;
		rr = atan(xx);	
		t3 = DTWTIME;

//xprintf(6," %032llx %12.10e\n\r",*((unsigned long long*)&rr),rr);

		xprintf(UXPRT,"%2d %13.11f %11.9f %5d   %16.14Lf %5d %7.3e\n\r",i+1, xf, r,(t1-t0),rr,(t3-t2), (rr-r) );
		xf  += 0.125;
//		xx +=  (1<<i)*0.01;
	}
	
	x = 0; xx = 0;
	xprintf(UXPRT,"ct        arg       tanf      ticks          tan      ticks\n\r");
	for (i = 0; i < NLOOP+1; i++) 
	{
		for (tdly = 0; tdly < 1000000; tdly++);
		t0 = DTWTIME;
		r = tanf(x);
		t1 = DTWTIME;

		t2 = DTWTIME;
xx = x;
		rr = tan(xx);	
		t3 = DTWTIME;

		xprintf(UXPRT,"%2d %13.9f %11.9f %5d   %16.14Lf %5d\n\r",i+1, x, r,(t1-t0),rr,(t3-t2) );
		x  += (((3.1415926535897932)/2)/NLOOP);
//		xx += (((3.14159265358979323)/2)/NLOOP);
	}
	xf=0; xx=0;
	xprintf(UXPRT,"\n\rct        arg       sinf     ticks         sin      ticks\n\r");
	for (i = 0; i < NLOOP+1; i++) 
	{
		for (tdly = 0; tdly < 1000000; tdly++);
		t0 = DTWTIME;
		r = sinf(xf);
		t1 = DTWTIME;

		t2 = DTWTIME;
xx = xf;
		rr = sin(xx);	
		t3 = DTWTIME;

		xprintf(UXPRT,"%2d %13.11f %11.9f %5d  %13.11Lf %16.14f %5d %7.3e\n\r",i+1, xf, r,(t1-t0),xx, rr,(t3-t2), (rr-r) );
		xf  += (((3.1415926535897932)/4)/NLOOP);
//		xx += (((3.14159265358979323)/4)/NLOOP);

	}

/* --------------------- Endless loop prep  ---------------------------------------------------------------------------- */
#define FLASHCOUNT 42000000;	// LED flash
u32	t_led = DTWTIME + FLASHCOUNT; // Set initial time
/* --------------------- Endless loop follows -------------------------------------------------------------------------- */
	while (1==1)
	{
		/* Flash the LED's to amuse the hapless Op, or signal the wizard programmer that the loop is running. */
		if (((int)(DTWTIME - t_led)) > 0) // Has the time expired?
		{ // Here, yes.
			toggle_4leds(); 	// Advance some LED pattern
			t_led += FLASHCOUNT; 	// Set next toggle time

		}
	}
	return 0;	
}

