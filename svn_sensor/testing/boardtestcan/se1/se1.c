/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : se1.c
* Hackeroo           : deh
* Date First Issued  : 06/06/2013
* Board              : sensor board RxT6 w STM32F103RGT6
* Description        : sensor logger w shaft encoder
*******************************************************************************/
/* 
02/01/2013 Rev 115 Changes to add Tim4_pod_common.c for time syncing.
Initial = POD6 routine.

Open minicom on the PC with 115200 baud and 8N1.

*/
#include <math.h>
#include <string.h>

#include "PODpinconfig.h"
#include "libopenstm32/adc.h"
#include "libopenstm32/can.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/gpio.h"
#include "libopenstm32/usart.h"
#include "libusartstm32/usartallproto.h"
//#include "libmiscstm32/systick1.h"
#include "libmiscstm32/printf.h"
#include "libmiscstm32/clockspecifysetup.h"

#include "adcsensor_pod.h"
#include "canwinch_pod_common_systick.h"
#include "common_can.h"
#include "SENSORpinconfig.h"
#include "sensor_threshold.h"
#include "rw_eeprom.h"
#include "scb.h"
#include "se1.h"
#include "tick_pod6.h"
#include "panic_leds.h"


//#include "Tim4_pod_common.h"

//#include "dma17_fill.h"

//#include <malloc.h>

/* For test with and without XTAL clocking */
//#define NOXTAL 
#ifdef NOXTAL

/* Parameters for setting up clock. (See: "libmiscstm32/clockspecifysetup.h" */
// INTERNAL RC osc parameters -- 64 MHz
const struct CLOCKS clocks = { \
HSOSELECT_HSI,	/* Select high speed osc 			*/ \
PLLMUL_16X,		/* Multiplier PLL: 0 = not used 		*/ \
0,			/* Source for PLLMUL: 0 = HSI, 1 = PLLXTPRE (1 bit predivider)	*/ \
0,			/* PLLXTPRE source: 0 = HSE, 1 = HSE/2 (1 bit predivider on/off)	*/ \
APBX_2,			/* APB1 clock = SYSCLK divided by 1,2,4,8,16; freq <= 36 MHz */ \
APBX_1,			/* APB2 prescalar code = SYSCLK divided by 1,2,4,8,16; freq <= 72 MHz */ \
AHB_1,			/* AHB prescalar code: SYSCLK/[2,4,8,16,32,64,128,256,512] (drives APB1,2) */ \
8000000			/* Oscillator source frequency, e.g. 8000000 for an 8 MHz xtal on the external osc. */ \
};

#else

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

#endif

/* Parameters for setting up CAN */

// Default: based on 72 MHz clock|36 MHz AHB freqs--500,000 bps, normal, port B
//const struct CAN_PARAMS can_params = CAN_PARAMS_DEFAULT;	// See 'canwinch_pod.h'

// Experimental CAN params: Based on 64 MHz clock|32 MHz AHB freqs
const struct CAN_PARAMS can_params = { \
500000,		// baudrate
0,		// port: port: 0 = PA 11|12; 2 = PB; 3 = PD 0|1;  (1 = not valid; >3 not valid) 
0,		// silm: CAN_BTR[31] Silent mode (0 or non-zero)
0,		// lbkm: CAN_BTR[30] Loopback mode (0 = normal, non-zero = loopback)
4,		// sjw:  CAN_BTR[24:25] Resynchronization jump width
4,		// tbs2: CAN_BTR[22:20] Time segment 2 (e.g. 5)
11,		// tbs1: CAN_BTR[19:16] Time segment 1 (e.g. 12)
1,		// dbf:  CAN_MCR[16] Debug Freeze; 0 = normal; non-zero =
0,		// ttcm: CAN_MCR[7] Time triggered communication mode
1,		// abom: CAN_MCR[6] Automatic bus-off management
0,		// awum: CAN_MCR[5] Auto WakeUp Mode
0		// nart: CAN_MCR[4] No Automatic ReTry (0 = retry; non-zero = transmit once)
};


/* ************************************************************
Step through the LEDs
***************************************************************/
static int ledct;

void walk_LEDs(void)
{
	switch (ledct)
	{
	case 0: LED19RED_off;		LED20RED_off;		LED21GREEN_on;		break;
	case 1: LED19RED_off;		LED20RED_on;		LED21GREEN_off;		break;
	case 2: LED19RED_on;		LED20RED_off;		LED21GREEN_off;		break;
	default: ledct = 0; break;
	}
	ledct += 1;		// Step through all four LEDs
	if (ledct > 2) ledct = 0;
	return;
}
/* **************************************************************************************
 * void system_reset(void);
 * @brief	: Software caused RESET
 * ************************************************************************************** */
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
	SCB_AIRCR  |= (0x5FA << 16) | SCB_AIRCR_SYSRESETREQ;	// Cause a RESET
	while (1==1);
}

/* **************************************************************************************
 * void prog_checksum_loader(void);
 * @brief	: Do program checksum/prog load until given "GO AHEAD" command
 * ************************************************************************************** */
void prog_checksum_loader(void)
{	
//int sw_startup  = 0;	// while loop lock
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
		USART1_txint_putc(c);
	}

/*#################################################################################################
And now for the main routine 
  #################################################################################################*/
int main(void)
{
	int j;
	int i = 0; 		// Timing loop variable
	int can_ret = -4;

/* --------------------- Begin setting things up -------------------------------------------------- */ 

	clockspecifysetup((struct CLOCKS*)&clocks);		// Get the system clock and bus clocks running

/* ---------------------- Set up pins ------------------------------------------------------------- */
	SENSORgpiopins_Config();	// Now, configure pins

	/* Use DTW_CYCCNT counter for startup timing */
/* CYCCNT counter is in the Cortex-M-series core.  See the following for details 
http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0337g/BABJFFGJ.html */
	*(volatile unsigned int*)0xE000EDFC |= 0x01000000; // SCB_DEMCR = 0x01000000;
	*(volatile unsigned int*)0xE0001000 |= 0x1;	// Enable DTW_CYCCNT (Data Watch cycle counter)

//[enable GPS power]

	init_printf(0,putc);	// This one-time initialization is needed by the tiny printf routine

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
	USART1_rxinttxint_init(115200,32,2,96,4); // Initialize USART and setup control blocks and pointers
//	USART1_rxinttxint_init(  9600,32,2,32,3); // Initialize USART and setup control blocks and pointers

	/* Announce who we are */
	USART1_txint_puts("\n\rse1: 06-07-2013 \n\r");
	USART1_txint_send();	// Start the line buffer sending

	/* Display things for to entertain the hapless op */
	printf ("  hclk_freq (MHz) : %9u\n\r",  hclk_freq/1000000);	USART1_txint_send();
	printf (" pclk1_freq (MHz) : %9u\n\r", pclk1_freq/1000000);	USART1_txint_send();
	printf (" pclk2_freq (MHz) : %9u\n\r", pclk2_freq/1000000);	USART1_txint_send();
	printf ("sysclk_freq (MHz) : %9u\n\r",sysclk_freq/1000000);	USART1_txint_send();

/* --------------------- eeprom --------------------------------------------------------------------- */
	if ((j=rw_eeprom_init()) != 0)
	{
		printf("eeprom init failed: %i\n\r",j); USART1_txint_send();
	}

/* --------------------- ADC setup and initialization ----------------------------------------------- */
	adc_init_sequence_se();	// Time delay + calibration, then start conversion



/* --------------------- CAN setup ------------------------------------------------------------------- */
	/* Configure CAN criver RS pin: Sensor RxT6 board = (PB 7) */
	can_nxp_setRS_sys(0,1); // (1st arg) 0 = high speed mode; not-zero = stanby mode

	/* Setup CAN registers and initialize routine */
	can_ret = can_init_pod_sys((struct CAN_PARAMS*)&can_params); // 'struct' hold all the parameters

	/* Check if initialization was successful, or timed out. */
	if (can_ret <= 0)
	{
		printf("can init failed: code = %d\n\r",can_ret);USART1_txint_send(); 
		panic_leds(6);	while (1==1);	// Six flash panic display
	}
	printf("\n\rcan ret ct: %d\n\r",can_ret);USART1_txint_send(); // Look at how long "exit initialization" took

	/* Set filters to respond "this" unit number and time sync broadcasts */
	can_filter_unitid_sys(IAMUNITNUMBER);	// Setup msg filter banks

	printf ("IAMUNITNUMBER %0x %0x\n\r",IAMUNITNUMBER,(unsigned int)CAN_UNITID_SE1 >> CAN_UNITID_SHIFT);USART1_txint_send();

	/* Enable sending of CAN msg every 1/64th sec, in sync with CO time sync */
	tick_pod6_init();	

/* --------------------- Program is ready, so do program-specific startup ---------------------------- */


i = 0;

/* CAN tx test */
//struct CANRCVBUF can_msg;
//can_msg.id = IAMUNITNUMBER | 0x05000000;
//can_msg.id = CAN_UNITID_POD6;	// PD board hack sensor ID
//can_msg.cd.ui[0] = 0x76543210;	// 1st 4 bytes of msg
//can_msg.cd.ui[1] = 0xfedcba98;		// 2nd 4 bytes: if zero, then "n" in msg is only 4 bytes
// can_msg.cd.ull = 0xfedcba9876543210;
//   can_msg.cd.ull = 0x00000000abcd0000;

/* CAN rx test */
struct CANRCVTIMBUF * pFifo1 = 0;	// FIFO 1 buffer receive msg pointer
struct CANRCVBUF * pFifo0 = 0;		// FIFO 2 buffer receive msg pointer

int can_put = -1;
int k = 0;
int m = 0;

extern u32 CAN_ticks;
extern s32 CAN_dev;
extern s32 CAN_ave;
extern u32 CAN_dbg1;
extern u32 CAN_dbg2;
extern s32 CAN_dif;
extern u32	stk_val;
extern u32	can_msgovrflow;	
extern u8	LEDflg;

/* --------------------- Endless Stuff ----------------------------------------------- */
	while (1==1)
	{
		if (LEDflg != 0)
		{
			LEDflg = 0;
			TOGGLE_GREEN;	// Slow flash of green means "OK"
		}


		/* Check on incoming data */
		pFifo1 = canrcvtim_get_sys();
		if (pFifo1 != 0)
		{ // Here, we got something!
			i += 1;
			can_msg_rcv_expand_sys(&pFifo1->R);	// Expand a compressed msg
//			printf("FIFO1: %08x %08x %08xL %08xH\n\r",pFifo1->R.id, pFifo1->R.dlc, pFifo1->R.cd.ui[0],pFifo1->R.cd.ui[1]);  USART1_txint_send();

			printf("%5d %7d %5d %6d\n\r",can_msgovrflow,CAN_dbg1, CAN_dbg2,CAN_dif);  USART1_txint_send();
		}
		pFifo0 = canrcv_get_sys();
		if (pFifo0 != 0)
		{ // Here, we got something!
			can_msg_rcv_expand_sys(pFifo0);	// Expand a compressed msg
			printf("FIFO0: %08x %08x %08xL %08xH\n\r",pFifo0->id, pFifo0->dlc, pFifo0->cd.ui[0],pFifo0->cd.ui[1]); USART1_txint_send();
		}
		/* printf rate throttling */
//		if ( i >= 64)
//		while ( ((int)(SYSTICK_getcount32() - temp)) > 0  );
//		{
//			i = 0;
//			j = 0;
//			while (j++ < 1)	// Test a "burst" of msgs added to buffer
//			 {
//				can_msg_setsize_sys(&can_msg, 8);	// Fixed xmt byte count setup
//				can_msg_rcv_compress(&can_msg);	// Set byte count according to MSB
//				can_put = can_msg_put_sys(&can_msg);// Setup CAN msg in output buffer/queue
//				can_msg.cd.ui[0] += 1;		// Make the msg change for the next round
//      	         }

//			toggle_4leds();

extern unsigned int can_debugP;
extern unsigned int can_debugM;
extern unsigned int can_terr;



unsigned int can_esr = CAN_ESR(CAN1);
unsigned int can_tec = (can_esr >> 16) & 0xff;	// Tx error counter
unsigned int can_lec = (can_esr >>  4) &  0x3;	// Tx last error code

/* p 650 CAN_MSR
Bit 11 RX: CAN Rx signal
        Monitors the actual value of the CAN_RX Pin.
*/
unsigned int can_msr = CAN_MSR(CAN1);
unsigned int can_rcvbit = (can_msr >> 11) & 0x1;
//			printf("%5u %6d%6u%6u%6u%5u%4u%3u\n\r",k++,can_put, can_debugP,can_debugM,can_terr,can_tec, can_lec, can_rcvbit ) ;  USART1_txint_send();

//		}
	}
	return 0;	
}

