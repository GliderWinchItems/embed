/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : co1.c
* Hackeroo           : deh
* Date First Issued  : 01/22/2013
* Board              : STM32F103VxT6_pod_mm
* Description        : Prototype a sensor ### OLIMEX ###
*******************************************************************************/
/* 

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
#include "libmiscstm32/systick1.h"
#include "libmiscstm32/printf.h"
#include "libmiscstm32/clockspecifysetup.h"

#include "adcsensor_pod.h"
#include "canwinch_pod.h"
#include "sensor_threshold.h"
#include "common_can.h"
#include "scb.h"

#include "dma17_fill.h"

#include <malloc.h>

#define IAMUNITNUMBER	CAN_UNITID_OLICAW	// Each node on the CAN bus gets a unit number

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
1		// nart: CAN_MCR[4] No Automatic ReTry (0 = retry; non-zero = auto retry)
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
	SCB_AIRCR  |= (0x5FA << 16) | SCB_AIRCR_SYSRESETREQ;	// Cause a RESET
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
And now for the main routine 
  #################################################################################################*/
int main(void)
{
	s32 i = 0; 		// Timing loop variable
	char* p;		// Temp getline pointer
	s32 can_ret = -4;

/* --------------------- Begin setting things up -------------------------------------------------- */ 

	clockspecifysetup((struct CLOCKS*)&clocks);		// Get the system clock and bus clocks running

/* ---------------------- Set up pins ------------------------------------------------------------- */
	gpio_setup();		// Need this to make the LED work

//[enable GPS power]

	init_printf(0,putc);	// This one-time initialization is needed by the tiny printf routine

	SYSTICK_init(0);	// Set SYSTICK for interrupting and to max count (24 bit counter)

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
	USART2_rxinttxint_init(115200,32,2,96,4); // Initialize USART and setup control blocks and pointers
//	USART2_rxinttxint_init(  9600,32,2,32,3); // Initialize USART and setup control blocks and pointers

	/* Announce who we are */
	USART2_txint_puts("\n\rco1 OLIMEX 01-08-2013 22:42\n\r");
	USART2_txint_send();	// Start the line buffer sending

	/* Display things for to entertain the hapless op */
	printf ("  hclk_freq (MHz) : %9u\n\r",  hclk_freq/1000000);	USART2_txint_send();
	printf (" pclk1_freq (MHz) : %9u\n\r", pclk1_freq/1000000);	USART2_txint_send();
	printf (" pclk2_freq (MHz) : %9u\n\r", pclk2_freq/1000000);	USART2_txint_send();
	printf ("sysclk_freq (MHz) : %9u\n\r",sysclk_freq/1000000);	USART2_txint_send();

	USART2_txint_puts("'\n\r x>enter>' will reset counters\n\r");
	USART2_txint_send();	// Start the line buffer sending

/* --------------------- CAN setup ------------------------------------------------------------------- */
	/* Setup CAN registers and initialize routine */
	can_ret = can_init_pod((struct CAN_PARAMS*)&can_params); // 'struct' hold all the parameters


	/* Check if initialization was successful, or timed out. */
	if (can_ret <= 0)
	{
		printf("can init failed: code = %d\n\r",can_ret);USART2_txint_send(); 
		while (1==1);
	}
	printf("\n\rcan ret ct: %d\n\r",can_ret);USART2_txint_send(); // Look at how long "exit initialization" took

	/* Set filters to respond "this" unit number and time sync broadcasts */
	can_filter_unitid(IAMUNITNUMBER);	// Setup msg filter banks

	printf ("IAMUNITNUMBER %0x %0x\n\r",IAMUNITNUMBER,(u32)IAMUNITNUMBER >> CAN_UNITID_SHIFT);USART2_txint_send();

	/* Program checksum check and prog reload */
//	prog_checksum_loader();	// Return means we got the "Go ahead" command

/* --------------------- Program is ready, so do program-specific startup ---------------------------- */

	dma17_fill_init();	// Setup DMA17 



#include "packet_mgr.h"
//struct CANRCVTIMBUF  dcan;
//packet_mgr_add(&dcan);





#define TICKS	72000000
#include "gps_1pps_se.h"
	Tim2_pod_init();
u32 ref2;
ref2 = Tim2_gettime_ui() + TICKS;
u32 temp = SYSTICK_getcount32();

u32 temp4;
s32 temp3;
//s32 acc = 0;
i = 0;

/* CAN tx test */
struct CANRCVBUF can_msg;
can_msg.id = IAMUNITNUMBER;
//can_msg.id = CAN_UNITID_CO1;	// POD UNITID
//can_msg.cd.ui[0] = 0x76543210;	// 1st 4 bytes of msg
//can_msg.cd.ui[1] = 0xfedcba98;		// 2nd 4 bytes: if zero, then "n" in msg is only 4 bytes
// can_msg.cd.ull = 0xfedcba9876543210;
   can_msg.cd.ull = 0x000aa0012340000;

/* CAN test high priority msg */
struct CANRCVTIMBUF can_msg1;	// NOTE: This one gets a linux time field
can_msg1.R.id = CAN_TIMESYNC1;	// ID test of high priority msg
can_msg1.R.dlc = 0; 	// initialize unused bit fields
can_msg1.R.cd.ull = 0xee0000000000cc00;



/* CAN rx test */
struct CANRCVTIMBUF * pFifo1 = 0;	// FIFO 1 buffer receive msg pointer
struct CANRCVBUF * pFifo0 = 0;		// FIFO 2 buffer receive msg pointer

s32 can_put = -1;
s32 j;
s32 k = 0;
/* --------------------- Endless Stuff ----------------------------------------------- */
	while (1==1)
	{
		/* Check on incoming data */
		pFifo1 = canrcvtim_get();
		if (pFifo1 != 0)
		{ // Here, we got something!
			can_msg_rcv_expand(&pFifo1->R);	// Expand a compressed msg
			printf("FIFO1: %08x %08x %08xL %08xH\n\r",pFifo1->R.id, pFifo1->R.dlc, pFifo1->R.cd.ui[0],pFifo1->R.cd.ui[1]);  USART2_txint_send();
			can_msg1.R.cd.ull += 1;
		}
		pFifo0 = canrcv_get();
		if (pFifo0 != 0)
		{ // Here, we got something!
			can_msg_rcv_expand(pFifo0);	// Expand a compressed msg
			printf("FIFO0: %08x %08x %08xL %08xH\n\r",pFifo0->id, pFifo0->dlc, pFifo0->cd.ui[0],pFifo0->cd.ui[1]); USART2_txint_send();

			/* Send received msg back to POD */
//			pFifo0->id = CAN_UNITID_CO1 | (0xa << (32-11)); // Add something to the not-masked ID
//			can_msg_rcv_compress (pFifo0);	// Set byte count according to MSB
//			can_put = can_msg_put(pFifo0);	// Setup CAN msg in output buffer/queue

			/* Send a high priority message */
//			can_msg_rcv_compress(&can_msg1.R);	// Set byte count according to MSB
//			can_put = can_msg_put(&can_msg1.R);// Setup CAN msg in output buffer/queue
		}
		
		/* printf rate throttling */
		if ( i++ > 10000)
//		while ( ((s32)(SYSTICK_getcount32() - temp)) > 0  );
//		if  ((s32)(Tim2_gettime_ui() - ref2) > 0)
		{
			i = 0;
			ref2 += TICKS;
			j = 0;
			while (j++ < 1)	// Test a "burst" of msgs added to buffer
			 {
//				can_msg_setsize(&can_msg, 8);	// Fixed xmt byte count setup
				can_msg_rcv_compress(&can_msg);	// Set byte count according to MSB
				can_put = can_msg_put(&can_msg);// Setup CAN msg in output buffer/queue
				can_msg.cd.ui[0] += 1;		// Make the msg change for the next round
        	         }

			toggle_led();
			temp3 = (s32)(temp - temp4);
extern u32 can_debugP;
extern u32 can_debugM;
extern u32 can_terr;

/* p 656 CAN_ESR register
Bits 31:24 REC[7:0]: Receive error counter
            The implementing part of the fault confinement mechanism of the CAN protocol. In case of
            an error during reception, this counter is incremented by 1 or by 8 depending on the error
            condition as defined by the CAN standard. After every successful reception the counter is
            decremented by 1 or reset to 120 if its value was higher than 128. When the counter value
            exceeds 127, the CAN controller enters the error passive state.
Bits 6:4 LEC[2:0]: Last error code
           This field is set by hardware and holds a code which indicates the error condition of the last
           error detected on the CAN bus. If a message has been transferred (reception or
           transmission) without error, this field will be cleared to ‘0’.
           The LEC[2:0] bits can be set to value 0b111 by software. They are updated by hardware to
           indicate the current communication status.
           000: No Error
           001: Stuff Error
           010: Form Error
           011: Acknowledgment Error
           100: Bit recessive Error
           101: Bit dominant Error
           110: CRC Error
           111: Set by software
*/
u32 can_esr = CAN_ESR(CAN1);
u32 can_tec = (can_esr >> 16) & 0xff;	// Tx error counter
u32 can_lec = (can_esr >>  4) &  0x3;	// Tx last error code

/* p 650 CAN_MSR
Bit 11 RX: CAN Rx signal
        Monitors the actual value of the CAN_RX Pin.
*/
u32 can_msr = CAN_MSR(CAN1);
u32 can_rcvbit = (can_msr >> 11) & 0x1;


			printf("%5u%6u%6u%6u%6u%5u%4u%3u\n\r",k++,can_put, can_debugP,can_debugM,can_terr,can_tec, can_lec, can_rcvbit ) ;  USART2_txint_send();
//			printf("%0x %0x %0x\n\r",IAMUNITNUMBER,CAN_UNITID_SHIFT,CAN_UNITID_MASK); USART2_txint_send();
		}

		/* Echo incoming chars back to the sender */
		if ((p=USART2_rxint_getline()) != 0)	// Check if we have a completed line
		{ // Here, pointer points to line buffer with a zero terminated line
			switch (*p)			
			{
			case 'x': // Reset counters
				USART2_txint_puts("RESET COUNTERS\n\r");USART2_txint_send();
				break;
			case 'h': // Histogram
				while ((p=USART2_rxint_getline()) == 0);	// Pause until a key is hit

				break;
			}
		}
	}
	return 0;	
}

