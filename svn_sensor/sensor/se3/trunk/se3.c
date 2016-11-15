/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : se3.c
* Author             : deh
* Date First Issued  : 07/19/2013
* Board              : sensor board RxT6 w STM32F103RGT6
* Description        : sensor w shaft encoder
*******************************************************************************/
/* 
Hack of SE2 routine

Open minicom on the PC with 115200 baud and 8N1.

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

#include "adcsensor_eng.h"
#include "canwinch_pod_common_systick2048.h"
#include "common_can.h"
#include "SENSORpinconfig.h"
#include "sensor_threshold.h"
#include "rw_eeprom.h"
#include "se3.h"
//#include "tick_pod6.h"
#include "panic_leds.h"
#include "adcsensor_foto.h"
#include "eeprom_can.h"
#include "common_eeprom.h"

#include "common_highflash.h"



//#include "Tim4_pod_common.h"

//#include "dma17_fill.h"

//#include <malloc.h>

/* testmsg2.txt example--*/
//   ------------------- for calibrating raw readings ----------------------------------------------------------------------
//    Array Number read    value,
//    index  type  format  data		  	Description
//i     0     0     "%u"    5			@ Number of black segments
//i     1     4     "%f"  0.11453721234 	@ Shaft counter to feet conversion
//i     2     6     "%c"  "Shaft sensor DRIVE SHAFT" @ Some ascii description about the unit's use
//    Note: the next available index would be 9, since the above ascii takes up 6+ four byte slots.
//i     9     5     "%lf" 7.123456789012345 	@ Test double (2 slots)
//i    11     2     "%llu" 1234567890123456789 	@ Test unsigned long long
//i    13     0     "%d"   -256			@ Test of index 


struct FLASHP_SE3
{
	unsigned int crc;	// crc-32 placed by loader
	unsigned int numblkseg;	// Number of black segments
	float ctr_to_feet;	// Shaft counts to feet conversion
//	char c[4*7];		// ASCII identifier
	unsigned int c[7];	
	double dtest;		// Test double
	unsigned long long ulltest;	// Test long long
	unsigned int testidx;	// Test index
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
extern const struct FLASHP_SE3 __highflashp;
__attribute__ ((section(".ctbldata")))
const unsigned int flashp_size = sizeof (struct FLASHP_SE3);
__attribute__ ((section(".ctbldata1")))
const struct FLASHP_SE3* flashp_se3 = (struct FLASHP_SE3*)&__highflashp;


/* For test with and without XTAL clocking */
#define NOXTAL 
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
	SCB_AIRCR = (0x5FA << 16) | SCB_AIRCR_SYSRESETREQ;	// Cause a RESET
	while (1==1);
}
/* *************** CAN ascii ***************************************************************************** */
static struct CANRCVBUF* 	pfifo0;	// Pointer to CAN driver buffer for incoming CAN msgs, low priority
static struct CANRCVBUF canascii;	// CAN msg with ascii payload
static u8* pcas1 = &canascii.cd.uc[2];	// Pointer to payload loading
static int canputc_sw = 0;		// 0 = Don't send ascii CAN msgs
static u32 canid_ldr;

void putcCAN_init(void)
{
	/* Get CAN ID from ldr.c (unique unit CAN ID). */
	u32* vectjmp = (u32*)0x08000004;	// Loader reset vector address
	u32* tblptr = (u32*)((u8*)(*(u32*)vectjmp) + 7);
	canid_ldr = *tblptr;
	canid_ldr = canid_ldr | (CAN_EXTRID_DATA_CMD << CAN_DATAID_SHIFT);	// Unit loader Command ID
printf(  "LDR CAN IDcmd: %08X\n\r", canid_ldr );	USART1_txint_send();
	canascii.cd.uc[0] = LDR_ASCII_DAT;	// Command code
	return;
}

void putcCAN_poll(void)
{
return;
	while ( (pfifo0 = canrcv_get_sys()) != 0)		// Did we receive a LESS-THAN-HIGH-PRIORITY CAN BUS msg?
	{ // Here yes.  Retrieve it from the CAN buffer
//printf("C %08X %02X %02X\n\r",pfifo0->id, pfifo0->cd.uc[0], pfifo0->cd.uc[1]);USART1_txint_send();
		// Is this for us?
		if (pfifo0->id == canid_ldr)
		{ // Here, yes.  Is it the SWITCH command?
			if (pfifo0->cd.uc[0] == LDR_ASCII_SW)
				canputc_sw = pfifo0->cd.uc[1];	// Set switch
		}
	}
	return;
}

void putcCAN(char c)
{
	if (canputc_sw != 0xA5) return;	// Return if not in "send" mode?
//	if (c == '\r') return;		// Ignore these
	*pcas1++ = c;			// Store char in payload
	if ((c == '\n') || (pcas1 > &canascii.cd.uc[7]))
	{ // Here, newline, or payload full
		canascii.dlc = pcas1 - &canascii.cd.uc[0]; // Payload size
int j;
for (j = 0; j < 3; j++)
		can_msg_put_sys(&canascii);	// send
		pcas1 = &canascii.cd.uc[1];	// Reset for next 
int i;
for (i = 1; i < canascii.dlc; i++)
  USART1_txint_putc(canascii.cd.uc[i]);
USART1_txint_send();

	}
	return;
}

/* **************************************************************************************
 * void putc ( void* p, char c); // This is for the tiny printf
 * ************************************************************************************** */
// Note: the compiler will give a warning about conflicting types
// for the built in function 'putc'.  Use ' -fno-builtin-putc' to eliminate compile warning.
void putc ( void* p, char c)
{
	p=p;	// Get rid of the unused variable compiler warning
//putcCAN(c);
	USART1_txint_putc(c);
	return;
}


/*#################################################################################################
And now for the main routine 
  #################################################################################################*/
int main(void)
{
	int i = 0; 		// Timing loop variable
	int init_ret = -4;
//	int eeprom_ret;
//	int j;

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

//	putcCAN_init();


	/* Announce who we are */
	USART1_txint_puts("\n\rSE3 (SHAFT ENCODER): 11-10-2014 v1 \n\r");
	USART1_txint_send();	// Start the line buffer sending

	/* Display things to entertain the hapless op */
	extern void (*const vector_table[]) (void);
	printf ("Vector & flash start: %08x \n\r",(u32)&vector_table);
#ifdef NOXTAL
 	printf ("NO  XTAL\n\r");
#else
 	printf ("YES XTAL\n\r");
#endif
	USART1_txint_send();

	printf ("  hclk_freq (MHz) : %9u\n\r",  hclk_freq/1000000);//	USART1_txint_send();
	printf (" pclk1_freq (MHz) : %9u\n\r", pclk1_freq/1000000);	USART1_txint_send();
	printf (" pclk2_freq (MHz) : %9u\n\r", pclk2_freq/1000000);//	USART1_txint_send();
	printf ("sysclk_freq (MHz) : %9u\n\r",sysclk_freq/1000000);	USART1_txint_send();


	/*  Some code to test the passing of parameters from the .txt file through the loader. */
	struct FLASHP_SE3* pfsh = (struct FLASHP_SE3*)&__highflashp;
	printf ("flashp address: %08X\n\r", (u32)pfsh); USART1_txint_send();

	printf ("numblkseg  : %d\n\r", pfsh->numblkseg);
	printf ("ASCII      : %s\n\r", (char*)&pfsh->c[0]);
	int ww = pfsh->ctr_to_feet;	// NOTE: this printf doesn't handle floats or doubles
	int ff = pfsh->ctr_to_feet * 1000000 - ww*1000000;
	printf ("ctr-to-feet: %d.%06u\n\r", ww/1000000,ff);
	ww = pfsh->dtest;
	ff = pfsh->dtest * 1000000 - ww*1000000;
	printf ("dtest      : %d.%06u\n\r", ww/1000000,ff);
	printf ("testidx    : %d\n\r", pfsh->testidx);
	union ZZ { unsigned long long ull; unsigned int ui[2];} zz;
	zz.ull = pfsh->ulltest;	// NOTE: this printf doesn't handle long longs
	printf ("ulltest    : %08x %08x\n\r",zz.ui[1], zz.ui[0]);
	USART1_txint_send();

/* --------------------- Get Loader CAN ID -------------------------------------------------------------- */
	/* Pick up the unique CAN ID stored when the loader was flashed. */
	u32 canid_ldr = *(u32*)((u32)((u8*)*(u32*)0x08000004 + 7 + 0));	// First table entry = can id
	printf(  "CANID_LDR : %08X\n\r", canid_ldr );	USART1_txint_send();

/* --------------------- Get Application Can ID --------------------------------------------------------- */
	extern void* __highflashlayout;
	struct HIGHFLASHH* phfl = (struct HIGHFLASHH*)&__highflashlayout;
	can_params.iamunitnumber = phfl->appcanid.canid[0]; // First "slot" is used by this shaft sensor prog
printf("CANID_APP : %08X\n\r",can_params.iamunitnumber);USART1_txint_send();

/* --------------------- CAN ID setup ------------------------------------------------------------------- */

	/* eeprom */
//	if ((j=rw_eeprom_init()) != 0)
//	{
//		printf("eeprom init failed: %i\n\r",j); USART1_txint_send();// For debugging
//	}
	/* If eeprom has a valid 'iamunitnumber' then use that one. */
//	canid = eeprom_get(EEPROM_IAMUNITNUMBER);	// Read eeprom

//	if ((~canid.b.u32 != canid.a.u32) || (canid.a.u32 != IAMUNITNUMBER))
//	{ // Here, eeprom does not have a valid unit id.
//		eeprom_ret = eeprom_put(EEPROM_IAMUNITNUMBER, &xid);	// Write it.

//		if ( (eeprom_ret & 0xff00) != 0)	// Check for error
//		{ // Here, something went wrong
//			printf("##write status reg error: 0x%04x\n\r",eeprom_ret); USART1_txint_send();
//		}
//		printf ("EEPROM: WRITE"); 
//	}
//	else
//	{
//		printf ("EEPROM: VALID");
//	}
//	printf (" unit id number: b=%0x a=%0x xid=%0x\n\r", canid.b.u32, canid.a.u32, xid); USART1_txint_send();
	
/* --------------------- ADC setup and initialization ------------------------------------------------ */
	adc_init_sequence_foto(can_params.iamunitnumber);	// Time delay + calibration, then start conversion

/* --------------------- CAN setup ------------------------------------------------------------------- */
	/* Configure CAN criver RS pin: Sensor RxT6 board = (PB 7) */
	can_nxp_setRS_sys(0,1); // (1st arg) 0 = high speed mode; not-zero = standby mode

	/* Setup CAN registers and initialize routine */
	init_ret = can_init_pod_sys(&can_params); // 'struct' hold all the parameters

	/* Check if initialization was successful, or timed out. */
	if (init_ret <= 0)
	{
		printf("can init failed: code = %d\n\r",init_ret);USART1_txint_send(); 
		panic_leds(6);	while (1==1);	// Six flash panic display with code 6
	}
	printf("\n\rcan ret ct: %d\n\r",init_ret);USART1_txint_send(); // Look at how long "exit initialization" took

	/* Set filters to respond "this" unit number and time sync broadcasts */
	can_filter_unitid_sys(canid_ldr);	// Setup msg filter banks
	can_filtermask16_add_sys(can_params.iamunitnumber);	// Setup msg filter banks

	printf ("IAMUNITNUMBER %08x %0x\n\r",can_params.iamunitnumber,can_params.iamunitnumber >> CAN_UNITID_SHIFT);USART1_txint_send();



/* --------------------- Program is ready, so do program-specific startup ---------------------------- */
i = 0;


extern s32 CAN_dev;
extern s32 CAN_ave;
extern s32 CAN_dif;

//struct CANRCVBUF tcan;
//tcan.id = 0x22400000;
//tcan.dlc = 1;
//tcan.cd.uc[0]=0xA5;


/* Green LED flashing */
u32 stk_64flgctr_prev = 0;
/* --------------------- Endless Stuff ----------------------------------------------- */
	while (1==1)
	{
		putcCAN_poll();
		/* Check for a 1/2048th sec tick ('canwinch_pod_common_systick2048.c') */
		if (stk_64flgctr != stk_64flgctr_prev)
		{
			stk_64flgctr_prev = stk_64flgctr;
//can_msg_put_sys(&tcan);	// send
//if ((stk_64flgctr & 0x7ff) == 0)
//{printf("SW: %02X\n\r",canputc_sw);USART1_txint_send(); }
			/* Flash green LED in sync with time--ON/OFF each 1 sec */
			if ((stk_64flgctr & 0x7ff) == 0) {TOGGLE_GREEN;}	// Slow flash of green means "OK"
			
/*			printf("%7d %7d %7d %7d %7d %3u%3u%3u%3u\n\r", CAN_ave, CAN_dif, CAN_dev, encoder_ctrA, speed_filteredA,\
			adcsensor_foto_err[0],adcsensor_foto_err[1],adcsensor_foto_err[2],adcsensor_foto_err[3]); 

*/
		}
	}
	return 0;	
}
	
