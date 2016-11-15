/******************************************************************************
* File Name          : gate.c
* Date               : 10/11/2013
* Board              : F4-Discovery
* Description        : USB PC<->CAN gateway
*******************************************************************************/
/* 
TODO 
10-21-2013 Move 'Default_Handler' out of this routine
           Pass 'sysclk' to usb routine and delete 'SysInit' in startup.s
	   vector.c rather than .s and the problem with .weak
*/
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>

#include "libopencm3/stm32/f4/rcc.h"
#include "libopencm3/stm32/f4/gpio.h"
#include "libopencm3/stm32/f4/scb.h"

#include "systick1.h"
#include "clockspecifysetup.h"

#include "DISCpinconfig.h"	// Pin configuration for STM32 Discovery board

#include "canwinch_ldr.h"
#include "common_can.h"
#include "panic_leds.h"
#include "PC_gateway_comm.h"
#include "USB_PC_gateway.h"
#include "CAN_gateway.h"

#include "libopencm3/stm32/systick.h"

//Don't use: #include "usb1.h" because it will bring in ST defines that are conflict with libopencm3.
void usb1_init(void);


/* &&&&&&&&&&&&& Each node on the CAN bus gets a unit number &&&&&&&&&&&&&&&&&&&&&&&&&& */
#define IAMUNITNUMBER	CAN_UNITID_GATE2	// PC<->CAN bus gateway
/* &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& */


/* The following values provide --
External 8 MHz xtal
sysclk  =  168 MHz
PLL48CK =   48 MHz
PLLCLK  =  168 MHz
AHB     =  168 MHz
APB1    =   42 MHz
APB2    =   84 MHz

NOTE: PLL48CK must be 48 MHz for the USB
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



/* Parameters for setting up CAN */

// Default: based on 72 MHz clock|36 MHz AHB freqs--500,000 bps, normal, port B
//const struct CAN_PARAMS can_params = CAN_PARAMS_DEFAULT;	// See 'canwinch_pod.h'

// Experimental CAN params: Based on 64 MHz clock|32 MHz AHB freqs
const struct CAN_PARAMS can_params = { \
IAMUNITNUMBER,	// CAN ID for this unit
CANBAUDRATE,	// baudrate (in common_all/trunk/common_can.h)
3,		// port: port: 0 = PA 11|12; 2 = PB; 3 = PD 0|1;  (1 = not valid; >3 not valid) 
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



static struct PCTOGATEWAY pctogateway; // Receives de-stuffed incoming msgs from PC.
static struct CANRCVBUF canrcvbuf;
static struct PCTOGATECOMPRESSED pctogatecompressed;


/* Sequence number checking for incoming msgs from the PC */
static u32 seqnum;
static u32 seqnum_old = 0;

/* Circular buffer for passing CAN BUS msgs to PC */
#define CANBUSBUFSIZE	32	// Number of incoming CAN msgs to buffer
static struct CANRCVBUF canbuf[CANBUSBUFSIZE];
static u32 canmsgct[CANBUSBUFSIZE]; // Msg seq number for CAN-to-PC.
static int canbufidxi = 0;	// Incoming index into canbuf
static int canbufidxm = 0;	// Outgoing index into canbuf
static int incIdx(int x){x += 1; if (x >= CANBUSBUFSIZE) x = 0; return x;} 

static struct CANRCVBUF* 	pfifo0;	// Pointer to CAN driver buffer for incoming CAN msgs, low priority
static struct CANRCVTIMBUF*	pfifo1;	// Pointer to CAN driver buffer for incoming CAN msgs, high priority

/* Put sequence number on incoming CAN messages that will be sent to the PC */
u8 canmsgctr = 0;	// Count incoming CAN msgs

/* Error counters */
u32 err_pcmsgtoosmall = 0;	// Incoming msg from PC too small
u32 err_canbufovrrun = 0;	// Buffer overrun situation
u32 err_sequence  = 0;		// Sequence number from PC error
u32 err_illegalid = 0;		// Msg sent to 'CAN_gateway_send': extend id bits w IDE = 0
u32 err_dlctoobig = 0; 		// Msg sent to 'CAN_gateway_send': payload ct too big


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


/* LED identification
Discovery F4 LEDs: PD 12, 13, 14, 15

|-number on pc board below LEDs
|   |- color
v vvvvvv  macro
12 green   
13 orange
14 red
15 blue
*/

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
	lednum += 1;		// Step through all four LEDs
	if (lednum > 15) lednum = 12;

}
/*#################################################################################################
And now for the main routine 
  #################################################################################################*/
int main(void)
{
	int init_ret = -4;

	/* Shameful hack for delaying writing to USB to avoid USB buffer overrun. */
	u32 t_start;
	u32 t_ctr;
	u8 sw;

int temp;

/* LED flashing */
volatile unsigned int xz;

/* --------------------- Begin setting things up -------------------------------------------------- */ 
	clockspecifysetup((struct CLOCKS*)&clocks);		// Get the system clock and bus clocks running
/* ---------------------- Set up pins ------------------------------------------------------------- */
	/* Configure pins */
	DISCgpiopins_Config();	// Configure pins

LEDSALL_on; // debugging
/* ---------------------- Set usb ----------------------------------------------------------------- */
	usb1_init();	// Initialization for USB (STM32F4_USB_CDC demo package)
	setbuf(stdout, NULL);
LEDSALL_off; // debugging
/* ---------------------- DTW sys counter -------------------------------------------------------- */

	/* Use DTW_CYCCNT counter for timing */
/* CYCCNT counter is in the Cortex-M-series core.  See the following for details 
http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0337g/BABJFFGJ.html */
	*(volatile unsigned int*)0xE000EDFC |= 0x01000000; // SCB_DEMCR = 0x01000000;
	*(volatile unsigned int*)0xE0001000 |= 0x1;	// Enable DTW_CYCCNT (Data Watch cycle counter)

/* ---------------------- Let the hapless Op know it is alive ------------------------------------ */
	int i;
	/* Do this several times because it takes the PC a while to recognize and start 'ttyACM0' and some of
           the chars are missed. */
	for (i = 0; i < 3; i++) 
	{
		/* Announce who we are */
		printf(" \n\rF4 DISCOVERY GATEWAY: 10-21-2013  0\n\r");
		/* Make sure we have the correct bus frequencies */
		printf ("   hclk_freq (MHz) : %9u\n\r",  hclk_freq/1000000);	
		printf ("  pclk1_freq (MHz) : %9u\n\r", pclk1_freq/1000000);	
		printf ("  pclk2_freq (MHz) : %9u\n\r", pclk2_freq/1000000);	
		printf (" sysclk_freq (MHz) : %9u\n\r",sysclk_freq/1000000);
	}
/* --------------------- CAN setup ------------------------------------------------------------------- */
	/*  Pin usage for CAN--
	PD00 CAN1  Rx LQFP 81 Header P2|36 BLU
	PD01 CAN1  Tx LQFP 82 Header P2|33 WHT
	PC04 GPIIO RS LQFP 33 Header P1|20 GRN
	*/
	/* Configure CAN driver RS pin: PC4 LQFP 33, Header P1|20, fo hi speed. */
	can_nxp_setRS_ldr(0,(volatile u32 *)GPIOC, 4); // (1st arg) 0 = high speed mode; not-zero = standby mode

	/* Setup CAN registers and initialize routine */
	init_ret = can_init_pod_ldr((struct CAN_PARAMS*)&can_params); // 'struct' that holds all the parameters

	/* Check if initialization was successful, or timed out. */
	if (init_ret <= 0)
	{
		printf(" can init failed: code = %d\n\r",init_ret); 
		panic_leds(6);	while (1==1);	// Flash panic display with code 6
	}
	printf("\n\rcan ret ct: %d\n\r",init_ret); // Just a check for how long "exit initialization" took

	/* Set filters to respond "this" unit number and time sync broadcasts */
	can_filter_unitid_ldr(can_params.iamunitnumber);	// Setup msg filter banks

	printf (" IAMUNITNUMBER %0x %0x\n\r",(unsigned int)IAMUNITNUMBER,(unsigned int)CAN_UNITID_SE1 >> CAN_UNITID_SHIFT); 

	/* Set filter for the hardware to accept all msgs. */
	int can_ret = can_filtermask16_add_ldr( 0 );	// Allow all msgs
	/* Check if initialization was successful, or timed out. */
	if (can_ret < 0)
	{
		printf("can_filtermask16_add failed: code = %d\n\r",can_ret);
		panic_leds(7);	while (1==1);	// Flash panic display with code 7
	}
	printf("All pass filter added\n\r\n\r");

/* --------------------- Program is ready, so do program-specific startup ---------------------------- */

	/* Shameful hack for delaying writing to USB to avoid USB buffer overrun. */
	t_start = *(volatile unsigned int *)0xE0001004;
	t_ctr = 0;
	sw = 0;

	/* LED flash */
#define FLASHCOUNT 21000000;
u32	t_led = *(volatile unsigned int *)0xE0001004 + FLASHCOUNT;

	PC_msg_initg(&pctogateway);	// Initialize struct for CAN message from PC
/* --------------------- Endless Polling Loop ----------------------------------------------- */
	while (1==1)
	{
		/* Flash the LED's to amuse the hapless Op */
		if (((int)(*(volatile unsigned int *)0xE0001004 - t_led)) > 0)
		{
			toggle_4leds(); 
			t_led += FLASHCOUNT;
		}

		/* ================ PC --> CAN ================================================================= */
		if ((temp=USB_PC_msg_getASCII(&pctogateway)) == 0)	// Do we have a valid message from the PC?
		{ // Here, yes.
			if ( (pctogateway.ct) > 4)	// Ignore msgs that are too small
			{
// NOTE: This section NOT TESTED!!!!
				/* Extract msg into standard CAN format */
				seqnum = CANpctocan(&canrcvbuf, &pctogateway);

				/* At this point 'canrcvbuf' has an uncompressed standard form CAN msg */
				
				/* Check the sequence number */
				seqnum_old += 1;
				if (seqnum != seqnum_old)
				{
					seqnum_old = seqnum;
					err_sequence += 1;	// Running count of errors
				}

				
				/* This would be a good place to put a check on bogus CAN id's and any other
                                   criteria to prevent putting the msg out on the CAN bus.  Note 'CAN_gateway_send'
				   limit aborts if the payload is over 8 bytes. */

				/* Place the msg in the buffer where it will be xmitted. */
				if ((CAN_gateway_send(&canrcvbuf)) == -1 ) err_dlctoobig += 1; 
				if ((CAN_gateway_send(&canrcvbuf)) == -2 ) err_illegalid += 1;	
			}
			else
			{
				err_pcmsgtoosmall += 1;	// Count bogus msgs things from the PC
			}
			PC_msg_initg(&pctogateway);	// Initialize struct for the next incoming message
		}

		/* ================= CAN --> PC ================================================================= */
		if ( (pfifo1 = canrcvtim_get_ldr()) != 0)	// Did we receive a HIGH PRIORITY CAN BUS msg?
		{ // Here yes.  Retrieve it from the CAN buffer and save it in our vast mainline storage buffer.
			canbuf[canbufidxi] = pfifo1->R;		// Copy struct into our mainline buffer
			canmsgct[canbufidxi] = canmsgctr;	// Save sequence count that goes with this msg
			canmsgctr += 1;				// Count incoming CAN msgs
			temp = incIdx(canbufidxi);		// Increment the index for incoming msgs.
			if (canbufidxm == temp)  		// Did this last fill the last one?
			{ // Yes, we have filled the buffer.  This CAN msg might be dropped (by not advancing the index)
				err_canbufovrrun += 1;		// Up the error counter.
			}
			else
			{ // Here, there is room in the buffer and we are good to go.
				canbufidxi = temp;		// Update the index to next buffer position.
			}
		}
		if ( (pfifo0 = canrcv_get_ldr()) != 0)		// Did we receive a LESS-THAN-HIGH-PRIORITY CAN BUS msg?
		{ // Here yes.  Retrieve it from the CAN buffer and save it in our vast mainline storage buffer.
			canbuf[canbufidxi] = *pfifo0;		// Copy struct
			canmsgct[canbufidxi] = canmsgctr;	// Save sequence count that goes with this msg
			canmsgctr += 1;				// Count incoming CAN msgs
			temp = incIdx(canbufidxi);		// Increment the index for incoming msgs.
			if (canbufidxm == temp)  		// Did this last fill the last one?
			{ // Yes, we have filled the buffer.  This CAN msg might be dropped (by not advancing the index)
				err_canbufovrrun += 1;		// Up the error counter.
			}
			else
			{ // Here, there is room in the buffer and we are good to go.
				canbufidxi = temp;		// Update the index to next buffer position.
			}
		}

		/* Shameful hack to avoid problems with USB routine buffering. */
		/* Check if we are waiting for a previous usb write to clear.  While we are waiting msgs are being buffered. */
 		if ( (int)(*(volatile unsigned int *)0xE0001004 - t_start) > 0) // Check DTW ctr time against end of interval
		{ // Here, the time has expired so we ought to be able to send without clobbering previous stuff.
			sw = 0;		// Reset "sent something" switch
			while (canbufidxi != canbufidxm)	// Set up all the buffered msgs until we are caught up.				
			{ // Here, yes.  Set up a buffered msg from the CAN bus to go to the PC.
				if ((CANcompress(&pctogatecompressed, &canbuf[canbufidxm])) == 0) // Compress if possible and copy
				{ // Here, we made a game try to reduce the number of bytes that need to be sent.
					pctogatecompressed.seq = canmsgct[canbufidxm];	// Add sequence number (for PC checking for missing msgs)
					USB_toPC_msgASCII(&pctogatecompressed); 	// Send to PC
					t_ctr += pctogatecompressed.ct+6;		// Tot binary bytes sent (for computing USB hack wait delay)
				}
				canbufidxm = incIdx(canbufidxm);			// Advance outgoing buffer index.

				sw += 1;	//Show that we got at least one msg set up.
			}
			
			if (sw != 0) // Did we setup any msgs?  Compute timer ct at end-of-wait.
				t_start = *(volatile unsigned int *)0xE0001004 + t_ctr*1620 + 3000; // Set end of wait count
			else
				t_start = *(volatile unsigned int *)0xE0001004 + 1000; // Nothing setup, set a short wait
			t_ctr = 0; // Reset count of bytes (binary) that were set up in USB buffer (as ascii)		
		}
	}
	return 0;	
}


