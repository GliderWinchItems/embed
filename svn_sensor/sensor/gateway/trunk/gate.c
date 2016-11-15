/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : gate.c
* Author             : deh
* Date First Issued  : 07/22/2013
* Board              : sensor board RxT6 w STM32F103RGT6
* Description        : sensor w FTDI for a PC<->CAN gateway
*******************************************************************************/
/* 
Hack of SE2 routine

Open minicom on the PC with 115200 baud and 8N1.

NOTE:  ALL 'printf' lines must start with ' ' since the PC will distinguish a msg
from the CAN bus to the PC from a msg from the gateway to the PC by looking at the
first byte following the end-of-frame byte.  CAN to PC msgs start with 0x0, whereas
gateway msgs start with ' '.  If the ' ' is missing the gateway to PC msgs will likely
only result in the first char being missing.

*/
#include <string.h>


#include "adc.h"
#include "libopenstm32/can.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/gpio.h"
#include "scb.h"
#include "libopenstm32/usart.h"
#include "libusartstm32/usartallproto.h"
#include "libmiscstm32/printf.h"
#include "libmiscstm32/clockspecifysetup.h"

#include "canwinch_pod_common_systick2048.h"
#include "common_can.h"
#include "SENSORpinconfig.h"
#include "sensor_threshold.h"
#include "rw_eeprom.h"
#include "panic_leds.h"
#include "USART1_PC_gateway.h"
#include "PC_gateway_comm.h"
#include "CAN_gateway.h"



/* &&&&&&&&&&&&& Each node on the CAN bus gets a unit number &&&&&&&&&&&&&&&&&&&&&&&&&& */
#define IAMUNITNUMBER	CAN_UNITID_GATE1	// PC<->CAN bus gateway
/* &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& */

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
const struct CAN_PARAMS can_params = { \
IAMUNITNUMBER,	// CAN ID for this unit
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
 * void putc ( void* p, char c); // This is for the tiny printf
 * ************************************************************************************** */
// Note: the compiler will give a warning about conflicting types
// for the built in function 'putc'.  Use ' -fno-builtin-putc' to eliminate compile warning.
void putc ( void* p, char c)
	{
		p=p;	// Get rid of the unused variable compiler warning
		USART1_txdma_putc(c);
	}
/* **************************************************************************************
 * static void USART1_txdma_framesend (void); // Since all ASCII no byte stuff required
 * ************************************************************************************** */
static void USART1_txdma_framesend (void)
{
	USART1_txdma_putc(CAN_PC_FRAMEBOUNDARY);	// Place end-of-frame
	USART1_txdma_send();				// Send the mess
	return;

}
/*#################################################################################################
And now for the main routine 
  #################################################################################################*/
int main(void)
{
	int j;
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
	/* 
	rxdma --
	ARGS: 
		baud rate, e.g. 9600, 38400, 57600, 115200, 230400, 460800, 921600
		rx buffer size, (long enough for the longest line)
                Size of "getline" buffer
	*/
	USART1_rxdma_init(115200,256,128);
/*
	NOTE: txdma--printf uses _putc to fill a line buffer, so 'USART1_txdma_send();'
              is required.  USART1_txdma_puts(...) includes a 'USART1_txdma_send();'
	ARGS: 
		baud rate, e.g. 9600, 38400, 57600, 115200, 230400, 460800, 921600
		rx buffer size, (long enough for the longest line)
		Number of xmit line buffers, 	(e.g. 4)
*/
//$	USART1_txdma_init(115200,64,8); // "Standard" speed
	USART1_txdma_init(460800,64,8);	// Speed testing (must match 'canldr.c' for the PC side)

	/* Announce who we are */
	USART1_txdma_puts(" \n\rGATEWAY: 10-27-2013 v0\n\r");	USART1_txdma_framesend();

	/* Display things for to entertain the hapless op */
#ifdef NOXTAL
 	printf (" NO  XTAL\n\r");
#else
 	printf (" YES XTAL\n\r");
#endif
	USART1_txdma_framesend();
	printf ("   hclk_freq (MHz) : %9u\n\r",  hclk_freq/1000000);	USART1_txdma_framesend();
	printf ("  pclk1_freq (MHz) : %9u\n\r", pclk1_freq/1000000);	USART1_txdma_framesend();
	printf ("  pclk2_freq (MHz) : %9u\n\r", pclk2_freq/1000000);	USART1_txdma_framesend();
	printf (" sysclk_freq (MHz) : %9u\n\r",sysclk_freq/1000000);	USART1_txdma_framesend();


/* --------------------- eeprom --------------------------------------------------------------------- */
	if ((j=rw_eeprom_init()) != 0)
	{
		printf(" eeprom init failed: %i\n\r",j); USART1_txdma_framesend();
	}
/* --------------------- CAN setup ------------------------------------------------------------------- */
	/* Configure CAN criver RS pin: Sensor RxT6 board = (PB 7) */
	can_nxp_setRS_sys(0,1); // (1st arg) 0 = high speed mode; not-zero = standby mode

	/* Setup CAN registers and initialize routine */
	init_ret = can_init_pod_sys((struct CAN_PARAMS*)&can_params); // 'struct' hold all the parameters

	/* Check if initialization was successful, or timed out. */
	if (init_ret <= 0)
	{
		printf(" can init failed: code = %d\n\r",init_ret); USART1_txdma_framesend();
		panic_leds(6);	while (1==1);	// Six flash panic display with code 6
	}
	printf("\n\rcan ret ct: %d\n\r",init_ret); // Look at how long "exit initialization" took

	/* Set filters to respond "this" unit number and time sync broadcasts */
	can_filter_unitid_sys(can_params.iamunitnumber);	// Setup msg filter banks
	CAN_gateway_unitid = can_params.iamunitnumber;		// Set initial unitd number

	printf (" IAMUNITNUMBER %0x %0x\n\r",IAMUNITNUMBER,(unsigned int)CAN_UNITID_SE1 >> CAN_UNITID_SHIFT); USART1_txdma_framesend();

	/* Set filter for the hardware to accept all msgs. */
	int can_ret = can_filtermask16_add_sys( 0 );	// Allow all msgs
	/* Check if initialization was successful, or timed out. */
	if (can_ret < 0)
	{
		printf("can_filtermask16_add failed: code = %d\n\r",can_ret);USART1_txdma_framesend();
		system_reset();
	}
	printf("All pass filter added\n\r\n\r");USART1_txdma_framesend();

/* --------------------- Program is ready, so do program-specific startup ---------------------------- */

/* CAN rx test */
//extern s32 CAN_ave;
//extern s32 CAN_dev;
//extern s32 CAN_dif;

/* Subroutines buried below */
void do_pc_to_gateway(char* p, int size);
int canbufidxinc(int idx);



static int pc_msg_get_ret;	
static struct PCTOGATEWAY pctogateway; // Receives de-stuffed incoming msgs from PC.
static struct CANRCVBUF canrcvbuf;
static struct PCTOGATECOMPRESSED pctogatecompressed;

/* Circular buffer for passing CAN BUS msgs to PC */
#define CANBUSBUFSIZE	60	// Number of incoming CAN msgs to buffer
static struct CANRCVBUF canbuf[CANBUSBUFSIZE];
static int canbufidxi = 0;	// Incoming index into canbuf
static int canbufidxm = 0;	// Outgoing index into canbuf

struct CANRCVBUF* 	pfifo0;	// Pointer to CAN driver buffer for incoming CAN msgs, low priority
struct CANRCVTIMBUF*	pfifo1;	// Pointer to CAN driver buffer for incoming CAN msgs, high priority
u32 canoverrunct;		// Count of incoming CAN msgs overrunning buffer


/* Green LED flashing */
static u32 stk_64flgctr_prev;

	PC_msg_initg(&pctogateway);	// Initialize struct for CAN message from PC
/* --------------------- Endless Loop ----------------------------------------------- */
	while (1==1)
	{
		/* Check for a 1/2048th sec tick ('canwinch_pod_common_systick2048.c') */
		if (stk_64flgctr != stk_64flgctr_prev)
		{
			stk_64flgctr_prev = stk_64flgctr;

			/* Flash green LED in sync with time--ON/OFF each 1 sec */
			if ((stk_64flgctr & 0x7ff) == 0) {TOGGLE_GREEN;}	// Slow flash of green means "OK"
		}

			/* PC --> CAN */
		if ((pc_msg_get_ret=USART1_PC_msg_get(&pctogateway)) == 1)	// Do we have a valid message from the PC?
		{ // Here, yes.  The first byte contains the code as to where to direct the msg.
			if ( (pctogateway.ct) > 0)	// Ignore consecutive framing bytes that show a return ct of zero
			{
				/* Get msg setup in an uncompressed format. */
				if (pctogateway.ct == sizeof (struct CANRCVBUF))
				{ // Here it was not compressed.
					CANcopyuncompressed(&canrcvbuf, &pctogateway);	// Copy uncompressed form from input byte buffer to local struct
				}				
				else
				{ // Here, it is in compressed form.
					CANcopycompressed(&canrcvbuf, &pctogateway);	// Copy compressed form, uncompressing into 'canrcvbuf'	
				}

				/* Separate GATEWAY msg from CAN BUS msg using unitid number. */
				if (((canrcvbuf.id & CAN_UNITID_MASK) == CAN_UNITID_GATE) && ((CANmsgvalid(&canrcvbuf)) == 0))
				{ // Here, this message has the gateway address)
//					do_pc_to_gateway(&canrcvbuf);	// Do something wonderful.
				}
				else
				{ // Here, all others go out on the CAN BUS
					CAN_gateway_send(&canrcvbuf);	// Put message out on CAN bus;
				}
			}
			PC_msg_initg(&pctogateway);	// Initialize struct for the next message
		}

		/* CAN --> PC */
		if ( (pfifo1 = canrcvtim_get_sys()) != 0)	// Did we receive a high priority CAN BUS msg?
		{ // Here yes.  Retrieve it from the CAN buffer and save it in our vast mainline storage buffer.
			canbuf[canbufidxi] = pfifo1->R;	// Copy struct into our mainline buffer
			if (canbufidxm == canbufidxinc(canbufidxi)) // Does next index increment catch the outgoing index?
			{ // Yes, we have an overrun.  This CAN msg gets dropped.
				canoverrunct += 1;	// Up the error counter.
			}
			else
			{ // Here, there is space and we are good to go.
				canbufidxi = canbufidxinc(canbufidxi);	// Advance the index to next buffer position.
			}
		}
		if ( (pfifo0 = canrcv_get_sys()) != 0)	// Did we receive a less-than-high CAN BUS msg?
		{ // Here yes.  Retrieve it from the CAN buffer and save it in our vast mainline storage buffer.
			canbuf[canbufidxi] = *pfifo0;	// Copy struct
			if (canbufidxm == canbufidxinc(canbufidxi)) // Does next index increment catch the outgoing index?
			{ // Yes, we have an overrun.  This CAN msg gets dropped.
				canoverrunct += 1;	// Up the error counter.
			}
			else
			{ // Here, there is space and we are good to go.
				canbufidxi = canbufidxinc(canbufidxi);	// Advance the index to next buffer position.
			}
		}
		if (USART1_txdma_busy() == 0)	// Are all the uart output line buffers full?
		{ // Here, no.
			if (canbufidxi != canbufidxm)	// Are there incoming CAN msgs buffered to be sent?				
			{ // Here, yes.  Set a msg from the CAN bus up to go to the PC.
				if ((CANcompress(&pctogatecompressed, &canbuf[canbufidxm])) == 0) // Can it be compressed?
				{ // Here, yes. It can be compressed AND it was!  Send with byte framing, byte stuffing, etc.
					USART1_toPC_msg(&pctogatecompressed.cm[0],  pctogatecompressed.ct);
//do_pc_to_gateway((char*)&pctogatecompressed, (2 + (canbuf[canbufidxm].dlc & 0x1f) )  );
//do_pc_to_gateway((char*)&canbuf[canbufidxm], (2 + (canbuf[canbufidxm].dlc & 0x1f) )  );
				}
				else
				{ // Here, no.  Send the whole ugly uncompressed mess.
//					USART1_toPC_msg((u8*)&canbuf[canbufidxm], sizeof(struct CANRCVBUF) );

				}
				canbufidxm = canbufidxinc(canbufidxm);	// Advance outgoing buffer index.
			}
		}
	}
	return 0;	
}
/******************************************************************************
 * int canbufidxinc(int idx);
 * @brief 	: increment canbuf index
 * @param	: index
 * @return	: incremented index
*******************************************************************************/
int canbufidxinc(int idx)
{
	idx += 1; if (idx >= CANBUSBUFSIZE) idx = 0;
	return idx;
}

/******************************************************************************
 * void do_pc_to_gateway(struct CANRCVBUF *p, int size);
 * @brief 	: Msg is from PC to Gateway (that's us!)
 * @param	: p = pointer to a mess with the msg.
*******************************************************************************/
void do_pc_to_gateway(char* p, int size)

{
	/* Test-- send msg back to PC */
// p->c[1] = (p->p - &p->c[1]); // DEBUGGING--Overwrite 1st data byte with count
// void USART1_toPC_msg(u8* ps, char c, int size)
//	USART1_toPC_msg(&p->cd.u8[0], sizeof(struct CANRCVBUF) );

	int i;
	for (i = 0; i < size; i++)
		printf("%02x ",*p++);
	printf("|%d\n\r", size); USART1_txdma_send();
	return;	
}

