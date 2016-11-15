/******************************************************************************
* File Name          : pod6.c
* Date First Issued  : 06/02/2015
* Board              : STM32F103VxT6_pod_mm
* Description        : Use pod board to prototype two AD7799 w CAN for winch
*******************************************************************************/
/* 
02/01/2013 Rev 115 Changes to add Tim4_pod_common.c for time syncing.

Open minicom on the PC with 115200 baud and 8N1.

*/
#include <math.h>
#include <string.h>
#include <malloc.h>
extern int errno;

#include "PODpinconfig.h"
#include "libopenstm32/adc.h"
#include "libopenstm32/can.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/gpio.h"
#include "libopenstm32/usart.h"
#include "libusartstm32/usartallproto.h"
#include "libmiscstm32/printf.h"
#include "libmiscstm32/clockspecifysetup.h"
#include "libmiscstm32/DTW_counter.h"

#include "adcsensor_pod.h"
#include "../../../sw_f103/trunk/lib/libsensormisc/canwinch_setup_F103_pod.h"
#include "sensor_threshold.h"
#include "../../../../svn_common/trunk/common_can.h"
#include "../../../../svn_common/trunk/db/can_db.h"
#include "scb.h"
#include "pod6.h"
#include "tick_pod6.h"
#include "cmd_n_F103.h"

#include "canwinch_pod_common_systick2048_printerr.h"
#include "../../../../svn_discoveryf4/sw_discoveryf4/trunk/lib/libopencm3/cm3/common.h"
#include "can_driver.h"
#include "can_driver_filter.h"
#include "can_msg_reset.h"
#include "can_gps_phasing.h"

#include "db/gen_db.h"

static void printmsg(char* pc, struct CANRCVBUF* pcan);

/* Easy way for other routines to access via 'extern'*/
struct CAN_CTLBLOCK* pctl1;

/* Specify msg buffer and max useage for TX, RX0, and RX1. */
const struct CAN_INIT msginit = { \
180,	/* Total number of msg blocks. */
140,	/* TX can use this huge ammount. */
64,	/* RX0 can use this many. */
8	/* RX1 can use this piddling amount. */
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

/*******************************************************************************
 * void can_nxp_setRS_sys(int rs, int board);
 * @brief 	: Set RS input to NXP CAN driver (TJA1051) (on some PODs) (SYSTICK version)
 * @param	: rs: 0 = NORMAL mode; not-zero = SILENT mode 
 * @param	: board: 0 = POD, 1 = sensor RxT6 board
 * @return	: Nothing for now.
*******************************************************************************/
void can_nxp_setRS_sys(int rs, int board)
{
	/* RS (S) control PB7 (on sensor board) PD11 on pod board */
	// Floating input = resistor controls slope
	// Pin HI = standby;
	// Pin LO = high speed;
	if (board == 0)
	{
		configure_pin ((volatile u32 *)GPIOD, 11);	// configured for push-pull output
		if (rs == 0)
			GPIO_BRR(GPIOD)  = (1<<11);	// Set bit LO for SILENT mode
		else
			GPIO_BSRR(GPIOD) = (1<<11);	// Set bit HI for NORMAL mode
	}
	else
	{
		configure_pin ((volatile u32 *)GPIOB,  7);	// configured for push-pull output	
		if (rs == 0)
			GPIO_BRR(GPIOB)  = (1<< 7);	// Set bit LO for SILENT mode
		else
			GPIO_BSRR(GPIOB) = (1<< 7);	// Set bit HI for NORMAL mode
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

/* $$$$$$$$$$$$ Relocate the interrupt vectors from the loader to this program $$$$$$$$$$$$$$$$$$$$ */
extern void relocate_vector(void);
	relocate_vector();
/* --------------------- Begin setting things up -------------------------------------------------- */ 
	// Start system clocks using parameters matching CAN setup parameters for POD board
	clockspecifysetup(canwinch_setup_F103_pod_clocks() );
/* ---------------------- Set up pins ------------------------------------------------------------- */
	PODgpiopins_default();	// Set gpio port register bits for low power
	PODgpiopins_Config();	// Now, configure pins
	MAX3232SW_on;		// Turn on RS-232 level converter (if doesn't work--no RS-232 chars seen)
	ANALOGREG_on;		// Turn on 3.2v analog regulator
	SDCARDREG_on;		// Turn on 3.3v regulator to SD card, GPS header, and CAN driver (trace cut)
	ENCODERGPSPWR_on;	// Turn on 5.0v encoder regulator hacked to CAN driver

	/* Use DTW_CYCCNT counter for startup timing */
	DTW_counter_init();

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
	USART1_txint_puts("\n\rPOD6R 06-29-2015\n\r");
	USART1_txint_send();	// Start the line buffer sending

	/* Display things for to entertain the hapless op */
	printf ("  hclk_freq (MHz) : %9u\n\r",  hclk_freq/1000000);	USART1_txint_send();
	printf (" pclk1_freq (MHz) : %9u\n\r", pclk1_freq/1000000);	USART1_txint_send();
	printf (" pclk2_freq (MHz) : %9u\n\r", pclk2_freq/1000000);	USART1_txint_send();
	printf ("sysclk_freq (MHz) : %9u\n\r",sysclk_freq/1000000);	USART1_txint_send();
/* --------------------- ADC setup and initialization ----------------------------------------------- */
	adc_init_sequence_se();	// Time delay + calibration, then start conversion
/* --------------------- Get Loader CAN ID -------------------------------------------------------------- */
	/* Pick up the unique CAN ID stored when the loader was flashed. */
	u32 canid_ldr = *(u32*)((u32)((u8*)*(u32*)0x08000004 + 7 + 0));	// First table entry = can id
	printf(  "CANID_LDR  : 0x%08X\n\r", canid_ldr );	USART1_txint_send();
/* --------------------- CAN setup ------------------------------------------------------------------- */
	/* Configure and set MCP 2551 driver: RS pin (PD 11) on POD board */
	can_nxp_setRS_sys(0,0); // (1st arg) 0 = high speed mode; 1 = standby mode (Sets yellow led on)
	GPIO_BSRR(GPIOE) = (0xf<<LED3);	// Set bits = all four LEDs off

	/* Initialize CAN for POD board (F103) and get control block */
	// Set hardware filters for FIFO1 high priority ID & mask, plus FIFO1 ID for this UNIT
	pctl1 = canwinch_setup_F103_pod(&msginit, canid_ldr); // ('can_ldr' is fifo1 reset CAN ID)

	/* Check if initialization was successful. */
	if (pctl1 == NULL)
	{
		printf("CAN1 init failed: NULL ptr\n\r");USART1_txint_send(); 
		while (1==1);
	}
	if (pctl1->ret < 0)
	{
		printf("CAN init failed: return code = %d\n\r",pctl1->ret);USART1_txint_send(); 
		while (1==1);
	}

	/* Enable sending of CAN msg every 1/64th sec, in sync with CO time sync */
	int ret = tick_pod6_init();	
	if (ret < 0)
	{
		printf("tick6_pod.c failed: return code (NULL pctl1) = %d\n\r",pctl1->ret);USART1_txint_send(); 
		while (1==1);
	}
/* ------------------------ Setup CAN hardware filters ----------------------------------------------- */
	can_msg_reset_init(pctl1, canid_ldr);	// Specify CAN ID for this unit for msg caused RESET
	
	// Add CAN IDs this function will need *from* the CAN bus.  Hardware filter rejects all others. */
	//                                      ID #1                    ID#2         FIFO  BANK NUMBER
	ret  = can_driver_filter_add_two_32b_id(CANID_CMD_TENSION_1,     CANID_TIME_MSG, 0, 2);
	ret |= can_driver_filter_add_two_32b_id(CANID_CMD_CABLE_ANGLE_1, CANID_DUMMY,    0, 3);
	if (ret != 0)
	{
		printf("filter additions: failed init\n\r");USART1_txint_send(); 
		while (1==1);		
	}	
/* ------------------------ SYSTICK counter ---------------------------------------------------------- */
	can_gps_phasing_init(pctl1);
/* ---------------- When CAN interrupts are enabled reception of msgs begins! ------------------------ */
	can_driver_enable_interrupts();	// Enable CAN interrupts
/* ------------------------ Command 'n' type printout ------------------------------------------------ */
	#define CMDNLISTSIZE	32
	ret = cmd_n_init_F103(CMDNLISTSIZE); // "Command n" listing: get buffer
	if (ret != 0)
	{
		printf("cmd_n_F103_init: failed init\n\r");USART1_txint_send(); 
		while (1==1);		
	}
	printf("cmd_n_F103_init: success list size: %d\n\r",CMDNLISTSIZE);USART1_txint_send(); 

	printf("sizeof (struct CAN_POOLBLOCK) %d\n\r",sizeof (struct CAN_POOLBLOCK));	USART1_txint_send(); 
/* --------------------- Program is ready, so do program-specific startup ---------------------------- */
int i = 0;

/* CAN tx test */
//struct CANRCVBUF can_msg;
//can_msg.id = IAMUNITNUMBER | 0x05000000;
//can_msg.id = CAN_UNITID_POD6;	// PD board hack sensor ID
//can_msg.cd.ui[0] = 0x76543210;	// 1st 4 bytes of msg
//can_msg.cd.ui[1] = 0xfedcba98;		// 2nd 4 bytes: if zero, then "n" in msg is only 4 bytes
// can_msg.cd.ull = 0xfedcba9876543210;
//   can_msg.cd.ull = 0x00000000abcd0000;

/* CAN rx test */
struct CANRCVBUF * pFifo1 = 0;	// FIFO 1 buffer receive msg pointer
struct CANRCVBUF * pFifo0 = 0;	// FIFO 0 buffer receive msg pointer

//int can_put = -1;
//int j;
//int k = 0;
//int m = 0;

//extern u32 CAN_ticks;
//extern s32 CAN_dev;
//extern s32 CAN_ave;
//extern u32 CAN_dbg1;
//extern u32 CAN_dbg2;
//extern s32 CAN_dif;
//extern u32	stk_val;
extern u8	LEDflg;
//extern int z1,z2;
extern u32 dbgStk;
extern int dbgTz;
extern int dbgTz_max;
extern int dbgTz_min;
extern int qw;

extern u32 debugA;
extern u32 debugB;
extern u32 debugC;
extern u32 debugD;
u32 debugA_prev = debugA;
u32 debugB_prev = debugB;
u32 debugC_prev = debugC;
u32 debugD_prev = debugD;
extern volatile int debugT2;
int msgtot;
int msgtotsum;
struct CAN_BLOCK_CTS mcts = {0,0,0,0,0,0};

//struct CANWINCHPODCOMMONERRORS* pcanerrors = &pctl1->can_errors;

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

//			printf("%4d %4d %4d \n\r",dbgTz, dbgTz_max, dbgTz_min);		USART1_txint_send();
			canwinch_pod_common_systick2048_printerr(&pctl1->can_errors);
printf("%d %d %d %d %d\n\r",debugA-debugA_prev, debugB-debugB_prev, debugC-debugC_prev, debugD-debugD_prev, debugT2);USART1_txint_send(); 
debugA_prev = debugA;debugB_prev = debugB;debugC_prev = debugC;debugD_prev = debugD;			

//			printf("%d %d %d ",z1,z2,z1+z2);
//			can_driver_printerr(pcanerrors);USART1_txint_send();
		}
		/* Check on incoming data */
		pFifo1 = can_driver_peek1(pctl1);	// Check for FIFO 1 msgs
		if (pFifo1 != NULL)
		{ // Here, we got something!
			i += 1;
			cmd_n_F103(pFifo1);
			printmsg("F1: ",pFifo1);
msgtot = can_driver_getcount(pctl1, NULL, &mcts);
msgtotsum = mcts.catx + mcts.carx0 + mcts.carx1 + mcts.cbtx + mcts.cbrx0 + mcts.cbrx1; // Tot individual cts
//printf("%d = %d + %d = %d %d %d : %d %d %d\n\r",msgtot+msgtotsum,msgtot,msgtotsum,mcts.catx,mcts.carx0,mcts.carx1,mcts.cbtx,mcts.cbrx0,mcts.cbrx1);

			can_driver_toss1(pctl1);	// Release buffer block
		}
		pFifo0 = can_driver_peek0(pctl1);	// Check for FIFO 0 msgs
		if (pFifo0 != NULL)
		{ // Here, we got something!
			cmd_n_F103(pFifo0);
			printmsg("F0: ",pFifo0);
			can_driver_toss0(pctl1);	// Release buffer block
		}
	}
	return 0;	
}
/* ***********************************************************************************************
 * static void printmsg(char* pc, struct CANRCVBUF* pcan);
 *************************************************************************************************/
static void printmsg(char* pc, struct CANRCVBUF* pcan)
{
	unsigned int i;
	printf("%s %08X %x ",pc,pcan->id, pcan->dlc & 0xf);
	for (i = 0; i < (pcan->dlc & 0xf); i++)
	{
		printf("%02X ",pcan->cd.uc[i]);
	}
	printf("\n\r");USART1_txint_send();
	return;
}
