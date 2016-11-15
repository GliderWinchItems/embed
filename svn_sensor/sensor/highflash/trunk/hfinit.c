/******************************************************************************
* File Name          : hfinit.c
* Date First Issued  : 10/15/2014
* Board              : F103
* Description        : Initialize highflash structs "manually"
*******************************************************************************/
/* 
---Appears to work, flash_write_n.c also.  cangate cmd p side needs work.
struct of structs in common_highflash.ch used
Last Changed Author: deh
Last Changed Rev: 521
Last Changed Date: 2014-08-01 22:40:29 -0400 (Fri, 01 Aug 2014)


*/

/* Temporary test to determine if high flash area defined in ldrapp.ld is large enough */
//#include "common_highflash.h"
//__attribute__ ((section(".highflashlayout")))
//const struct HIGHFLASHLAYOUT hflayout;

/* Default CAN ID's etc. */



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
#include "panic_leds.h"

#include "common_highflash.h"

//#include <malloc.h>

void printftestptr(u32* p, char* pc);

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

/* Instance in SRAM of image to be loaded into high flash. */
struct HIGHFLASHLAYOUT hfl;

const char* pbanner = "HFINIT.c: 10-16-2014 1";

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
	USART1_rxinttxint_init(115200,32,2,96,4); // Initialize USART and setup control blocks and pointers

	/* Announce who we are */
	printf("\n\r%s\n\r Set up high flash structs directly.", pbanner);
	USART1_txint_send();	// Start the line buffer sending

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


/* --------------------- Program is ready, so do program-specific startup ---------------------------- */

	/* Build an image of what we want to place in high flash */
#include "flash_write.h"
#include "flash_write_n.h"
#include "crc-32.h"

extern struct HIGHFLASHLAYOUT* __highflashlayout;

#define VER_APPCANID	3
#define VER_CRCPROG	4
#define VER_BOARDCALIB	5
#define VER_APPPARM	6
#define VER_HFL		1

	struct HIGHFLASHLAYOUT* phfl = (struct HIGHFLASHLAYOUT*)&hfl;	// Pointer makes it easier to put this in a subroutine later

	/* CAN ID struct */
	phfl->appcanid.version 		= VER_APPCANID;
	phfl->appcanid.numcanids	= NUMCANIDS;
	int i;
	for (i = 0; i < NUMCANIDS; i++) // Fill with default CAN ID's
		phfl->appcanid.canid[i]	= (0x0FFFE0004 | 0x4 | (i << 3));
printf("appcanid.crc: address: %08x  count:%d \n\r",(u32)&phfl->appcanid.version, (sizeof(struct APPCANID) - sizeof(u32)) );USART1_txint_send();
	phfl->appcanid.crc = rc_crc32( (unsigned char*)&phfl->appcanid.version, (sizeof(struct APPCANID) - sizeof(u32)) );

	/* Prog crc check struct */
	/* Fill CRC check blocks with a dummy values that will get past loader crc check */
#define DUMMYSTART ((u8*)0x08000000)	// Make dummy crc
#define DUMMYCOUNT 4		// Using a dummy count
	phfl->crcprog.version 		= VER_CRCPROG;
	phfl->crcprog.numprogsegs	= NUMPROGSEGS;
	for (i = 0; i < NUMCANIDS; i++)
	{
		phfl->crcprog.crcblk[i].pstart  = DUMMYSTART;
		phfl->crcprog.crcblk[i].count	= DUMMYCOUNT;
		phfl->crcprog.crcblk[i].crc     = rc_crc32(DUMMYSTART, DUMMYCOUNT);
	}
	phfl->crcprog.crc = rc_crc32((unsigned char*)&phfl->crcprog.version, (sizeof(struct CRCPROG) - sizeof (u32)) );

	/* Board calibrations */
	phfl->boardcalib.version 	= VER_BOARDCALIB;
	phfl->boardcalib.numboardcalib	= NUMBOARDCALIB;
	for (i = 0; i < NUMBOARDCALIB; i++)
		phfl->boardcalib.cal[i] = ~0;
	phfl->boardcalib.crc = rc_crc32((unsigned char*)&phfl->boardcalib.version, (sizeof(struct BOARDCALIB) - sizeof (u32)) );

	/* Application parameters */
	phfl->appparam.version 		= VER_APPPARM;
	phfl->appparam.numappparam	= NUMAPPPARAM;
	for (i = 0; i < NUMAPPPARAM; i++)
		phfl->appparam.cal[i] = ~0;
	phfl->appparam.crc = rc_crc32((unsigned char*)&phfl->appparam.version, (sizeof(struct APPPARAM) - sizeof (u32)) );

	/* struct of structs: overall version and crc. */
	phfl->version 			= VER_HFL;
	phfl->crc = rc_crc32((unsigned char*)&phfl->version, (sizeof(struct HIGHFLASHLAYOUT) - sizeof (u32)) );

	/* Write image to high flash */
printf("sizeof(struct HIGHFLASHLAYOUT): %d\n\r",sizeof(struct HIGHFLASHLAYOUT));USART1_txint_send();
	int ret  = flash_write_n((u8*)&__highflashlayout, (u8*)phfl, sizeof(struct HIGHFLASHLAYOUT) );
	s16 er1 = ret >> 16; s16 er2 = (ret & 0xffff);
	printf("Flash write ret: erase err ct: %d write err ct: %d\n\r",er1, er2); 
	if (ret !=0 ) printf("===> FLASH ERROR <====\n\r");
	printf("DONE writing flash\n\r\nTEST GETTING POINTERS TO STRUCTS\n\r");USART1_txint_send();

	/* Check getting pointers to structs, which checks if crc and versions match...they should! */
	
	/*******************************************************************************
	 * struct APPCANID*   gc_hiflash_appcanid(u32 version);
	 * struct CRCPROG*    gc_hiflash_crcprog(u32 version);
	 * struct BOARDCALIB* gc_hiflash_boardcalib(u32 version);
	 * struct APPPARAM*   gc_hiflash_appparam(u32 version);
	 * @brief 	: Check crc and return pointer to struct
	 * @param	: version = version number expected
	 * @return	: NULL = crc failed or version did not match, otherwise pointer 
	*******************************************************************************/
	// App CAN ID table
	struct APPCANID* gapp = gc_hiflash_appcanid(VER_APPCANID);
	printftestptr((u32*)gapp, "APPCANID*\t ");

	// Program CRC check
	struct CRCPROG* gcrc = gc_hiflash_crcprog(VER_CRCPROG);
	printftestptr((u32*)gcrc, "CRCPROG*\t ");

	// Board calibration 
	struct BOARDCALIB* gbcb = gc_hiflash_boardcalib(VER_BOARDCALIB);
	printftestptr((u32*)gbcb, "BOARDCALIB*\t ");

	// App parameters
	struct APPPARAM* gram =  gc_hiflash_appparam(VER_APPPARM);
	printftestptr((u32*)gram, "APPPARAM*\t ");

	// struct of structs
	struct HIGHFLASHLAYOUT* ghfl =  gc_hiflash_highflash(VER_HFL);
	printftestptr((u32*)ghfl, "HIGHFLASHLAYOUT* ");

	printf("\n\rDONE! %s\n\r", pbanner);USART1_txint_send();	



/* Green LED flashing */
unsigned int flashinc = sysclk_freq/2;
u32	t_flash = *(volatile unsigned int *)0xE0001004 + flashinc; // Set initial time


/* --------------------- Endless Stuff ----------------------------------------------- */
	while (1==1)
	{
		/* Green LED OK flasher */
		if ((int)(*(volatile unsigned int *)0xE0001004 - t_flash) > 0)
		{
			t_flash += flashinc;
			TOGGLE_GREEN;	// Slow flash of green means "OK"			
		}


	}
	return 0;	
}
/*******************************************************************************
 * void printftestptr(u32* p, char* pc);
 * @brief	: Nice printout of getting pointers to structs
********************************************************************************/
void printftestptr(u32* p, char* pc)
{
	printf("%s", pc);
	if (p == NULL)
		printf("Get pointer call returned NULL!\n\r");
	else
		printf("%08X\n\r", (u32)p);
	USART1_txint_send();
	return;

}

