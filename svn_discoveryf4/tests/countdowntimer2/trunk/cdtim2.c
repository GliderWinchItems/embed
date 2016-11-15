/******************************************************************************
* File Name          : cdtim2.c
* Date First Issued  : 01/19/2014
* Board              : Discovery F4
* Description        : Countdown timers with switchdebounce2 (SPDT & different time logic)
*******************************************************************************/
/* 
This routine is a test/development/demo program for countdown timers--
'countdowntimer.c' [working 1/15/2014: rev 180]
   and switch debouncing--
'switchdebounce.c' [not implemented 1/15/2014: rev 180]

01-18-2014 rev183 spi2rw fixes, switchdebounce working with onboard Discovery F4 pb.

01-19-2014 rev184 Hack of cdtim.c that works with switchdebounce2 where the debounce
         timing is changed to show the switch change upon the first change, followed
         by a timeout where no change will can take place.  It also implements SPDT
         switches where two pins are used for the NC and NO contacts.  In all cases
         the port/pins can be either gpio or spi parallel-serial.

*/
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include "xprintf.h"
#include <malloc.h>

#include "libopencm3/stm32/f4/rcc.h"
#include "libopencm3/stm32/f4/gpio.h"
#include "libopencm3/stm32/f4/scb.h"

#include "systick1.h"
#include "clockspecifysetup.h"

#include "DISCpinconfig.h"	// Pin configuration for STM32 Discovery board

#include "panic_leds.h"
#include "libopencm3/stm32/systick.h"

#include "countdowntimer.h"
#include "switchdebounce2.h"
#include "bsp_uart.h"

//Don't use: #include "usb1.h" because it will bring in ST defines that are conflict with libopencm3.
//void usb1_init(void);	// This is the only link to the usb.  The rest is in 'syscalls.c'

/* USART|UART assignment for xprintf and read/write */
#define UXPRT	6	// Uart number for 'xprintf' messages
#define USTDO	2	// Uart number for gateway (STDOUT_FILE, STDIIN_FILE)

/* file descriptor */
int fd;


/* Subroutine prototypes */
void red_led  (struct COUNTDOWNTIMER* p);
void blue_led (struct SWITCHDEBVARS* p);

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


/* Debugging help. */
/* This mess is useful for identifying interrupts where there is no interrupt handler in the code.
   or the name is a mismatch.  The number that identifies the interrupt vector is stored in
   'Default_HandlerCode' and 'panic_leds(5)' repetiviely flashes all four leds in groups of 5 flashes.
   Stop by using gdb, and display the value in 'Default_Handler'.  */
int Default_HandlerCode = 999;  // Distinctive code that shows nothing stored.
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


/* ***** LED  ***********************************************************
Discovery F4 LEDs: PD 12, 13, 14, 15

|-number on pc board below LEDs */
#define GREENLED  12
#define ORANGELED 13
#define REDLED	  14
#define BLUELED	  15

/* Pin configuration to use pin as gp output.  For the options see--
'../svn_discoveryf4/sw_discoveryf4/trunk/lib/libopencm3/stm32/f4/gpio.h' */
// LED pins are configured with the following options--
const struct PINCONFIG	outpp = { \
	GPIO_MODE_OUTPUT,	// mode: output 
	GPIO_OTYPE_PP, 		// output type: push-pull 		
	GPIO_OSPEED_100MHZ, 	// speed: highest drive level
	GPIO_PUPD_NONE, 	// pull up/down: none
	0 };		// Alternate function code: not applicable
/* ************************************************************
Toggle 'lednum' on/off (Port D only)
***************************************************************/
void toggle_led (int lednum)	// Toggle orange led on/off
{
	if ((GPIO_ODR(GPIOD) & (1<<lednum)) == 0)
	{ // Here, LED bit was off
		GPIO_BSRR(GPIOD) = (1<<lednum);	// Set bit
	}
	else
	{ // HEre, LED bit was on
		GPIO_BSRR(GPIOD) = (1<<(lednum+16));	// Reset bit
	}
	return;
}

/* ********** Parallel to serial buffer images **************************/
// Parallel-Serial output & input expansion, via SPI and shift registers
#define SPI2BUFFSIZE	3	// Number of 8 bit shift registers
char swdbout[SPI2BUFFSIZE];	// Buffer holding image of parallel output
char swdbin [SPI2BUFFSIZE];	// Buffer holding image of parallel input

/* ********** Pushbuttons to be debounced *******************************/
// PA0 is the Discovery F4 on-board switch (Blue sw). fixed parameters
const struct SWITCHDEBPARAMS my_pushbutton = {\
	100, 	/* u32	topeni;		 Debounce time open (1/2 ms ticks) */\
	100,	/* u32	tclosei;	 Debounce time close (1/2 ms ticks) */\
	(u32*)GPIOA, /* u32	port;	 Address of gpio port */\
	0,  	/* u32	pin;		 Pin on port (0-31) (PA0 = 0) */\
	NULL,	/* u32	port2;		 Port, or for spi the pointer to the byte */\
	0,  	/* u32	pin2;		 Pin on port, or for spi the bit number within byte */\
	2,  	/* u32	cbx;		 Callback is made when flag: 0 = 1->0; 1 = 0->1; 2 = either change  */\
	(void*)&blue_led, /* void (*p)(void) Callback function pointer (to '_vars struct with flag) */\
	0,  	/* U32	updn;		 Use internal pull up/dn or none (0) */\
};
u32* my_pushbutton_flag;	// Pointer to switch variable that holds 'flag' for debounced state

// External switch (spi2 parallel-serial), fixed parameters
const struct SWITCHDEBPARAMS ext_pushbutton1 = {\
	15, 	/* u32	topeni;		 Debounce time open (1/2 ms ticks) */\
	50, 	/* u32	tclosei;	 Debounce time close (1/2 ms ticks) */\
	(u32*)GPIOA, /* u32	port;	 Address of gpio port */\
	0,  	/* u32	pin;		 Pin on port (0-31) (PA0 = 0) */\
	NULL,	/* u32	port2;		 Port, or for spi the pointer to the byte */\
	0,	/* u32	pin2;		 Pin on port, or for spi the bit number within byte */\
	2,	/* u32	cbx;		 Callback is made when flag: 0 = 1->0; 1 = 0->1; 2 = either change  */\
	(void*)&blue_led, /* void (*p)(void) Callback function pointer (to '_vars struct with flag) */\
	0,	/* U32	updn;		 Use internal pull up/dn or none (0) */\
};
u32* ext_pushbutton1_vars;	// Pointer to switch variables, holds 'flag' for debounced state

/* ********* Countdown timers ******************************************/
// Blink orange LED on/off with polling loop check of timer
struct COUNTDOWNTIMER timer_blink_org_led = {\
	0,    /* u32	ctr;			 Countdown counter */ \
	0,    /* u32	flag;			 Flag != 0--count time hit zero */\
	200,  /* u32	rep;			 Countdown repetition (1/2 ms ticks) */ \
	0,    /* u32	lod;			 Load value */ \
	0,    /* void 	(*func)(void)		 Callback pointer */ \
	0,    /* struct COUNTDOWNTIMER*	next;	 Pointer to next struct */ \
};
// Blink red LED on/off by using 'callback' from timer
struct COUNTDOWNTIMER timer_blink_red_led = {\
	0,    /* u32	ctr;			 Countdown counter */ \
	0,    /* u32	flag;			 Flag != 0--count time hit zero */\
	1000,  /* u32	rep;			 Countdown repetition (1/2 ms ticks) */ \
	0,    /* u32	lod;			 Load value */ \
	(void*)&red_led,    /* void 	(*func)(void)	 Callback pointer */ \
	0,    /* struct COUNTDOWNTIMER*	next;	 Pointer to next struct */ \
};  
// Time printf messages
struct COUNTDOWNTIMER timer_printfmsg = {\
	0,     /* u32	ctr;			 Countdown counter */\
	0,     /* u32	flag;			 Flag != 0--count time hit zero */\
	0,     /* u32	rep;			 Countdown repetition (1/2 ms ticks) */\
	100,     /* u32	lod;			 Load value */\
	0,     /* void 	(*func)(void)		 Callback pointer */\
	0,     /* struct COUNTDOWNTIMER* next;	 Pointer to next struct */\
};  


/*#################################################################################################
  main routine 
  #################################################################################################*/
int main(void)
{
	int i;

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
//	bsp_uart_dma_init_number(USTDO, 115200, 256, 256, 5, 6, 0xd0); // Flashing LED's means failed and you are screwed.
//	bsp_uart_dma_init_number(UXPRT, 115200, 256, 256, 1, 6, 0xd0); // Flashing LED's means failed and you are screwed.

/*	CHAR-BY-CHAR INTERRUPT DRIVEN  */
// int bsp_uart_init_number(u32 iuart, u32 baud, u32 txbuffsize, u32 rxbuffsize,  u32 uart_int_priority);
//	bsp_uart_int_init_number(2, 460800, 256, 256, 0x30);
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
	/* Announce who we are. ('xprintf' uses uart number to deliver the output.) */
	xprintf(UXPRT,  " \n\rF4 DISCOVERY: CDTIM (Test countdown timer and switch debounce) 01-23-2014 v0 ....\n\r");
	/* Make sure we have the correct bus frequencies */
	xprintf (UXPRT, "   hclk_freq (MHz) : %9u...............................\n\r",  hclk_freq/1000000);	
	xprintf (UXPRT, "  pclk1_freq (MHz) : %9u...............................\n\r", pclk1_freq/1000000);	
	xprintf (UXPRT, "  pclk2_freq (MHz) : %9u...............................\n\r", pclk2_freq/1000000);	
	xprintf (UXPRT, " sysclk_freq (MHz) : %9u...............................\n\r",sysclk_freq/1000000);

/* --------------------- Onboard LED setup ----------------------------------------------------------------------------- */
	for (i = 0; i < 4; i++)  f4gpiopins_Config ((volatile u32 *)GPIOD, (12+i), (struct PINCONFIG*)&outpp);
/* --------------------- Initialize switchdebouncing ------------------------------------------------------------------- */
	debouncesw2_init(swdbout, swdbin, SPI2BUFFSIZE);	// SPI out/in buffers
/* --------------------- Add pushbutton(s) to be debounced --------------------------------------------------------------- */
	// Add switch parameters to list and receive pointer to 'flag' that represents the debounced switch
	my_pushbutton_flag = debouncesw2_add(&my_pushbutton);	// Add on-board Discovery F4 pushbutton on port A pin 0
	if (my_pushbutton_flag == NULL)
		{xprintf (UXPRT, "Error upon adding: my_pushbutton\n\r"); panic_leds(7);} // Bombed out, so hang flasing leds
/* --------------------- Initialize timer ------------------------------------------------------------------------------ */
/* @brief	: Initialize for switch debouncing of parallel-serial extension
 * @param	: char *pout = pointer to byte array with bytes to output
 * @param	: char *pin  = pointer to byte array to receive bytes coming in
 * @param	: int count  = byte count of number of write/read cycles                                                 */
	timer_debounce_init();
/* --------------------- Add count down timers ------------------------------------------------------------------------- */
	countdowntimer_add(&timer_blink_org_led);
	countdowntimer_add(&timer_blink_red_led);
	countdowntimer_add(&timer_printfmsg);

/* --------------------- Endless loop follows -------------------------------------------------------------------------- */

//	u32 msgct = 0;	// Sequence number
	u32 td = 250;	// Beginning time delay for printf's
	u32 flag_prev = 0;
	u32 flag_prev2 = 0;
extern int x2flag;

	while(1==1)	// Endless polling loop
	{
		/* Repetition demo, polling */
		if (timer_blink_org_led.flag != flag_prev)
		{ // Here, time has expired.
			flag_prev = timer_blink_org_led.flag;
			toggle_led (ORANGELED);	// Toggle orange led on/off
xprintf (UXPRT,"%5i\n\r",x2flag);
		}
		/* One-shot demo, polling, with loading new and increasing count each timeout. */
		if (timer_printfmsg.flag != flag_prev2)
		{ // Here, time has expired.
			timer_printfmsg.flag = flag_prev2;
			timer_printfmsg.lod = td;	// Load new count
			toggle_led (GREENLED);		// Toggle green LED
//			xprintf (UXPRT,"%5i %5i\n\r",msgct++, td);
			td += 250;	// Increase delay with each msg.
		}
	}
	return 0;	
}

/* ********************************************************************************************************************
 * void red_led (struct COUNTDOWNTIMER* p);
 * @brief	: Demonstrate call-back from countdown timer: Blink red LED
 **********************************************************************************************************************/
/* This demos a CALL BACK.  The timer interrupt routine calls this routine when the time count goes
   to zero.  The setup struct has 'rep' (repetitive) time count set, so the timer automatically reloads.
   
   NOTE:  Very important.  When this routine executes it is running under interrupt.  If higher priority
   interrupts occur they will interrupt this routine, and eventually return.  You must not try to change global variables
   in any routine that does not run under this interrupt level.  What can happen is that this interrupt occurs just
   when another routine is in the process of a load and store of the variable, and it is caught with the 'load' having
   been executed, but not the 'store'.  This routine might try to set that variable to some value, e.g. zero, but when 
   it exits the store in the routine that was interrupted completes and the value this routine set is overwritten.
*/
void red_led (struct COUNTDOWNTIMER* p)
{
	/* We can call 'toggle_led' because it is a re-entrant routine...as long as we
           don't call it with the same LED number */
	toggle_led (REDLED);

	return;
}
/* ********************************************************************************************************************
 * void blue_led (struct SWITCHDEBVARS* p);
 * @brief	: Demonstrate call-back from countdown timer: Turn on blue led when debounced on-board pushbutton is pushed
 **********************************************************************************************************************/
/* This demos a CALL BACK.  The timer interrupt routine calls this routine when the debounced switch has been determined
to be transition OFF to ON, or vice-versa.
*/
void blue_led (struct SWITCHDEBVARS* p)
{
	if (p->flag != 0) // Check debounced state of switch
	{ // Here, debounced switch transitioned to ON
		GPIO_BSRR(GPIOD) = (1<<BLUELED);	// Set bit
	}
	else
	{ // Here, debounced switch transitioned to OFF
		GPIO_BSRR(GPIOD) = (1<<(BLUELED+16));	// Reset bit
	}

	return;
}

