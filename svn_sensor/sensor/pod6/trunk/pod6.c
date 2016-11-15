/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : pod6.c
* Hackeroo           : deh
* Date First Issued  : 02/11/2013
* Board              : STM32F103VxT6_pod_mm
* Description        : Use pod board to prototype a sensor logger w shaft encoder
*******************************************************************************/
/* 
02/01/2013 Rev 115 Changes to add Tim4_pod_common.c for time syncing.

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
#include "libmiscstm32/printf.h"
#include "libmiscstm32/clockspecifysetup.h"

#include "adcsensor_pod.h"
#include "canwinch_pod_common_systick2048.h"
#include "sensor_threshold.h"
#include "../../../../svn_common/trunk/common_can.h"
#include "../../../../svn_common/trunk/db/can_db.h"
#include "scb.h"
#include "pod6.h"
#include "tick_pod6.h"
#include "canwinch_pod_common_systick2048_printerr.h"

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

/* Parameters for setting up CAN--POD board */

// CAN params: Based on 64 MHz clock|32 MHz AHB freqs
// (1 + tbs1 + tbs2) = 4, 8, 16 (to make even 1E6, 5E5,...baud)
// (1 + 13 + 2) gives sample pt: 14/16 = 87.5% pt (CANOpen spec)
// pclk1_freq / (1 + tbs1 + tbs2) = power of 2.

const struct CAN_PARAMS can_params = { \
IAMUNITNUMBER,	// CAN ID for this unit
1000000,		// baudrate
2,		// port: port: 0 = PA 11|12; 2 = PB; 3 = PD 0|1;  (1 = not valid; >3 not valid) 
0,		// silm: CAN_BTR[31] Silent mode (0 or non-zero)
0,		// lbkm: CAN_BTR[30] Loopback mode (0 = normal, non-zero = loopback)
1,		// sjw:  CAN_BTR[24:25] Resynchronization jump width
2,		// tbs2: CAN_BTR[22:20] Time segment 2 (e.g. 5)
13,		// tbs1: CAN_BTR[19:16] Time segment 1 (e.g. 12)
1,		// dbf:  CAN_MCR[16] Debug Freeze; 0 = normal; non-zero =
0,		// ttcm: CAN_MCR[7] Time triggered communication mode
1,		// abom: CAN_MCR[6] Automatic bus-off management
0,		// awum: CAN_MCR[5] Auto WakeUp Mode
0		// nart: CAN_MCR[4] No Automatic ReTry (0 = retry; non-zero = transmit once)
};

/* ------- LED identification ----------- 
|-number on pc board below LEDs
| color   ADCx codewheel  bit number
3 green   ADC2   black    0   LED3
4 red	  ADC2   white    1   LED4
5 green   ADC1   black    0   LED5
6 yellow  ADC1   white    1   LED6
  --------------------------------------*/
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
	return;
}
void toggle_1led(int led)
{
	if ((GPIO_ODR(GPIOE) & (1<<led)) == 0)
	{ // Here, LED bit was off
		GPIO_BSRR(GPIOE) = (1<<led);	// Set bits = all four LEDs off
	}
	else
	{ // HEre, LED bit was on
		GPIO_BRR(GPIOE) = (1<<led);	// Reset bits = all four LEDs on
	}
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
	SCB_AIRCR = (0x5FA << 16) | SCB_AIRCR_SYSRESETREQ;	// Cause a RESET
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

/*#################################################################################################
And now for the main routine 
  #################################################################################################*/
int main(void)
{
	int i = 0; 		// Timing loop variable
	int can_ret = -4;

/* $$$$$$$$$$$$ Relocate the interrupt vectors from the loader to this program $$$$$$$$$$$$$$$$$$$$ */
extern void relocate_vector(void);
//$	relocate_vector();
/* --------------------- Begin setting things up -------------------------------------------------- */ 

	clockspecifysetup((struct CLOCKS*)&clocks);		// Get the system clock and bus clocks running

/* ---------------------- Set up pins ------------------------------------------------------------- */
	PODgpiopins_default();	// Set gpio port register bits for low power
	PODgpiopins_Config();	// Now, configure pins
	MAX3232SW_on;		// Turn on RS-232 level converter (if doesn't work--no RS-232 chars seen)
	ANALOGREG_on;		// Turn on 3.2v analog regulator
	SDCARDREG_on;		// Turn on 3.3v regulator to SD card, GPS header, and CAN driver (trace cut)
	ENCODERGPSPWR_on;	// Turn on 5.0v encoder regulator hacked to CAN driver

	/* Use DTW_CYCCNT counter for startup timing */
	DTW_counter_init();

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
	USART1_txint_puts("\n\rco1: POD sensor hack 05-17-2015\n\r");
	USART1_txint_send();	// Start the line buffer sending

	/* Display things for to entertain the hapless op */
	printf ("  hclk_freq (MHz) : %9u\n\r",  hclk_freq/1000000);	USART1_txint_send();
	printf (" pclk1_freq (MHz) : %9u\n\r", pclk1_freq/1000000);	USART1_txint_send();
	printf (" pclk2_freq (MHz) : %9u\n\r", pclk2_freq/1000000);	USART1_txint_send();
	printf ("sysclk_freq (MHz) : %9u\n\r",sysclk_freq/1000000);	USART1_txint_send();

/* --------------------- ADC setup and initialization ----------------------------------------------- */
	adc_init_sequence_se();	// Time delay + calibration, then start conversion

/* --------------------- CAN setup ------------------------------------------------------------------- */
	/* Configure and set MCP 2551 driver: RS pin (PD 11) on POD board */
	can_nxp_setRS_sys(0,0); // (1st arg) 0 = high speed mode; 1 = standby mode (Sets yellow led on)
	GPIO_BSRR(GPIOE) = (0xf<<LED3);	// Set bits = all four LEDs off

	/* Setup CAN registers and initialize routine */
	//  Arguments: Pointer to CAN parameter struct,  Number of msgs to buffer
	can_ret = can_init_pod_varbuf_sys((struct CAN_PARAMS*)&can_params, 140);

	/* Check if initialization was successful, or timed out. */

	if (can_ret <= 0)
	{
		printf("can init failed: code = %d\n\r",can_ret);USART1_txint_send(); 
		while (1==1);
	}
	printf("\n\rcan ret ct: %d\n\r",can_ret);USART1_txint_send(); // Look at how long "exit initialization" took

	/* Set filters to respond "this" unit number and time sync broadcasts */
	can_filter_unitid_sys(IAMUNITNUMBER);	// Setup msg filter banks

	printf ("IAMUNITNUMBER %0x %0x\n\r",IAMUNITNUMBER,(unsigned int)IAMUNITNUMBER >> CAN_UNITID_SHIFT);USART1_txint_send();

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
int j;
int k = 0;
int m = 0;

extern u32 CAN_ticks;
extern s32 CAN_dev;
extern s32 CAN_ave;
extern u32 CAN_dbg1;
extern u32 CAN_dbg2;
extern s32 CAN_dif;
extern u32	stk_val;
extern u8	LEDflg;
extern int z1,z2;
extern u32 dbgStk;
extern int dbgTz;
extern int dbgTz_max;
extern int dbgTz_min;
extern int qw;


	/* Print the header for the CAN driver error counts */
	canwinch_pod_common_systick2048_printerr_header();
/* --------------------- Endless Stuff ----------------------------------------------- */
	while (1==1)
	{
		if (LEDflg != 0)
		{
			LEDflg = 0;
			toggle_1led(LEDGREEN1);
		}

		if (dbgStk != 0)
		{
//			DISABLE_TXINT;
//			z1 = pendlistct(); z2 = friilistct();
//			ENABLE_TXINT;

			printf("%d %d ",qw, qw * 64);			
			dbgStk = 0;

			printf("%4d %4d %4d ",dbgTz, dbgTz_max, dbgTz_min);
		
USART1_txint_send();

//			printf("%d %d %d ",z1,z2,z1+z2);
			canwinch_pod_common_systick2048_printerr(&can_errors);
		}


		/* Check on incoming data */
		pFifo1 = canrcvtim_get_sys();
		if (pFifo1 != 0)
		{ // Here, we got something!
			i += 1;
			can_msg_rcv_expand_sys(&pFifo1->R);	// Expand a compressed msg
//			printf("FIFO1: %08x %08x %08xL %08xH\n\r",pFifo1->R.id, pFifo1->R.dlc, pFifo1->R.cd.ui[0],pFifo1->R.cd.ui[1]);  USART1_txint_send();

			printf("%5d %7d %5d %6d\n\r",can_errors.can_msgovrflow,CAN_dbg1, CAN_dbg2,CAN_dif);  USART1_txint_send();
		}
		pFifo0 = canrcv_get_sys();
		if (pFifo0 != 0)
		{ // Here, we got something!
			can_msg_rcv_expand_sys(pFifo0);	// Expand a compressed msg
			printf("FIFO0: %08x %08x %08xL %08xH\n\r",pFifo0->id, pFifo0->dlc, pFifo0->cd.ui[0],pFifo0->cd.ui[1]); USART1_txint_send();
		}

	}
	return 0;	
}

