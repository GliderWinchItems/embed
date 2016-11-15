/******************************************************************************
* File Name          : co1.c
* Date First Issued  : 12/25/2012
* Board              : STM32F103VxT6_pod_mm
* Description        : Sensor board with ublox gps and flash genie SD module
*******************************************************************************/
/* 
01/31/2013 rev 114 Tim4_pod_se.c with 1PPS & OC, but not debugged 
02/09/2013 rev 119 Tim4_pod_se.c copied from ../testing/ICOCOV/trunk
05/10/2014 rev 448 (approx) changes made to make logger run on sensor board with ublox gps
   ../svn_sensor/hw/trunk/eagle/f103R/logger_mods/SD_wiring
08/04/2016 (git) begin revision for latest parameters and can driver et al.  For some reason
   the old version was not sending/receiving CAN msgs.

Note: Use PA0 to stop SD card write.  Large pushbutton near SD card--press == hi.  
Pressing stops SD card write for 'x' seconds so that card can be removed.  Use LED
flashing...

 to UEXT connector
 TX = UEXT-3
 RX = UEXT-4
 GPS 1 pps = UEXT-5 (TIM4 CH1)
Open minicom on the PC with 115200 baud and 8N1.

More...

01/21/2014 Rev 252? Last before reworking
01/25/2014 Rev 361  Adding begin debug changes for gps binary fix msgs

*/
#include <math.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>

#include "SENSORpinconfig.h"
#include "common.h"
#include "libopenstm32/can.h"
#include "libopenstm32/gpio.h"
#include "libopenstm32/usart.h"
#include "libopenstm32/scb.h"
#include "libusartstm32/usartallproto.h"
//#include "libmiscstm32/printf.h"
#include "libmiscstm32/clockspecifysetup.h"
#include "libmiscstm32/DTW_counter.h"


#include "libsensormisc/canwinch_setup_F103_pod.h"
#include "../../../../svn_common/trunk/common_highflash.h"

#include "sdlog.h"
#include "SD_socket.h"
#include "Tim9.h"
#include "ledcontrol.h"


/* ../svn_sensor/sw_f103/trunk/lib */
//#include "adcsensor_pod.h"
#include "canwinch_pod.h"
//#include "sensor_threshold.h"
#include "common_can.h"
#include "common_time.h"
#include "can_log.h"
#include "p1_gps_time_convert.h"
#include "p1_PC_handler.h"
#include "gps_poll.h"
#include "gps_1pps_se.h"
#include "can_driver.h"
#include "can_msg_reset.h"
#include "can_driver_filter.h"
#include "CAN_poll_loop.h"
#include "gps_poll.h"
#include "can_log_printf.h"
#include "gps_poll_printf.h"

/* Each node on the CAN bus gets a unit number */
#define IAMUNITNUMBER	CAN_UNITID_OLI2	// Olimex (DEH board) CO

extern unsigned int ticks_flg;

/* Easy way for other routines to access via 'extern'*/
struct CAN_CTLBLOCK* pctl0;

/* Specify msg buffer and max useage for CAN1: TX, RX0, and RX1. */
const struct CAN_INIT msginit = { \
180,	/* Total number of msg blocks. */
15,	/* TX can use this huge ammount. */
145,	/* RX0 can use this many. */
25	/* RX1 can use this piddling amount. */
};



static char vv[96];
/* **************************************************************************************
 * void system_reset(void);
 * @brief	: Software caused RESET
 * ************************************************************************************** */
static void loop(volatile int ct)
{
	*(volatile unsigned int*)0xE000EDFC |= 0x01000000; // SCB_DEMCR = 0x01000000;
	*(volatile unsigned int*)0xE0001000 |= 0x1;	// Enable DTW_CYCCNT (Data Watch cycle counter)

	volatile unsigned int tx = (*(volatile unsigned int *)0xE0001004) + ct;
	while (  ((int)(tx - (*(volatile unsigned int *)0xE0001004)))  > 0 );
	return;
}
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
	SCB_AIRCR  = (0x5FA << 16) | SCB_AIRCR_SYSRESETREQ;	// Cause a RESET
	while (1==1);
;
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
 * void system_reset_waitmsg(char*);
 * @brief	: Print msg and wait before rebooting
 * ************************************************************************************** */

void system_reset_waitmsg(char* s)
{
	LeftREDsetmode(2, 150);	// Left RED led to flash: FAST = SD NOT READY
	USART1_txint_puts(s);  USART1_txint_send();
	loop(48000000 * 3);	// Wait for things to settle down
	system_reset();	// Wait for serial port to finish, then reset
	while(1==1);
}


/* **************************************************************************************
 * void putc ( void* p, char c); // This is for the tiny printf
 * ************************************************************************************** */
// Note: the compiler will give a warning about conflicting types
// for the built in function 'putc'.  Use ' -fno-builtin-putc' to eliminate compile warning.
//void putc ( void* p, char c)
//	{
//		p=p;	// Get rid of the unused variable compiler warning
//		USART1_txint_putc(c);
//	}
/*#################################################################################################
  And now---the main routine!
  #################################################################################################*/
int main(void)
{
	u32 ticks_flg_prev = 0;
	int SD_ret;	// SD card insertion switch initial status
	int SD_rdy = 0;	// SD card not yet initialized
	int i;
	int ret;

/* $$$$$$$$$$$$ Relocate the interrupt vectors from the loader to this program $$$$$$$$$$$$$$$$$$$$ */
extern void relocate_vector(void);
	relocate_vector();
/* --------------------- Begin setting things up -------------------------------------------------- */ 

//	clockspecifysetup((struct CLOCKS*)&clocks);		// Get the system clock and bus clocks running
	clockspecifysetup(canwinch_setup_F103_pod_clocks() );
/* ---------------------- Set up pins ------------------------------------------------------------- */
	SENSORgpiopins_Config();	// Now, configure pins

	/* Use DTW_CYCCNT counter for startup timing */
	DTW_counter_init();
setbuf(stdout, NULL);
//	init_printf(0,putc);	// This one-time initialization is needed by the tiny printf routine
//	SYSTICK_init(0);	// Set SYSTICK for interrupting and to max count (24 bit counter)
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
	USART1_rxinttxint_init(115200,32,3,96,4); // Initialize USART and setup control blocks and pointers

	/* Announce who we are */
	USART1_txint_puts("\n\rco1_sensor_ublox 08-11-2016 v0\n\r");
	USART1_txint_send();	// Start the line buffer sending

	/* Display things for to entertain the hapless op */
	printf ("  hclk_freq (MHz) : %9u\n\r",  hclk_freq/1000000);	USART1_txint_send();
	printf (" pclk1_freq (MHz) : %9u\n\r", pclk1_freq/1000000);	USART1_txint_send();
	printf (" pclk2_freq (MHz) : %9u\n\r", pclk2_freq/1000000);	USART1_txint_send();
	printf ("sysclk_freq (MHz) : %9u\n\r",sysclk_freq/1000000);	USART1_txint_send();

/* ---------------- Some test, monitoring, debug info ----------------------------------------------- */
	extern void* __paramflash0a;
	struct FLASHH2* pcanidtbl = (struct FLASHH2*)&__paramflash0a;
	printf ("FLASHH2 Address: %08X\n\r",(unsigned int)pcanidtbl);
	printf ("  CAN unit code: %08X\n\r",(int)pcanidtbl->unit_code);
	printf ("  Size of table: %d\n\r",(int)pcanidtbl->size);
	printf(" # func  CANID\n\r");
	u32 ii;
	u32 jj = pcanidtbl->size;
	if (jj > 16) jj = 16;
	for (ii = 0; ii < jj; ii++)
	{
		printf("%2d %4d  %08X\n\r",(int)ii, (int)pcanidtbl->slot[ii].func, (int)pcanidtbl->slot[ii].canid);
	}
	printf("\n\r");
/* --------------------- Get Loader CAN ID -------------------------------------------------------------- */
	/* Pick up the unique CAN ID stored when the loader was flashed. */
	struct FUNC_CANID* pfunc = (struct FUNC_CANID*)&__paramflash0a;
	u32 canid_ldr = pfunc[0].func;
	printf("CANID_LDR  : 0x%08X\n\r", (unsigned int)canid_ldr );
	printf("TBL SIZE   : %d\n\r",(unsigned int)pfunc[0].canid);
	USART1_txint_send();
/* --------------------- CAN setup ---------------------------------------------------------------------- */
	/* Configure and set MCP 2551 driver: RS pin (PD 11) on POD board */
	can_nxp_setRS_sys(0,1); 

	/* Initialize CAN for POD board (F103) and get control block */
	// Set hardware filters for FIFO1 high priority ID & mask, plus FIFO1 ID for this UNIT
	pctl0 = canwinch_setup_F103_pod(&msginit, canid_ldr); // ('can_ldr' is fifo1 reset CAN ID)

	/* Check if initialization was successful. */
	if (pctl0 == NULL)
	{
		printf("CAN1 init failed: NULL ptr\n\r");USART1_txint_send(); 
		while (1==1);
	}
	if (pctl0->ret < 0)
	{ // Here, an error code was returned.
		printf("CAN init failed: return code = %d\n\r",pctl0->ret);USART1_txint_send(); 
		while (1==1);
	}
/* ------------------------ Setup CAN hardware filters ------------------------------------------------- */
	can_msg_reset_init(pctl0, pcanidtbl->unit_code);	// Specify CAN ID for this unit for msg caused RESET
ret = 0;
/* ##### THIS NEEDS TO BE REPLACED with lookup in highflash #####	
	// Add CAN IDs this function will need *from* the CAN bus.  Hardware filter rejects all others. */
	//                                      ID #1                    ID#2                 FIFO  BANK NUMBER
//$	ret  = can_driver_filter_add_two_32b_id(CANID_CMD_TENSION_a11,  CANID_MSG_TIME_POLL,   0,     2);
//	ret  = can_driver_filter_add_two_32b_id(CANID_DUMMY,  0x30400000,   0,     2);
//$	ret |= can_driver_filter_add_two_32b_id(CANID_CMD_TENSION_a21,  CANID_MSG_TIME_POLL,   0,     3);
//$	ret |= can_driver_filter_add_two_32b_id(CANID_CMD_CABLE_ANGLE_1, CANID_DUMMY,          0,     4);
	if (ret != 0)
	{
		printf("filter additions: failed init\n\r");USART1_txint_send(); 
		while (1==1);		
	}

//	/* Setup unit CAN ID in CAN hardware filter. */
	u32 id;
//	id = pcanidtbl->unit_code;
//	//  Add one 32b CAN id (     CAN1, CAN ID, FIFO)
//	ret = can_driver_filter_add_one_32b_id(0,id,1);
//	if (ret < 0)
//	{
//		printf("FLASHH unit CAN ID failed: %d\n\r",ret);USART1_txint_send(); 
//		while (1==1);
//	}

	/* Go through table and load "command can id" into CAN hardware filter. */
	//                     (CAN1, even, bank 2)
	can_driver_filter_setbanknum(0, 0, 2);
	jj = pcanidtbl->size;
	if (jj > 16) jj = 16;	// Check for bogus size
	for (ii = 0; ii < jj; ii++) 
	{
		id = pcanidtbl->slot[ii].canid;
		//  Add one 32b CAN id (     CAN1, CAN ID, FIFO)
//		ret = can_driver_filter_add_one_32b_id(0,id,0);
		if (ret < 0)
		{
			printf("FLASHH CAN id table load failed: %d\n\r",ret);USART1_txint_send(); 
			while (1==1);
		}
	}

/* ------------------------ CAN msg loop (runs under interrupt) --------------------------------------- */
	ret = CAN_poll_loop_init();
	if (ret != 0)
	{ // Here the init failed (e.g. malloc)
		printf("CAN_poll_loop_init: failed %d\n\r",ret);USART1_txint_send(); 
		while (1==1);		
	}
/* ---------------------- Setup GPS ascii port for reading GPS sentences. ------------------------------*/
	USART3_rxinttxint_init(57600,128,2,64,2);	//  1 Hz GPS 
	cGPStype = UBLOX_NEO_6M;	// Set type of GPS

/* ----------------- Switches and LED for flashgenie SD card socket module. --------------------------- */


	/* Switches and LED for flashgenie SD card socket module. */
	SD_socket_init();	// Setup the three pins 

	/* Check if SD card switch shows that is inserted. */
	SD_ret = SD_socket_sw_status(0);
	if (SD_ret != 0)
	{ // Here, socket switch shows no SD card present
		system_reset_waitmsg("SD SOCKET SHOWS THAT CARD IS NOT INSERTED\n\r");
	}
	else
	{ // Here, socket switch shows the SD card is present.
		/* This is synchronous and will take time while it searches the last written packet location. */
		i = sdlog_init();
	}

	/* Setup TIM4 to measure processor clock using GPS 1_PPS */
	Tim4_pod_init();	// Setup of the timers and pins for TIM1CH1* (* = remapped)

	/* TIM9 setup: CAN_poll_loop and also for flashing LEDs. */
	timer_debounce_init();	// Init timer and start
	LEDsetup();		// Add countdown structs
	LeftREDsetmode(2, 150);	// Left RED led to flash: FAST = SD NOT READY
	if (i != 0)	// Initialization OK?
	{
		sprintf(vv,"SD INITIALIZATION FAILED.  It returned: 0x%x (%d)\n\r",i,i);
		system_reset_waitmsg(vv);
	}
	else
	{
		SD_rdy = 1; // Show SD card successfully initialized.
		LeftREDsetmode(2, 2000);	// Left RED led to flash: SLOW = OK
		printf("sdlog_init: %d OK\n\r",i);
	}


/* --------------------------- FIX THIS ---------------------------------------------------------------- */
	/* YOU'D BETTER BE READY: At this point loggable CAN msgs will begin logging (under the low priority interrupt) */
	if (SD_rdy != 0)	// Skip logging if an initialized SD card is not present.
		can_log_init();

/* -------------- List parameters (debugging parameters) ---------------------------------------------- */
	can_log_printf(&logger_f.logger_s);	// List parameter values
	can_log_printf_extra(&logger_f);	// List non-parameter values in total function

/* ------------- Get params & init gps, then list ----------------------------------------------------- */
	gps_poll_init();	
	gps_poll_printf(&gps_f.gps_s);		// List parameter values
	gps_poll_printf_extra(&gps_f);		// List non-parameter values in total function
	Tim4_pod_se_set_sync_msg();	// Setup CAN ID and dlc for time sync msg

/* ---------------- When CAN interrupts are enabled reception of msgs begins! ------------------------ */
	can_driver_enable_interrupts();	// Enable CAN interrupts
	CAN_poll_loop_trigger_active = 1;	// enable CAN_loop_trigger interrupts
/* -------------------- Endless bliss of logging ----------------------------------------------------- */
extern u32 canlogct;
extern volatile u32 canlogSDwritectr;
//extern volatile u32 canlogloadctr;
//extern volatile u32 canlogqueuesctr;

extern u32 gps_poll_flag_ctr;
extern u8	tim4_readyforlogging;
u8	tim4_readyforlogging_prev = 0;
int	tim4_readyforlogging_ctr = 0;
extern unsigned int tim4debug5;
extern unsigned int ticks;
extern unsigned int ticks_dev;
extern unsigned int ticksadj;
extern unsigned int tim4debug1;
extern unsigned int tim4debug6;
unsigned int tim4debug6_prev =tim4debug6;
extern u16	tim4_64th_0_er;

extern volatile u32 logerrct;		// Count log msg overrun errors
extern volatile u32 logoverrunctr;	// Count of SD card buffer overruns
extern volatile u32 logbypassctr;	// Count of packets not written/bypassed
extern volatile int logbuffdepth;	// Max depth into canbuf[];
extern int log_can_tim;			// Max time for log_can_log to execute

printf("\n\r## tim9_tick_rate (ticks/sec): %d\n\n\r",(int)tim9_tick_rate);

#ifdef CHECKTHETIMER9TICKRATE
#define TIM9INCR 2000	// Number of tim9 ticks per second
uint32_t tim9_tick_ctr_next = (tim9_tick_ctr + TIM9INCR);
int ticker = 0;	
#endif


	/* Main polling loop.  Poll GPS and Logging (SD card write) */
	while (1==1)
	{
		/* Trigger polling loop. */
		// This triggers low level interrupt to make pass through CAN poll loop
		CAN_poll_loop_trigger();

#ifdef CHECKTHETIMER9TICKRATE
if ((int)(tim9_tick_ctr - tim9_tick_ctr_next) > 0)
{
	tim9_tick_ctr_next = tim9_tick_ctr + TIM9INCR;
	printf("TICK %d\n\r",ticker++); // Should be once per second
}
#endif

		/* Handle the time extraction & conversion. */
		gps_poll(&gps_f);

		if (SD_ret == 0)
		{ // Here, the SD card was initially inserted
			if (SD_socket_sw_status(1) != 0)
			{ // Here, the SD card switch is now OFF
				system_reset_waitmsg("SD CARD WAS IN, BUT IT IS NOW SEEN AS OUT!  Insert and routine will reboot\n\r");
			}
		}

		if (SD_rdy != 0)
		{ // Here, a successfully initialized SD card
			/* Poll to log */
			if (sdlog_error == 0)	// 'can_log_can' sets 'sdlog_error'
			{
				can_log_trigger_logging();	// Trigger low priority interrupt to log all CAN msgs that are buffered
			}
			else
			{ // Here we have an unrecoverable error with the SD card.
				sprintf (vv,"HELP!  Something went wrong with the SD card.  Error code: %d\n\r",sdlog_error);
				system_reset_waitmsg(vv);
			}
		}

		/* Poll for keyboard/PC input and handling in the thereafter. */
		p1_PC_handler();

		/* LED 1 sec on/off to entertain the hapless op. */
		if (ticks_flg != ticks_flg_prev)	// One sec ticks
		{ // Here yes.
			ticks_flg_prev = ticks_flg;	// Update flag counter
//                                            
printf("%5d %5d %3d %3d %3d %3d %3d %3d",(int)canlogct,(int)canlogSDwritectr,(int)logerrct,(int)logoverrunctr,(int)logbypassctr,(int)logbuffdepth,(int)log_can_tim,(int)gps_poll_flag_ctr); USART1_txint_send();
if (tim4_readyforlogging != tim4_readyforlogging_prev)
{
	tim4_readyforlogging_prev = tim4_readyforlogging;
	tim4_readyforlogging_ctr += 1;
}
printf("%2d %3d %5d",tim4_readyforlogging, tim4_readyforlogging_ctr,tim4_64th_0_er);USART1_txint_send();
printf(" %7d %4d %5d %6d %d", ticks, ticks_dev, ticksadj, tim4debug5,tim4debug1);USART1_txint_send();
printf(" |%4d %3d\n\r",tim4debug6-tim4debug6_prev,ticks_dev-(tim4debug6-tim4debug6_prev));tim4debug6_prev = tim4debug6;USART1_txint_send();

			if ((ticks_flg & 1) == 0)	// Flash the leds on the Flashgenie socket
				SD_socket_setled(0);
			else
				SD_socket_setled(1);	
		}
	}
	return 0;	
}


