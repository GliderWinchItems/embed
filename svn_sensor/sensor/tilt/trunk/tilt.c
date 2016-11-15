/******************************************************************************
* File Name          : tilt.c
* Date First Issued  : 02/13/2015
* Board              : POD board
* Description        : 3-axis accelerometer CAN tiltometer 
*******************************************************************************/
/* 
02-05-2014 Hack of se4.c routine
02-13-2015 Hack of se4_h.c w reference to adcpod.c

*/

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

#include "tmpstruct.h"	// structs generated from .txt file for parameters, etc.
#include "adcsensor_tilt.h"
#include "canwinch_pod_common_systick2048.h"
#include "common_can.h"
#include "SENSORpinconfig.h"
#include "panic_leds.h"
#include "common_highflash.h"
#include "CANascii.h"

int dummy;

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
extern const struct FLASHP_SE4 __highflashp;
__attribute__ ((section(".ctbldata")))
const unsigned int flashp_size = sizeof (struct FLASHP_SE4);
__attribute__ ((section(".ctbldata1")))
const struct FLASHP_SE4* flashp_se3 = (struct FLASHP_SE4*)&__highflashp;



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
0xfffffffc,	// CAN ID for this unit is loaded here
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
//	int j;
	int init_ret = -4;

/* $$$$$$$$$$$$ Relocate the interrupt vectors from the loader to this program $$$$$$$$$$$$$$$$$$$$ */
extern void relocate_vector(void);
	relocate_vector();

/* --------------------- Begin setting things up -------------------------------------------------- */ 
	clockspecifysetup((struct CLOCKS*)&clocks);		// Get the system clock and bus clocks running

/* ---------------------- Set up pins ------------------------------------------------------------- */
	SENSORgpiopins_Config();	// Now, configure pins

	/* Use DTW_CYCCNT counter for timing */
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
	USART1_txint_puts("\n\r#### TILT #### 02-13-2015 \n\r");
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
	printf ("sysclk_freq (MHz) : %9u\n\r",sysclk_freq/1000000);	

	/* Some code to test the highflash. */
	printf ("\n\rTEST highflash\n\r");
	extern void* __highflashlayout;
	struct HIGHFLASHH* phfl = (struct HIGHFLASHH*)&__highflashlayout;
	printf("sizeof struct HIGHFLASHH: %08X %08X %08X\n\r",sizeof(struct HIGHFLASHH), (u32)phfl, sizeof(struct HIGHFLASHH) + (u32)phfl) ;
	printf("highflash:  crc: %08X        ver: %08X\n\r\tapp:crc: %08X    app:ver: %08X app:canidct: %d\n\r     crcprg:crc: %08X crcprg:ver: %08x crcprg:ct: %d\n\r",\
	phfl->crc,phfl->version,\
	phfl->appcanid.crc, phfl->appcanid.version, phfl->appcanid.numcanids,\
	 phfl->crcprog.crc,   phfl->crcprog.version, phfl->crcprog.numprogsegs);
	
	printf("\n\rFLASHH.APPCANID.canid[]: 0x%08X\n\r", (u32)&phfl->appcanid.canid[0]);
	unsigned int i;
	for (i = 0; i < phfl->appcanid.numcanids; i++) //List CAN ID table
		printf("%2d  0x%08X\n\r", i, phfl->appcanid.canid[i]);
	
	i = sizeof(struct FLASHP_SE4);
	printf ("\nsizeof(struct FLASHP_SE4): %d(bytes) %d(uints)\n",i,i/4);

	/*  Some code to test the passing of parameters from the .txt file through the loader. */
	printf ("\n\rTEST the format & setup of parameters from the .txt file\n\r");
	struct FLASHP_SE4* phfp = (struct FLASHP_SE4*)&__highflashp;
	printf ("flashp address: 0x%08X\n\r", (u32)phfp); 
	printf ("numblkseg  : %d\n\r", phfp->numblkseg);
	int ww = phfp->ctr_to_feet;	// NOTE: this printf doesn't handle floats or doubles
	int ff = phfp->ctr_to_feet * 1000000 - ww*1000000;
	printf ("ctr-to-feet: %d.%06u\n\r", ww/1000000,ff);
	printf ("Hi thres A3: %d\n\r", phfp->adc3_htr_initial);
	printf ("Lo thres A3: %d\n\r", phfp->adc3_ltr_initial);
	printf ("Hi thres A2: %d\n\r", phfp->adc2_htr_initial);
	printf ("Lo thres A2: %d\n\r", phfp->adc2_ltr_initial);
	printf ("ASCII      : %s\n\r", (char*)&phfp->c[0]);
	ww = phfp->dtest;
	ff = phfp->dtest * 1000000 - ww*1000000;
	printf ("dtest      : %d.%06u\n\r", ww/1000000,ff);
	union ZZ { unsigned long long ull; unsigned int ui[2];} zz;
	zz.ull = phfp->ulltest;	// NOTE: this printf doesn't handle long longs
	char rr[21]; int m = 19; unsigned long long yy = phfp->ulltest; unsigned long long xx;
	while (m >= 0) 
	{xx = yy % 10;rr[m] = '0' + xx; yy = yy / 10; m -= 1;} 
	rr[20] = 0;
	printf ("ulltest (1): %s\n\r",&rr[0]);
	printf ("ulltest (2): %08x %08x\n\r",zz.ui[1], zz.ui[0]);
	printf ("testidx    : %d\n\r", phfp->testidx);
	printf ("canid_error2           ; 0x%08X\n\r",phfp->canid_error2	  );	// [0]encode_state er ct [1]adctick_diff<2000 ct
	printf ("canid_error1           ; 0x%08X\n\r",phfp->canid_error1 	  );	// [2]elapsed_ticks_no_adcticks<2000 ct  [3]cic not in sync
	printf ("canid_adc3_histogramA  ; 0x%08X\n\r",phfp->canid_adc3_histogramA );	// ADC3 Histogram tx: request count, switch buffers. rx: send count
	printf ("canid_adc3_histogramB  ; 0x%08X\n\r",phfp->canid_adc3_histogramB );	// ADC3 Histogram tx: bin number, rx: send bin count
	printf ("canid_adc2_histogramA  ; 0x%08X\n\r",phfp->canid_adc2_histogramA );	// ADC2 Histogram tx: request count, switch buffers; rx send count
	printf ("canid_adc2_histogramB  ; 0x%08X\n\r",phfp->canid_adc2_histogramB );	// ADC2 Histogram tx: bin number, rx: send bin count
	printf ("canid_adc3_adc2_readout; 0x%08X\n\r",phfp->canid_adc3_adc2_readout);	// ADC3 ADC2 readings readout
	printf ("\n\r");
	USART1_txint_send();

/
/* --------------------- Get Loader CAN ID -------------------------------------------------------------- */
	/* Pick up the unique CAN ID stored when the loader was flashed. */
	u32 canid_ldr = *(u32*)((u32)((u8*)*(u32*)0x08000004 + 7 + 0));	// First table entry = can id
	printf(  "CANID_LDR  : 0x%08X\n\r", canid_ldr );	USART1_txint_send();
/* --------------------- Get Application Can ID --------------------------------------------------------- */
	can_params.iamunitnumber = phfl->appcanid.canid[0]; // First "slot" is used by this shaft sensor prog
	printf("CANID_APP  : 0x%08X\n\r",can_params.iamunitnumber);USART1_txint_send();
/* --------------------- ADC setup and initialization ------------------------------------------------ */
	adc_init_sequence_se_tilt(phfp);		// Time delay + calibration, then start conversion
/* --------------------- CAN setup ------------------------------------------------------------------- */
	/* Configure CAN criver RS pin: Sensor RxT6 board = (PB 7) */
	can_nxp_setRS_sys(0,1); // (1st arg) 0 = high speed mode; not-zero = standby mode

	/* Setup CAN registers and initialize routine */
	init_ret = can_init_pod_sys((struct CAN_PARAMS*)&can_params); // 'struct' hold all the parameters

	/* Check if initialization was successful, or timed out. */
	if (init_ret <= 0)
	{
		printf("can init failed: code = %d\n\r",init_ret);USART1_txint_send(); 
		panic_leds(6);	while (1==1);	// Six flash panic display with code 6
	}
	printf("\n\rcan ret ct: %d\n\r",init_ret);USART1_txint_send(); // Look at how long "exit initialization" took

	/* Set filters to respond "this" unit number and time sync broadcasts */
	can_filter_unitid_sys(canid_ldr);	// Setup msg filter banks

	/* Set filter to accept all msgs.  Loggable ones are selected by software. */
	init_ret = can_filtermask16_add_sys( 0 );	// Allow all msgs
	if (init_ret < 0)
	{
		printf("CAN additional filters failed: code = %d\n\r",init_ret);USART1_txint_send(); 
		panic_leds(6);	while (1==1);	// Six flash panic display with code 6
	}

	printf ("IAMUNITNUMBER %08x %0x\n\r",can_params.iamunitnumber,can_params.iamunitnumber >> CAN_UNITID_SHIFT);USART1_txint_send();
/* --------------------- Program is ready, so do program-specific startup ---------------------------- */

extern s32 CAN_dev;
extern s32 CAN_ave;
extern s32 CAN_dif;

extern u32 adc2histo[2][ADCHISTOSIZE];
extern u32 adc3histo[2][ADCHISTOSIZE];
extern int adc2histoctr[2];
extern int adc3histoctr[2];
extern union ADC12VAL adc12valbuff[2][ADCVALBUFFSIZE];
extern unsigned short adc3valbuff[2][ADCVALBUFFSIZE];
//int k = 0;

struct CANRCVBUF* pcan;
u32 z = 0;
u32 q = 0;

/* Green LED flashing */
static u32 stk_64flgctr_prev;

extern void testfoto(void);
//testfoto();

int xct = 0;
/* --------------------- Endless Stuff ----------------------------------------------- */
	while (1==1)
	{
		/* Check for a 1/2048th sec tick ('canwinch_pod_common_systick2048.c') */
		if (stk_64flgctr != stk_64flgctr_prev)
		{
			stk_64flgctr_prev = stk_64flgctr;

			/* Flash green LED in sync with time--ON/OFF each 1 sec */
			if ((stk_64flgctr & 0x7ff) == 0) {TOGGLE_GREEN;}	// Slow flash of green means "OK"

			/* Throttle CANascii to stay within bandwidth of CAN bus & PC<->gateway link. */			
			if ((stk_64flgctr & 0x7ff) == 0) {

				printf("A %3d %7d %7d %7d %7d %7d %3u%3u%3u%3u", xct++,CAN_ave, CAN_dif, CAN_dev, encoder_ctrA2, speed_filteredA2,\
				adcsensor_foto_err2[0],adcsensor_foto_err2[1],adcsensor_foto_err2[2],adcsensor_foto_err2[3]); 

//				printf ("HSTct: %5d %5d %5d %5d : ",adc2histoctr[k],adc3histoctr[k],adc12valbuff[0][47].us[1],adc3valbuff[0][1]);
//				for (j = 40; j < 70; j++)
//				{
//					printf("%5d %5d | ",adc2histo[k][j]/512,adc3histo[k][j]/512);
//				}
				printf ("\n\r");USART1_txint_send(); 
			}
		}
		/* Check incoming CAN msgs for readout commands and send response. */

		pcan = canrcv_get_sys();
		if (pcan != 0) 	// Check for PC CAN msg to readout histogram
		{
			CANascii_poll(pcan);	// Check for command code from PC to enable/disable CAN ascii
			adc_histo_cansend(pcan);// Check for commands to readout histogram and other functions
			z += 1;
			switch (pcan->id & ~0x1)
			{
		/* NOTE: The following CAN IDs are hard coded and not .txt file derived. */
			case 0xd1e00014: q = 0; break;
			case 0xd1e00034: q = 0; break;
			case 0xd1e00024:
			case 0xd1e00044:
				printf ("%u %u\n\r",z,q++);USART1_txint_send(); 
			}
		}
	}
	return 0;	
}

