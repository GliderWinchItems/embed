/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : co1.c
* Hackeroo           : deh
* Date First Issued  : 12/25/2012
* Board              : STM32F103VxT6_pod_mm
* Description        : Use pod board to prototype a sensor logger ### DEH OLIMEX 1 ###
*******************************************************************************/
/* 
01/31/2013 rev 114 Tim4_pod_se.c with 1PPS & OC, but not debugged 
02/09/2013 rev 119 Tim4_pod_se.c copied from ../testing/ICOCOV/trunk

Note: Use PA0 to stop SD card write.  Large pushbutton near SD card--press == hi.  
Pressing stops SD card write for 'x' seconds so that card can be removed.  Use LED
flashing...

USART2 = D connector on Olimex board
USART1 = GPS wired to UEXT connector
 TX = UEXT-3
 RX = UEXT-4
 GPS 1 pps = UEXT-5 (TIM4 CH1)
Open minicom on the PC with 115200 baud and 8N1.

More...

01/21/2014 Rev 252? Last before reworking
01/25/2014 Rev 361  Adding begin debug changes for gps binary fix msgs

*/
#include <math.h>
#include <string.h>
#include <time.h>

/* ../svn_pod/sw_stm32/trunk/lib */
#include "PODpinconfig.h"
#include "common.h"
//#include "libopenstm32/adc.h"
#include "libopenstm32/can.h"
//#include "libopenstm32/rcc.h"
#include "libopenstm32/gpio.h"
#include "libopenstm32/usart.h"
#include "libopenstm32/scb.h"
#include "libusartstm32/usartallproto.h"
//#include "libmiscstm32/systick1.h"
#include "libmiscstm32/printf.h"
#include "libmiscstm32/clockspecifysetup.h"
#include "sdlog.h"

/* ../svn_sensor/sw_f103/trunk/lib */
//#include "adcsensor_pod.h"
#include "canwinch_pod.h"
//#include "sensor_threshold.h"
#include "common_can.h"
#include "common_time.h"
#include "can_log.h"
#include "p1_gps_time_convert.h"
#include "p1_PC_handler.h"
#include "gps_poll.h"
#include "gps_1pps_se.h"

/* Each node on the CAN bus gets a unit number */
#define IAMUNITNUMBER	CAN_UNITID_OLI2	// Olimex (DEH board) CO

extern unsigned int ticks_flg;

/* Parameters for setting up clock. (See: "libmiscstm32/clockspecifysetup.h" */
const struct CLOCKS clocks = { \
HSOSELECT_HSE_XTAL,	/* Select high speed osc 			*/ \
PLLMUL_8X,		/* Multiplier PLL: 0 = not used 		*/ \
1,			/* Source for PLLMUL: 0 = HSI, 1 = PLLXTPRE (1 bit predivider)	*/ \
0,			/* PLLXTPRE source: 0 = HSE, 1 = HSI/2 (1 bit predivider on/off)	*/ \
APBX_2,			/* APB1 clock = SYSCLK divided by 1,2,4,8,16; freq <= 36 MHz */ \
APBX_1,			/* APB2 prescalar code = SYSCLK divided by 1,2,4,8,16; freq <= 72 MHz */ \
AHB_1,			/* AHB prescalar code: SYSCLK/[2,4,8,16,32,64,128,256,512] (drives APB1,2) */ \
8000000			/* Oscillator source frequency, e.g. 8000000 for an 8 MHz xtal on the external osc. */ \
};

/* Parameters for setting up CAN */

// Default: based on 72 MHz clock|36 MHz AHB freqs--500,000 bps, normal, port B
//const struct CAN_PARAMS can_params = CAN_PARAMS_DEFAULT;	// See 'canwinch_pod.h'

// Experimental CAN params: Based on 64 MHz clock|32 MHz AHB freqs
const struct CAN_PARAMS can_params = { \
500000,		// baudrate
2,		// port: port: 0 = PA 11|12; 2 = PB; 3 = PD 0|1;  (1 = not valid; >3 not valid) 
0,		// silm: CAN_BTR[31] Silent mode (0 or non-zero)
0,		// lbkm: CAN_BTR[30] Loopback mode (0 = normal, non-zero = loopback)
4,		// sjw:  CAN_BTR[24:25] Resynchronization jump width
4,		// tbs2: CAN_BTR[22:20] Time segment 2 (e.g. 5)
11,		// tbs1: CAN_BTR[19:16] Time segment 1 (e.g. 12)
1,		// dbf:  CAN_MCR[16] Debug Freeze; 0 = normal; non-zero =
0,		// ttcm: CAN_MCR[7] Time triggered communication mode
1,		// abom: CAN_MCR[6] Automatic bus-off management
0,		// awum: CAN_MCR[5] Auto WakeUp Mode
1		// nart: CAN_MCR[4] No Automatic ReTry (0 = retry; non-zero = transmit once)
};


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

/* **************************************************************************************
 * void system_reset(void);
 * @brief	: Software caused RESET
 * ************************************************************************************** */
static void loop(volatile int ct)
{
	while (ct > 0) ct -= 1; 
	return;
}
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
	SCB_AIRCR  = (0x5FA << 16) | SCB_AIRCR_SYSRESETREQ;	// Cause a RESET
	while (1==1);
}

/* **************************************************************************************
 * void prog_checksum_loader(void);
 * @brief	: Do program checksum/prog load until given "GO AHEAD" command
 * ************************************************************************************** */
void prog_checksum_loader(void)
{	
//s32 sw_startup  = 0;	// while loop lock
/*
	// Endless loop until checksum gets checked, (or power down and restart the whole mess)
	while (sw_startup == 0)
	{
		// [We might want a timeout on this while]
		while ((canbuffptr = can_unitid_getmsg()) == 0); // Wait for "my unitid" msg

		// Msg is for us, so ignore UNITID.  Look at DATAID, other bits, RTR, IDE
		switch ((canbufptr->id) & ~CAN_UNITID_MASK)
		{
		case CAN_CHECKSUMREQ: // Checksum request: contains start|end addresses
			can_send_checksum (canbufptr);	// Respond with what we computed
			break;

		case (CAN_LOADER_ERASE | CAN_RIxR_RTR): // Loader payload: contains erase block address
			can_loader_erase(canbufptr);	// Returns only when finished
			can_send_loader_response();	// Let sender know we are ready
			break;

		case CAN_LOADER_DATA: // Loader payload: contains 8 bytes of program data
			can_loader_data(canbufptr);	// Returns only when finished
			can_send_loader_response();	// Let sender know we are ready
			break;
		
		case (CAN_DATAID_GO | CAN_RIxR_RTR): // Checksumming complete: "Go ahead" command
			sw_startup = 1;	// Break 
			break;

		case CAN_DATAID_RESET: // Software forced RESET command
			system_reset();	// Cause RESET, never return.
			break;
		
	}
	return;
*/
}

/* **************************************************************************************
 * void putc ( void* p, char c); // This is for the tiny printf
 * ************************************************************************************** */
// Note: the compiler will give a warning about conflicting types
// for the built in function 'putc'.  Use ' -fno-builtin-putc' to eliminate compile warning.
void putc ( void* p, char c)
	{
		p=p;	// Get rid of the unused variable compiler warning
		USART2_txint_putc(c);
	}

/*#################################################################################################
  And now---the main routine!
  #################################################################################################*/
int main(void)
{
	u32 ticks_flg_prev = 0;
	s32 can_ret = -4;
	int i;

/* $$$$$$$$$$$$ Relocate the interrupt vectors from the loader to this program $$$$$$$$$$$$$$$$$$$$ */
extern void relocate_vector(void);
//	relocate_vector();

/* --------------------- Begin setting things up -------------------------------------------------- */ 

	clockspecifysetup((struct CLOCKS*)&clocks);		// Get the system clock and bus clocks running

/* ---------------------- Set up pins ------------------------------------------------------------- */
	gpio_setup();		// Need this to make the LED work

//[enable GPS power]

	init_printf(0,putc);	// This one-time initialization is needed by the tiny printf routine

//	SYSTICK_init(0);	// Set SYSTICK for interrupting and to max count (24 bit counter)

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
	USART2_rxinttxint_init(115200,32,3,96,4); // Initialize USART and setup control blocks and pointers

	/* Announce who we are */
	USART2_txint_puts("\n\rco1_OLIMEX 01-24-2014 v0\n\r");
	USART2_txint_send();	// Start the line buffer sending

	/* Display things for to entertain the hapless op */
	printf ("  hclk_freq (MHz) : %9u\n\r",  hclk_freq/1000000);	USART2_txint_send();
	printf (" pclk1_freq (MHz) : %9u\n\r", pclk1_freq/1000000);	USART2_txint_send();
	printf (" pclk2_freq (MHz) : %9u\n\r", pclk2_freq/1000000);	USART2_txint_send();
	printf ("sysclk_freq (MHz) : %9u\n\r",sysclk_freq/1000000);	USART2_txint_send();

/* --------------------- CAN setup ------------------------------------------------------------------- */
	/* Setup CAN registers and initialize routine */
	can_ret = can_init_pod((struct CAN_PARAMS*)&can_params); // 'struct' hold all the parameters


	/* Check if initialization was successful, or timed out. */
	if (can_ret <= 0)
	{
		printf("can init failed: code = %d\n\r",can_ret);USART2_txint_send(); 	while (1==1);
	}
	printf("\n\rcan ret ct: %d\n\r",can_ret);USART2_txint_send(); // Look at how long "exit initialization" took

	/* Set filters to respond "this" unit number and time sync broadcasts */
	can_filter_unitid(IAMUNITNUMBER);	// Setup msg filter banks

	/* Let the hapless Op see who this guy is */
	printf ("IAMUNITNUMBER %0x %0x\n\r",IAMUNITNUMBER,(u32)IAMUNITNUMBER >> CAN_UNITID_SHIFT);USART2_txint_send();

printf("sizeof(struct CANRCVTIMBUF) = %u\n\r",sizeof(struct CANRCVTIMBUF)); USART2_txint_send();
printf("sizeof(struct CANRCVBUF) = %u\n\r",sizeof(struct CANRCVBUF)); USART2_txint_send();

	/* Program checksum check and prog reload */
//	prog_checksum_loader();	// Return means we got the "Go ahead" command

/* ################ Basic CAN'ed machine is ready, so do program-specific startup ##################### */
/* --------------------- Setup for CO'ing  ------------------------------------------------------------ */

	/* Setup GPS ascii port for reading GPS sentences. */
	USART1_rxinttxint_init(4800,96,2,70,2);	//  1 Hz GPS 

	/* Set type of GPS */
	cGPStype = GARMIN_18X_1_HZ;		//  cGPStype == GARMIN_18X_1_HZ (3)

	/* Setup TIM1 to measure processor clock using GPS 1_PPS */
	Tim4_pod_init();	// Setup of the timers and pins for TIM1CH1* (* = remapped)

	/* Setup Timers for time stamping */

/* Test SD card inserted here. Loop if not sensed. */

	/* This is synchronous and will take time while it searches the last written packet location. */	
	if ((i = sdlog_init()) != 0)	// Initialize the SD Card (and spi2, etc.). (@7)
	{
		printf("SD INITIALIZATION FAILED.  It returned: %x (%d)\n\r",i,i);  USART2_txint_send();
		loop(10000000);	system_reset();	// Wait for serial port to finish, then reset
	}

/* This would be a place to put a test of the SD working, i.e. read/write. */


	/* Wait for good time stamps from GPS */
// Should we have "several" LEDs for Uli, e.g. SD OK, GPS OK?

	/* Set filter to accept all msgs.  Loggable ones are selected by software. */
	can_ret = can_filtermask16_add( 0 );	// Allow all msgs

	/* Check if initialization was successful, or timed out. */
	if (can_ret < 0)
	{
		printf("can_filtermask16_add failed: code = %d\n\r",can_ret);USART2_txint_send();
		system_reset();
	}
	USART2_txint_puts("All pass filter added\n\r\n\r");USART2_txint_send();

	/* YOU'D BETTER BE READY: At this point loggable CAN msgs will begin logging (under the low priority interrupt) */
	can_log_init();

/* -------------------- Endless bliss of logging ----------------------------------------------------- */

	/* Main polling loop.  Poll GPS and Logging (SD card write) */
	while (1==1)
	{
		/* Poll gps to setup time/date */
		gps_poll();

		/* Poll to log */
		if (sd_error == 0)	// 'can_log_can' sets 'sd_error'
		{
			can_log_trigger_logging();	// Trigger low priority interrupt to log all CAN msgs that are buffered
		}
		if (sd_error != 0)
		{ // Here we have an unrecoverable error with the SD card.
			printf ("HELP!  Something wrong with the SD card.  Error code: %d\n\r",sd_error);USART2_txint_send();
			loop(20000000);	system_reset();	// Wait for serial port to finish, then reset
		}

		/* Poll for keyboard/PC input and handling in the thereafter. */
		p1_PC_handler();

		/* LED 1 sec on/off to entertain the hapless op. */
		if (ticks_flg != ticks_flg_prev)	// One sec ticks
		{ // Here yes.
			ticks_flg_prev = ticks_flg;	// Update flag counter
			toggle_led ();
		}
			

	}
	return 0;	
}

