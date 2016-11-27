/******************************************************************************
* File Name          : gatef.c
* Date               : 11/04/2013
* Board              : F4-Discovery
* Description        : USB PC<->CAN gateway--FTDI version
*******************************************************************************/
/* 
TODO 
10-21-2013 Move 'Default_Handler' out of this routine
           Pass 'sysclk' to usb routine and delete 'SysInit' in startup.s
	   vector.c rather than .s and the problem with .weak
11-24-2013 Hack of 'gateway/gate.c' to change from usb to ftdi
12-01-2013 rev 132 Changed compression scheme to work with extended addresses
12-16-2013 rev 148 - bin/ascii switch, and better compression.  Works with mode 0 & 1 (bin & asc)
12-17-2013 rev 149 - moved CANuncompress, CANcompress calls into USB_PC_gateway.c, add CAN_error_msgs.c to count errors
12-17-2013 rev 150 - added mode 2 (Gonzaga ascii/hex format)
02-11-2014 rev 202 - fixed bug where PC->gatef msgs occasionally were garbled gatef->PC msgs
06-15-2015 rev 313 - last of 'canwinch_ldr', and begin switch to new linked list can_driver
06-17-2015 rev 317 - Switch to new linked list can_driver "appears to work"

*/
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>


#include <math.h>
#include <string.h>
#include <stdio.h>
#include "xprintf.h"

#include <malloc.h>

#include "libopencm3/stm32/f4/rcc.h"
#include "libopencm3/stm32/f4/gpio.h"
#include "libopencm3/stm32/f4/scb.h"

#include "../../../sw_discoveryf4/trunk/lib/libusartstm32f4/bsp_uart.h"
#include "systick1.h"
#include "clockspecifysetup.h"

#include "DISCpinconfig.h"	// Pin configuration for STM32 Discovery board
#include "can_driver.h"
#include "common_can.h"
#include "panic_leds.h"
#include "PC_gateway_comm.h"
#include "USB_PC_gateway.h"
#include "libopencm3/stm32/systick.h"
#include "CAN_test_msgs.h"
#include "CAN_error_msgs.h"
#include "canwinch_setup_F4_discovery.h"
#include "DTW_counter.h"
#include "db/gen_db.h"

#ifndef NULL 
#define NULL	0
#endif

/* Circular buffer for passing CAN BUS msgs to PC */
#define CANBUSBUFSIZE	64	// Number of incoming CAN msgs to buffer
static struct CANRCVBUF canbuf[CANBUSBUFSIZE];
static u32 canmsgct[CANBUSBUFSIZE]; // Msg seq number for CAN-to-PC.
static int canbufidxi = 0;	// Incoming index into canbuf
static int canbufidxm = 0;	// Outgoing index into canbuf

static void canbuf_add(struct CANRCVBUF* p);
void canin(struct CANRCVBUF* p);		// Debug
void caninB(struct CANRCVBUF* p);  	// Debug

/* Specify msg buffer and max useage for CAN1: TX, RX0, and RX1. */
const struct CAN_INIT msginit1 = { \
180,	/* Total number of msg blocks. */
80,	/* TX can use this huge ammount. */
80,	/* RX0 can use this many. */
16	/* RX1 can use this piddling amount. */
};

/* Specify msg buffer and max useage for CAN2: TX, RX0, and RX1. */
const struct CAN_INIT msginit2 = { \
180,	/* Total number of msg blocks. */
140,	/* TX can use this huge ammount. */
32,	/* RX0 can use this many. */
8	/* RX1 can use this piddling amount. */
};


/* USART|UART assignment for xprintf and read/write */
#define UXPRT	6	// Uart number for 'xprintf' messages
#define USTDO	2	// Uart number for gateway (STDOUT_FILE, STDIIN_FILE)

/* &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& */
/* ------------- Each node on the CAN bus gets a unit number --------------------- */
//#define IAMUNITNUMBER	CAN_UNITID_GATE2	// PC<->CAN bus gateway
/* &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& */
/* ######################### GATEWAY HEARTBEAT CAN ID ############################ */
//static struct CANRCVBUF can_hb = {CANID_HB_GATEWAY1}; // See CANID_INSERT.sql
static struct CANRCVBUF can_hb = {CANID_HB_GATEWAY2}; // See CANID_INSERT.sql
//static struct CANRCVBUF can_hb = {CANID_HB_GATEWAY3}; // See CANID_INSERT.sql
/* ############################################################################### */
/* %%%%%%%%%%%%% Board hardware setup %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
#define SETUP_CAN1	// Include code for setup of CAN1
#define SETUP_CAN2	// Include code for setup of CAN2
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

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

/* Easy way for other routines to access via 'extern'*/
struct CAN_CTLBLOCK* pctl1;	// CAN1 control block pointer
struct CAN_CTLBLOCK* pctl2;	// CAN1 control block pointer

static struct PCTOGATEWAY pctogateway; 	// CAN->PC
static struct PCTOGATEWAY pctogateway1; // PC->CAN
static struct CANRCVBUF canrcvbuf;

#ifdef SETUP_CAN1
static struct CANRCVBUF* 	pc1r0;	// Pointer to CAN1 buffer for incoming CAN msgs, low priority
static struct CANRCVBUF*	pc1r1;	// Pointer to CAN1 buffer for incoming CAN msgs, high priority
#endif
#ifdef SETUP_CAN2
static struct CANRCVBUF* 	pc2r0;	// Pointer to CAN2 buffer for incoming CAN msgs, low priority
static struct CANRCVBUF*	pc2r1;	// Pointer to CAN2 buffer for incoming CAN msgs, high priority
#endif

//static struct CANRCVBUF* ptest_pc;	// Test gateway->PC
//static struct CANRCVBUF* ptest_can;	// Test gateway->CAN bus

/* Gateway heart-beat msg */
#define DELAYHEARTBEAT	128	// Number of 1/64th sec ticks between HBs
static u32 t_hb;	// DTW ticks

/* Put sequence number on incoming CAN messages that will be sent to the PC */
u8 canmsgctr = 0;	// Count incoming CAN msgs

/* file descriptor */
int fd;

char vv[128];	// sprintf buf

static u32	t_cmdn;

/* **************************************************************************************
 * static void printmsg(struct CANRCVBUF* p);
 * @brief	: Simple hex print
 * @param	: p = pointer to msg
 * ************************************************************************************** */
static void printmsg(struct CANRCVBUF* p, int sw)
{
	int i;
	int ct = (p->dlc & 0xf); if (ct > 8) ct = 8;
	xprintf (UXPRT, "F%1d %08X %d ",sw,p->id,ct);
	for (i = 0; i < ct; i++)
	  xprintf(UXPRT, "%02X ",p->cd.uc[i]);
	xprintf(UXPRT, "\n\r");
	return;
}
/* **************************************************************************************
 * static int CAN_gateway_send(struct CAN_CTLBLOCK* pctl, struct CANRCVBUF* pg);
 * @brief	: Setup CAN message for sending
 * @param	: pg = Pointer to message buffer (see common_can.h)
 * @return	: 0 = OK; -1 = dlc greater than 8; -2 = illegal extended address
 * ************************************************************************************** */

static int CAN_gateway_send(struct CAN_CTLBLOCK* pctl, struct CANRCVBUF* pg)
{
	/* Check number of bytes in payload and limit to 8. */
	if ((pg->dlc & 0x0f) > 8) 
		return -1;	// Payload ct too big
	
	/* Check if an illegal id combination */
	if ( ((pg->id & 0x001ffff9) != 0) && ((pg->id & 0x04) == 0) ) 
	{ // Here, in the additional 18 extended id bits one or more are on, but IDE flag is for standard id (11 bits)
		return -2; // Illegal id
	}

	/* Add msg to CAN outgoing buffer. */
	return can_driver_put(pctl, pg, 8, 0);
}

/******************************************************************************
 * static int adv_index(int idx, int size)
 * @brief	: Format and print date time in readable form
 * @param	: idx = incoming index
 * @param	: size = number of items in FIFO
 * return	: index advanced by one
 ******************************************************************************/
static int adv_index(int idx, int size)
{
	int localidx = idx;
	localidx += 1; if (localidx >= size) localidx = 0;
	return localidx;
}
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
	int tmp;
//u32 xctr = 1;
u32 yctr = 0;
u32 zctr = 0;

	int temp;


/* --------------------- Begin setting things up -------------------------------------------------- */ 
	clockspecifysetup(canwinch_setup_F4_discovery_clocks()); // Get the system clock and bus clocks running
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

   The strategy is to setup the USART/UART so that it will handle STDOUT, and STDIN, making 'printf' etc. work
   directly.  Bulk calls are made to _write, _read in subroutines, and these routines will work with the correct
   usart/uart via the 'fd' that relates the fd to uart control block during the call to _open.  Normally one would
   use 'fprintf' etc., but that isn't working, and this shameful sequence is an interim solution that allows easily
   changing the STDOUT, STDIN uart.

   USART2 and USART6 are shown below.  Select one, or make up one for the USART/UART this will be used.  
   Either DMA or CHAR-BY-CHAR interrupt driven can be used.  DMA for faster high volume loads.

*/
/*	DMA DRIVEN  (note: non-blocking, i.e. discard chars if port buff is full.) */
// int bsp_uart_dma_init_number(u32 iuart, u32 baud, u32 rxbuffsize, u32 txbuffsize, u32 dmastreamrx, u32 dmastreamtx, u32 dma_tx_int_priority
//	u8 block, u8 nstop);
	bsp_uart_dma_init_number(USTDO,2000000, 2048, 1024, 5, 6, 0xd0, 1, 0); // Flashing LED's means failed and you are screwed.
//	bsp_uart_dma_init_number(USTDO, 921600, 2048, 1024, 5, 6, 0xd0, 1, 0); // Flashing LED's means failed and you are screwed.
//	bsp_uart_dma_init_number(USTDO, 460800, 256, 256, 5, 6, 0xd0, 1, 0); // Flashing LED's means failed and you are screwed.
//	bsp_uart_dma_init_number(USTDO, 230400, 1024, 256, 5, 6, 0x10),1,0); // Flashing LED's means failed and you are screwed.
//	bsp_uart_dma_init_number(USTDO, 115200, 256, 256, 5, 6, 0xd0, 1, 0); // Flashing LED's means failed and you are screwed.
//	bsp_uart_dma_init_number(UXPRT, 115200, 256, 256, 1, 6, 0xd0, 1, 0); // Flashing LED's means failed and you are screwed.

/*	CHAR-BY-CHAR INTERRUPT DRIVEN  (note: non-blocking, i.e. discard chars if port buff is full.) */
// int bsp_uart_init_number(u32 iuart, u32 baud, u32 txbuffsize, u32 rxbuffsize,  u32 uart_int_priority	
//	u8 block, u8 nstop);
//	bsp_uart_int_init_number(USTDO, 460800,  256,  256, 0x30, 1, 0);
//	bsp_uart_int_init_number(USTDO, 230400, 4096, 1024, 0x40, 1, 0);
//	bsp_uart_int_init_number(USTDO, 921600, 4092, 1024, 0x40, 1, 0);
//	bsp_uart_int_init_number(USTDO, 115200,  256,  256, 0x10, 1, 0);

	bsp_uart_int_init_number(UXPRT, 115200,  128,  512, 0x30, 0, 0);

/* Setup STDOUT, STDIN (a shameful sequence until we sort out 'newlib' and 'fopen'.)  The following 'open' sets up 
   the USART/UART that will be used as STDOUT_FILENO, and STDIN_FILENO.  Don't call 'open' again!  */
	fd = open("tty2", 0,0); // This sets up the uart control block pointer versus file descriptor ('fd')
//	fd = open("tty6", 0,0); // This sets up the uart control block pointer versus file descriptor ('fd')
/* ---------------------- DTW sys counter -------------------------------------------------------- */
	/* Use DTW_CYCCNT counter for startup timing */
	DTW_counter_init();
/* ---------------------- Let the hapless Op know it is alive ------------------------------------ */
	int i;
	/* Do this several times because it takes the PC a while to recognize and start 'ttyACM0' and some of
           the chars are missed.  No such problem with ttyUSBx, however. */
	for (i = 0; i < 1; i++) 
	{
		/* Announce who we are. ('xprintf' uses uart number to deliver the output.) */
		xprintf(UXPRT,  " \n\rF4 DISCOVERY GATEWAY FTDI w LINKED LIST CAN1 & CAN2 DRIVER: 11-21-2016  hw fp v0\n\r");
		/* Make sure we have the correct bus frequencies */
		xprintf (UXPRT, "   hclk_freq (MHz) : %9u...............................\n\r",  hclk_freq/1000000);	
		xprintf (UXPRT, "  pclk1_freq (MHz) : %9u...............................\n\r", pclk1_freq/1000000);	
		xprintf (UXPRT, "  pclk2_freq (MHz) : %9u...............................\n\r", pclk2_freq/1000000);	
		xprintf (UXPRT, " sysclk_freq (MHz) : %9u...............................\n\r",sysclk_freq/1000000);
	}

/* Unique device ID--12 bytes */
unsigned int uid[3];
uid[0] = *(unsigned int *)0x1fff7a10;
uid[1] = *(unsigned int *)0x1fff7a14;
uid[2] = *(unsigned int *)0x1fff7a18;
xprintf(UXPRT,"Unique device ID: 0x%08X 0x%08X 0x%08X\n\r",uid[0],uid[1],uid[2]);

/* Byte-by-byte unique device ID display */
xprintf(UXPRT,"Unique device ID: 0x%08X ",uid[0]);
unsigned char* puid = (unsigned char *)0x1fff7a14;
for (i = 0; i < 8; i++)
{
	if (*puid < (unsigned char)0x20)
		xprintf(UXPRT,"0x%02X ", *puid++);	
	else
		xprintf(UXPRT,"%c ", *puid++);
}
xprintf (UXPRT,"\n\r");

xprintf (UXPRT,"Flash size: %dK\n\r",(*(unsigned int*)0x1FFF7A22)&0xffff);


/* Testing of floating pt. */
volatile int idelay = 90000;
while (idelay-- > 0);

volatile float a = .14159265;
volatile float x;
volatile unsigned int f0 = DTWTIME;
x = a * 12.7 + 10.1;
volatile unsigned int f1 = DTWTIME;
//int ix = x * 1000000;
//xprintf (UXPRT, "fp test int: %d",ix);
xprintf (UXPRT, "fp test: %10.6f",(double)x);
volatile unsigned int f2 = DTWTIME;
xprintf (UXPRT, "  dur0 (tick): %d  dur1 (usec): %d\n\r",(f1-f0),(f2-f1)/168);

idelay = 90000;
while (idelay-- > 0);

f0 = DTWTIME;
x = atanf(a);
f1 = DTWTIME;
xprintf (UXPRT, "atanf test: %10.6f",(double)x);
f2 = DTWTIME;
xprintf (UXPRT, "  dur0 (tick): %d  dur1 (usec): %d\n\r",(f1-f0),(f2-f1)/168);

f0 = DTWTIME;
volatile double aa = .1415926535897932;
double xx = atan(aa);
f1 = DTWTIME;
xprintf (UXPRT, "atan  test: %10.6f",xx);
f2 = DTWTIME;
xprintf (UXPRT, "  dur0 (tick): %d  dur1 (usec): %d\n\r",(f1-f0),(f2-f1)/168);

f0 = DTWTIME;
volatile long double aaa = .1415926535897932;
long double xxx = atanl(aaa);
f1 = DTWTIME;
xprintf (UXPRT, "atanl test: %10.6f",xxx);
f2 = DTWTIME;
xprintf (UXPRT, "  dur0 (tick): %d  dur1 (usec): %d\n\r",(f1-f0),(f2-f1)/168);

/* --------------------- CAN setup ------------------------------------------------------------------- */
	/*  Pin usage for CAN1 on DiscoveryF4--
	PD01 CAN1  Tx LQFP 82 Header P2|33 WHT
	PD00 CAN1  Rx LQFP 81 Header P2|36 BLU
	GRN  (MCP2551 RS pin) Grounded for high speed
	*/
#ifdef SETUP_CAN1
	/* Setup CAN registers and initialize routine */
	pctl1 =  canwinch_setup_F4_discovery(&msginit1, 1);	// Number msg bufferblocks, CAN1

	/* Check if initialization was successful. */
	if (pctl1 == NULL)
	{
		xprintf(UXPRT,"CAN1 init failed: NULL ptr\n\r");
		panic_leds(6); while (1==1);	// Flash panic display with code 6
	}
	if (pctl1->ret < 0)
	{
		xprintf(UXPRT,"CAN1 init failed: return code = %d\n\r",pctl1->ret);
		panic_leds(6); while (1==1);	// Flash panic display with code 6
	}
	xprintf(UXPRT,"CAN1 initialized OK@ baudrate %d\n\r",CANWINCH_BAUDRATE);
#endif
#ifdef SETUP_CAN2
	/*  Pin usage for CAN1 on DiscoveryF4--
	PB13 CAN2  Tx LQFP 82 Header P2|33 WHT
	PB12 CAN2  Rx LQFP 81 Header P2|36 BLU
	GRN  (MCP2551 RS pin) Grounded for high speed
	*/
	pctl2 =  canwinch_setup_F4_discovery(&msginit1, 2);	// Number msg bufferblocks, CAN2

	/* Check if initialization was successful. */
	if (pctl2 == NULL)
	{
		xprintf(UXPRT,"CAN2 init failed: NULL ptr\n\r");
		panic_leds(6); while (1==1);	// Flash panic display with code 6
	}
	if (pctl2->ret < 0)
	{
		xprintf(UXPRT,"CAN2 init failed: return code = %d\n\r",pctl2->ret);
		panic_leds(6); while (1==1);	// Flash panic display with code 6
	}
	xprintf(UXPRT,"CAN2 initialized OK @ baudrate %d\n\r",CANWINCH_BAUDRATE_CAN2);
#endif
	can_driver_enable_interrupts();	// Enable CAN interrupts
	xprintf(UXPRT,"CAN interrupts enabled\n\r");
/* --------------------- Initialize the time for the test msg generation ----------------------------- */
	CAN_test_msg_init();

/* --------------------- Hardware is ready, so do program-specific startup ---------------------------- */
#define FLASHCOUNT 21000000;	// LED flash
u32	t_led = DTWTIME + FLASHCOUNT; // Set initial time

	PC_msg_initg(&pctogateway);	// Initialize struct for CAN message from PC
	PC_msg_initg(&pctogateway1);	// Initialize struct for CAN message from PC

	/* Set modes for routines that receive and send CAN msgs */
	pctogateway.mode_link = MODE_LINK;
	pctogateway1.mode_link = MODE_LINK;

/* Test disable/enable global interrupts */
__asm__ volatile ("CPSID I");

__asm__ volatile ("CPSIE I");

t_cmdn = DTWTIME + 168000000; // Set initial time
u32 hb_inc = DELAYHEARTBEAT * (sysclk_freq/64);
t_hb = DTWTIME + hb_inc;

/* --------------------- Endless Polling Loop ----------------------------------------------- */
	while (1==1)
	{
		/* Flash the LED's to amuse the hapless Op or signal the wizard programmer that the loop is running. */
		if (((int)(DTWTIME - t_led)) > 0) // Has the time expired?
		{ // Here, yes.
			toggle_4leds(); 	// Advance some LED pattern
			t_led += FLASHCOUNT; 	// Set next toggle time
		}
		/* ================ PC --> CAN ================================================================= */
		temp=USB_PC_get_msg_mode(STDIN_FILENO, &pctogateway1, &canrcvbuf);	// Check if msg is ready
		if (temp != 0)	// Do we have completion of a msg?
		{ // Here, yes.  We have a msg, but it might not be valid.
			if ( temp == 1 ) // Was valid?
			{ // Here, yes.
				tmp = temp >> 16; // Isolate compression error codes
				if (tmp < 0)	// Did the compression detect some anomolies?
				{ // Here, something wrong with the msg--
xprintf(UXPRT,"TMP err: %d %u\n\r", tmp, yctr++);
				}
				else
				{ // Here, msg is OK msg from the PC

				/*
				   This would be a good place to put a check for stopping msgs with certain CAN id's from
				   being sent on the CAN bus, or  and any other criteria to prevent putting the msg out on 
				   the CAN bus.  Note 'CAN_gateway_send' also has some checks for illegal CAN msgs. 
				*/

					/* Place the msg in the buffer where it will be xmitted. */
#ifdef SETUP_CAN1
					tmp = CAN_gateway_send(pctl1, &canrcvbuf);	// Add to xmit buffer (if OK)
					Errors_CAN_gateway_send(tmp);		// Count any error returns
#endif					
//xprintf(UXPRT,"OK: %u: %08x %u %u %u\n\r",xctr++,canrcvbuf.id,canrcvbuf.cd.ui[0], yctr,zctr);
#ifdef SETUP_CAN2
					tmp = CAN_gateway_send(pctl2, &canrcvbuf);	// Add to xmit buffer (if OK)
					Errors_CAN_gateway_send(tmp);		// Count any error returns					
#endif
					PC_msg_initg(&pctogateway1);	// Initialize struct for next msg from PC to gateway
				}
			}
			else
			{ // Something wrong with the msg.  Count the various types of error returns from 'USB_PC_msg_getASCII'
				Errors_USB_PC_get_msg_mode(temp);
xprintf(UXPRT,"USB_PC_msg_getASCII err: %d %u\n\r", temp, zctr++);
			} // Note: 'pctogateway1' gets re-intialized in 'PC_msg_initg' when there are errors.
		}

		/* Periodically send a test msg out on the CAN bus (see 'CAN_test_msgs.c') */
//		if ( (ptest_can = CAN_test_msg_CAN()) != 0) // Do we have a fixed test msg to send out on the CAN bus?
//		{ // Here, one is ready and 'ptest' points to the msg
//			tmp = CAN_gateway_send(pctl1, ptest_can);	// Add to xmit buffer (if msg is valid)
//			Errors_CAN_gateway_send(tmp);		// Count any error returns
//		}

		/* Send heart-beat periodically. */
		if (((int)(DTWTIME - t_hb)) > 0) // Has the time expired?
		{ // Here, yes.
			t_hb += hb_inc; 			// Set next toggle time
#ifdef SETUP_CAN1
			tmp = CAN_gateway_send(pctl1, &can_hb);	// Add to xmit buffer (if OK)
			Errors_CAN_gateway_send(tmp);		// Count any error returns
#endif					
#ifdef SETUP_CAN2
			tmp = CAN_gateway_send(pctl2, &can_hb);	// Add to xmit buffer (if OK)
			Errors_CAN_gateway_send(tmp);		// Count any error returns					
#endif
			canbuf_add(&can_hb);			// Add to PC buffer (if msg is valid)
		}
		/* ================= CAN1 --> PC ================================================================= */
#ifdef SETUP_CAN1
		while ( (pc1r1 = can_driver_peek1(pctl1)) != 0)	// Did we receive a HIGH PRIORITY CAN BUS msg?
		{ // Here yes.  Retrieve it from the CAN buffer and save it in our vast mainline storage buffer ;)
			canbuf_add(pc1r1);	// Add msg to buffer
//canin(pc1r1);
caninB(pc1r1);
//printmsg(pc1r1, 1);
			can_driver_toss1(pctl1); // Release buffer block, fifo1 linked list
		}

		while ( (pc1r0 = can_driver_peek0(pctl1)) != 0)		// Did we receive a LESS-THAN-HIGH-PRIORITY CAN BUS msg?
		{ // Here yes.  Retrieve it from the CAN buffer and save it in our vast mainline storage buffer
			canbuf_add(pc1r0);	// Add msg to buffer
//canin(pc1r0);
caninB(pc1r0);
//printmsg(pc1r0, 0);
			can_driver_toss0(pctl1); // Release buffer block, fifo0 linked list
		}
#endif		
		/* ================= CAN2 --> PC ================================================================= */
#ifdef SETUP_CAN2
		while ( (pc2r1 = can_driver_peek1(pctl2)) != 0)	// Did we receive a HIGH PRIORITY CAN BUS msg?
		{ // Here yes.  Retrieve it from the CAN buffer and save it in our vast mainline storage buffer ;)
			canbuf_add(pc2r1);	// Add msg to buffer
//canin(pc2r1);
caninB(pc2r1);
//printmsg(pc2r1, 1);
			can_driver_toss1(pctl2); // Release buffer block, fifo1 linked list
		}

		while ( (pc2r0 = can_driver_peek0(pctl2)) != 0)		// Did we receive a LESS-THAN-HIGH-PRIORITY CAN BUS msg?
		{ // Here yes.  Retrieve it from the CAN buffer and save it in our vast mainline storage buffer.
			canbuf_add(pc2r0);	// Add msg to buffer
//canin(pc2r0);
caninB(pc2r0);
//printmsg(pc2r0, 0);
			can_driver_toss0(pctl2); // Release buffer block, fifo0 linked list
		}

#endif	
//		/* Send test msgs to PC (and not to CAN bus) */
//		if ( (ptest_pc = CAN_test_msg_PC()) != 0)	// Test msg ready? (CAN to PC direction)
//		{ // Here yes.  See 'CAN_test_msg.c' to set msgs and rate of generation
//			tmp = CAN_gateway_send(pctl1, ptest_can);	// Add to xmit buffer (if msg is valid)
//			Errors_CAN_gateway_send(tmp);		// Count any error returns
//		}

		/* Send buffered msgs to PC */
		while (canbufidxi != canbufidxm)	// Set up all the buffered msgs until we are caught up.				
		{ // Here, yes.  Set up a buffered msg from the CAN bus to go to the PC.
			pctogateway.cmprs.seq = canmsgct[canbufidxm];		// Add sequence number (for PC checking for missing msgs)
			USB_toPC_msg_mode(STDOUT_FILENO, &pctogateway, &canbuf[canbufidxm]); 	// Send to PC via STDOUT
			canbufidxm = adv_index(canbufidxm, CANBUSBUFSIZE);	// Advance outgoing buffer index.
		}	
	}
	return 0;	
}
/* **************************************************************************************
 * static void canbuf_add(struct CANRCVBUF* p);
 * @brief	: Add msg to buffer
 * @param	: p = Pointer to CAN msg
 * ************************************************************************************** */
static void canbuf_add(struct CANRCVBUF* p)
{
	int temp;
	canbuf[canbufidxi] = *p;		// Copy struct
	canmsgct[canbufidxi] = canmsgctr;	// Save sequence count that goes with this msg
	canmsgctr += 1;				// Count incoming CAN msgs
	temp = adv_index(canbufidxi, CANBUSBUFSIZE);	// Increment the index for incoming msgs.
	if (canbufidxm == temp)  		// Did this last fill the last one?
	{ // Yes, we have filled the buffer.  This CAN msg might be dropped (by not advancing the index)
		Errors_misc(-1);		// Add to buffer overrun counter
	}
	else
	{ // Here, there is room in the buffer and we are good to go.
		canbufidxi = temp;		// Update the index to next buffer position.
	}	
	return;
}
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
 * void caninB(struct CANRCVBUF* p);
 * @brief	: make count of CAN ids between "fiducial" msg
 * @param	: p = Pointer to CAN msg
  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
static u32 idctr = 0;
static u32 throttle = 0;
void caninB(struct CANRCVBUF* p)
{
	idctr += 1;
	if (p->id == 0x00200000)
	{
		throttle += 1;
		if (throttle >= 64)
		{
			xprintf (UXPRT, " %d\n\r",idctr);
			idctr = 0;
			throttle = 0;
		}
	}
	return;
}
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
 * void canin(struct CANRCVBUF* p);
 * @brief	: make list of different CAN ids and counts
 * @param	: p = Pointer to CAN msg
  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

#define CANINSIZE	130
struct CANIN
{
	u32	canid;
	u32	ct;
}canlst[CANINSIZE];
struct CANIN* ptmp;
static unsigned int tik = 0;



void canin(struct CANRCVBUF* p)
{
	int total;

	/* Check if this ID is in the list. */
	for (ptmp = &canlst[0]; ptmp->canid != 0; ptmp++)
	{
		if (ptmp->canid == p->id)
		{ // Here, we found it
			ptmp->ct += 1;


		if (((int)(DTWTIME - t_cmdn)) > 0) // Has the time expired?
//			if (ptmp->canid == 0x00200000)
			{ // Here, 1/64th sec tick from GPS
t_cmdn = DTWTIME + 168000000; 
//				if ((tik & 63) == 0) // 64 time ticks per second
				{ // Here, print one second summary line
					total = 0; // Total for this interval
					/* Print list        */
					for (ptmp = &canlst[0]; ptmp->canid != 0; ptmp++)
					{
						if ((ptmp->canid & ~0xffe00000) == 0)
						{ // Compress 11 b CAN id 
							xprintf (UXPRT,"%03x %d|",(ptmp->canid >> 20),ptmp->ct);
						}
						else
						{ // Full CAN id
							xprintf (UXPRT,"%08x %d|", ptmp->canid,ptmp->ct);
						}
						total += ptmp->ct;	// Sum for total
						ptmp->ct = 0;	// Reset count for next interval
					}
					xprintf (UXPRT, " %d\n\r",total);
					if ((tik & 4095) == 0) // Reset list
					{
						canlst[0].canid = 0;
						xprintf (UXPRT,"----\n\r");
					}
				}
				tik += 1;
			}
			return;
		}
	}
	/* Here, the search failed. Add ID to list. */
//xprintf (UXPRT, "%08X\n\r",p->id);
	if (ptmp >= &canlst[CANINSIZE-1]) ptmp = &canlst[CANINSIZE-2];
	ptmp->canid = p->id;
	ptmp->ct = 1;
	ptmp++;
	ptmp->canid = 0;
	return;
}

