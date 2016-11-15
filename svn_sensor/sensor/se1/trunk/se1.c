/******************************************************************************
* File Name          : se1.c
* Date First Issued  : 10/15/2014
* Board              : RxT6
* Description        : Sensor board with pressure, rpm, throttle pot, thermistor
*******************************************************************************/
/* 
 ======================
NOTE: 'adcsensor_eng.c' in 'static void adc_cic_filtering(void)' 
is where the CAN messages get setup and sent.
 ======================

04/19/2014 CAN message rev 427
 number/sec canID description
*1) 64 0x30800000 Throttle & thermistor
 2) 64 0x40800000 RPM & Manifold Pressure
 3)  2 0x70800000 Temperature (deg C x100)
 4) 64 0x80800000 Throttle (range 0 - 4095)
* = commented out in 'adcsensor.eng.c' (not needed)

02/01/2013 Rev 115 Changes to add Tim4_pod_common.c for time syncing.
Initial = POD6 routine.

10/15/2014: change to have ldr.c CAN ID retrieved from ldr.c vector, and
            highflash area for app CAN IDs and other parameters.

Open minicom on the PC with 115200 baud and 8N1.

*/

/* &&&&&&&&&&&&& Each node on the CAN bus gets a unit number &&&&&&&&&&&&&&&&&&&&&&&&&& */
#define IAMUNITNUMBER	CAN_UNITID_SE1	// Sensor board #1 w pressure, throttle pot, thermistor, rpm
/* &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& */


/* Calibrations, parameters, etc., that apply to this application--FLASHP
   The placement in flash is controlledl by the .ld file.
   The following struct must match the order/index in the .txt file the PC uses.
   The PC stores the data as unsigned ints into an array  

*/
/* Example .txt file the PC uses 
//i   0       4     "%f"    -0.5	@ Manifold pressure offset
//i   1       4     "%f"   9.629E-3	@ Manifold pressure scale (inch hg)
//i   2       4     "%f"   0.01         @ Transmission temperature (deg C)
*/

struct FLASHP_SE1
{
	unsigned int crc;	// crc-32 placed by loader
	float offset_mp;	// Manifold pressure offset
	float scale_mp;		// Manifold pressure scale (inch hg)
	float scale_temp;	// Transmission temperature (deg C)
	unsigned int jic[3];	// Some spares
};

/* Make block that can be accessed via the "jump" vector (e.g. 0x08004004) that points
   to 'unique_can_block', which then starts Reset_Handler in the normal way.  The loader
   program knows the vector address so it can then get the address of 'unique_can_block'
   and get the crc, size, and data area via a fixed offset from the 'unique_can_block'
   address. 

   The .ld file assures that the 'unique_can_block' function is is followed by the data
   by using the sections.  */
#include "startup_deh.h"
__attribute__ ((section(".ctbltext")))
void unique_can_block(void)
{
	Reset_Handler();
	while(1==1);
}
/* The order of the following is important. */
extern const struct FLASHP_SE1 __highflashp;
__attribute__ ((section(".ctbldata")))
const unsigned int flashp_size = sizeof (struct FLASHP_SE1);
__attribute__ ((section(".ctbldata1")))
const struct FLASHP_SE1* flashp_se1 = (struct FLASHP_SE1*)&__highflashp;



#include <math.h>
#include <string.h>

//#include "PODpinconfig.h"
#include "libopenstm32/adc.h"
#include "libopenstm32/can.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/gpio.h"
#include "scb.h"
#include "libopenstm32/usart.h"
#include "libusartstm32/usartallproto.h"
//#include "libmiscstm32/systick1.h"
#include "libmiscstm32/printf.h"
#include "libmiscstm32/clockspecifysetup.h"

#include "adcsensor_eng.h"
#include "canwinch_pod_common_systick2048.h"
#include "../../../../svn_common/trunk/common_can.h"
#include "SENSORpinconfig.h"
#include "sensor_threshold.h"
#include "rw_eeprom.h"
#include "se1.h"
#include "tick_pod6.h"
#include "panic_leds.h"
#include "rpmsensor.h"
#include "temp_calc.h"
#include "CANascii.h"
#include "canwinch_pod_common_systick2048_printerr.h"

/* Error counts for monitoring. */
extern struct CANWINCHPODCOMMONERRORS can_errors;	// A group of error counts

/* For test with and without XTAL clocking */
//#define NOXTAL 
#ifdef NOXTAL

/* Parameters for setting up clock. (See: "libmiscstm32/clockspecifysetup.h" */

/* NOTE: APB2 is set 32 MHz and the ADC set for max divide (divide by 8).  The slower ADC helps the 
   accuracy in the presence of source impedance. */

// INTERNAL RC osc parameters -- 64 MHz
const struct CLOCKS clocks = { \
HSOSELECT_HSI,	/* Select high speed osc 			*/ \
PLLMUL_16X,		/* Multiplier PLL: 0 = not used 		*/ \
0,			/* Source for PLLMUL: 0 = HSI, 1 = PLLXTPRE (1 bit predivider)	*/ \
0,			/* PLLXTPRE source: 0 = HSE, 1 = HSE/2 (1 bit predivider on/off)	*/ \
APBX_2,			/* APB1 clock = SYSCLK divided by 1,2,4,8,16; freq <= 36 MHz */ \
APBX_4,			/* APB2 prescalar code = SYSCLK divided by 1,2,4,8,16; freq <= 72 MHz */ \
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
APBX_4,			/* APB2 prescalar code = SYSCLK divided by 1,2,4,8,16; freq <= 72 MHz */ \
AHB_1,			/* AHB prescalar code: SYSCLK/[2,4,8,16,32,64,128,256,512] (drives APB1,2) */ \
8000000			/* Oscillator source frequency, e.g. 8000000 for an 8 MHz xtal on the external osc. */ \
};

#endif

/* Parameters for setting up CAN */

// Default: based on 72 MHz clock|36 MHz AHB freqs--500,000 bps, normal, port B
//const struct CAN_PARAMS can_params = CAN_PARAMS_DEFAULT;	// See 'canwinch_pod.h'

// Experimental CAN params: Based on 64 MHz clock|32 MHz AHB freqs
struct CAN_PARAMS can_params = { \
0xfffffffc,	// CAN ID for this unit
500000,		// baudrate
0,		// port: port: 0 = PA 11|12; 2 = PB; 3 = PD 0|1;  (1 = not valid; >3 not valid) 
0,		// silm: CAN_BTR[31] Silent mode (0 or non-zero)
0,		// lbkm: CAN_BTR[30] Loopback mode (0 = normal, non-zero = loopback)
2,		// sjw:  CAN_BTR[24:25] Resynchronization jump width
3,		// tbs2: CAN_BTR[22:20] Time segment 2 (e.g. 4)
4,		// tbs1: CAN_BTR[19:16] Time segment 1 (e.g. 9)
1,		// dbf:  CAN_MCR[16] Debug Freeze; 0 = normal; non-zero = freeze during debug
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
	SCB_AIRCR = (0x5FA << 16) | SCB_AIRCR_SYSRESETREQ;	// Cause a RESET
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
		CANascii_putc(c);
		USART1_txint_putc(c);
	}

/*#################################################################################################
And now for the main routine 
  #################################################################################################*/
int main(void)
{
	int i = 0; 		// Timing loop variable
	int init_ret = -4;

/* $$$$$$$$$$$$ Relocate the interrupt vectors from the loader to this program $$$$$$$$$$$$$$$$$$$$ */
extern void relocate_vector(void);
	relocate_vector();

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
	i = USART1_rxinttxint_init(115200,32,2,96,4); // Initialize USART and setup control blocks and pointers
//	USART1_rxinttxint_init(  9600,32,2,32,3); // Initialize USART and setup control blocks and pointers

	if (i != 0) panic_leds(7);	// Init failed: Bomb-out flashing LEDs 7 times

	/* Announce who we are */
	USART1_txint_puts("\n\rSE1: 05-12-2015 v1\n\r");
	USART1_txint_send();	// Start the line buffer sending

	CANascii_init();	// Setup for sending printf 'putc' as CAN msgs

	/* Display things for to entertain the hapless op */
#ifdef NOXTAL
 	printf ("NO  XTAL\n\r");
#else
 	printf ("YES XTAL\n\r");
#endif

	printf ("  hclk_freq (MHz) : %9u\n\r",  hclk_freq/1000000);	
	printf (" pclk1_freq (MHz) : %9u\n\r", pclk1_freq/1000000);	
	printf (" pclk2_freq (MHz) : %9u\n\r", pclk2_freq/1000000);	
	printf ("sysclk_freq (MHz) : %9u\n\r",sysclk_freq/1000000);	USART1_txint_send();

/* --------------------- Get Loader CAN ID ---------------------------------------------------------- */
	/* Pick up the unique CAN ID stored when the loader was flashed. */
	u32 canid_ldr = *(u32*)((u32)((u8*)*(u32*)0x08000004 + 7 + 0));	// First table entry = can id
	printf(  "CANID_LDR : %08X\n\r", canid_ldr );	USART1_txint_send();	

//$$$$ change
	can_params.iamunitnumber = 0x0800000;	// Use one "base" CAN ID

/* --------------------- eeprom --------------------------------------------------------------------- */
//	if ((j=rw_eeprom_init()) != 0)
//	{
//		printf("eeprom init failed: %i\n\r",j); USART1_txint_send();
//	}

/* --------------------- RPM routines (TIM4_CH4 and others) ------------------------------------------ */
	rpmsensor_init();

/* --------------------- ADC setup and initialization ------------------------------------------------ */
	adc_init_se_eng_sequence(can_params.iamunitnumber);	// Time delay + calibration, then start conversion

/* --------------------- CAN setup ------------------------------------------------------------------- */
	/* Configure CAN criver RS pin: Sensor RxT6 board = (PB 7) */
	can_nxp_setRS_sys(0,1); // (1st arg) 0 = high speed mode; not-zero = standby mode

	/* Setup CAN registers and initialize routine */
	//  Arguments: Pointer to CAN parameter struct,  Number of msgs to buffer
	init_ret = can_init_pod_varbuf_sys(&can_params, 129);

	/* Check if initialization was successful, or timed out. */
	if (init_ret <= 0)
	{
		panic_leds(6);	while (1==1);	// Six flash panic display with code 6
	}
	printf("\n\rcan ret ct: %d\n\r",init_ret);USART1_txint_send(); // Look at how long "exit initialization" took

	/* Set filters to respond "this" unit number and time sync broadcasts */
	can_filter_unitid_sys(canid_ldr);	// Setup msg filter banks

	printf ("IAMUNITNUMBER %08x %0x\n\r",can_params.iamunitnumber,can_params.iamunitnumber >> CAN_UNITID_SHIFT);USART1_txint_send();



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
//struct CANRCVTIMBUF * pFifo1 = 0;	// FIFO 1 buffer receive msg pointer
//struct CANRCVBUF * pFifo0 = 0;		// FIFO 2 buffer receive msg pointer

//int can_put = -1;
//int k = 0;
//int m = 0;

//extern u32 CAN_ticks;
//u32 CAN_ticks_prev = 0;
//extern s32 CAN_dev;
//extern s32 CAN_ave;
//extern s32 CAN_dbg1;
//extern s32 CAN_dbg2;
//extern s32 CAN_dbg3;
//extern s32 CAN_dif;
//extern u32	stk_val;
//extern u32	can_msgovrflow;	
//u32 		stk_64flgctr_prev1;

u32	uiT;
u32	uiT_prev = 0;
u32 	tctr = 0;
//int tx;
//int tx_prev = 0;
int sum = 0;
//int sctr = 0;
struct TIMCAPTRET32 strT = {0,0};

/* ADC testing */
#define ADCCOUNT 168000000/8;
u32	t_adc = *(volatile unsigned int *)0xE0001004 + ADCCOUNT; // Set initial time
extern unsigned int cicdebug0;
unsigned int cicdebug0_prev = 0;

/* Green LED flashing */
static u32 stk_64flgctr_prev;
static u32 throttleLED = 0;
static struct CANRCVBUF *pcan;

	/* Print the header for the CAN driver error counts */
	canwinch_pod_common_systick2048_printerr_header();

/* --------------------- Endless Stuff ----------------------------------------------- */
	while (1==1)
	{
		/* Green LED OK flasher */
		if (stk_64flgctr != stk_64flgctr_prev)
		{
			stk_64flgctr_prev = stk_64flgctr;
			throttleLED += 1;
			if (throttleLED >= 64)
			{
				throttleLED = 0;
				TOGGLE_GREEN;	// Slow flash of green means "OK"

				/* Print the counters in 'canwinch_pod_common_systick2048' */
				canwinch_pod_common_systick2048_printerr(&can_errors);
			}
//			printf("%7d %4d %2d %7d %7d %7d\n\r",CAN_ave,CAN_dbg1, CAN_dbg2,CAN_dbg3,CAN_dif, CAN_dev); USART1_txint_send();
		}

		strT = Tim4_inputcapture_ui();
		uiT = strT.ic;
		if (uiT != uiT_prev)
		{
			sum += (uiT - uiT_prev)-1066667-60;
//			printf("%8d ",(uiT - uiT_prev)-1066667-60);
			if (tctr++ >= 16)
			{
				tctr = 0;
//				printf("%5d %6d\n\r",sctr++,sum/16);
				sum = 0;
//				printf("\n\r");USART1_txint_send();
			}
			uiT_prev = uiT;
		}

		/* Periodically list the ADC readings */
		if (((int)(*(volatile unsigned int *)0xE0001004 - t_adc)) > 0) // Has the time expired?
		{ // Here, yes.
//			printf("%5d %5d: ",ct0++,(cicdebug0 - cicdebug0_prev)/3); // Sequence number, number of filtered readings between xprintf's
			cicdebug0_prev = cicdebug0;

//			for (i = 0; i < NUMBERADCCHANNELS_SE; i++)	// Loop through the three ADC channels
//				printf("%5d ", adc_last_filtered[i]);	// ADC filtered and scaled reading

//			printf("\n\r"); USART1_txint_send();
			t_adc += ADCCOUNT; 	// Set next time to display readings
		}


		/* Poll & compute calibrated temperature */
		temp_calc(); // Floating pt computation done at mainline priority

		/* Poll to check incoming CAN msgs. */
		pcan = canrcv_get_sys();
		if (pcan != 0) 	// Any CAN msgs ready?
		{ // Here.  Yes.
			CANascii_poll(pcan);	// Check for command code from PC to enable/disable CAN ascii
		}

	}

	return 0;	
}

