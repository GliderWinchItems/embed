/******************************************************************************
* File Name          : cansender.c
* Date First Issued  : 09/08/2016
* Board              : Sensor board
* Description        : CAN sending (responding) routine for testing
*******************************************************************************/
/* 

Strategy--
04/24/2016: I beseech you to read README.tension, and you
may be better informed than just the following...then maybe not.

07/xx/2015:
After intialization--
  main: endless loop
    printf for monitoring
    triggers a low level interrupt to poll send/handle CAN msgs
  timer3: interrupts 2048/sec
    Starts a SPI readout if there is a reading ready
  CAN RX0, RX1: 
    triggers low level interrupt to poll send/handle CAN msgs 
 
Interrupt levels
main             = background loop: printf, temperature computation, etc.
NVIC_I2C1_ER_IRQ = Polling loop for handling/sending msgs 'CAN_poll'
NVIC_I2C1_EV_IRQ = SPI polling loop (high priority than 'ER')
DMA1 CH1         = ADC
TIM1CH1          = CAN_poll in absence of CAN msgs
CAN_TX           = CAN1 xmit
CAN_RX0          = CAN1 FIFO0 (lower priority)
CAN_RX1          = CAN1 FIFO1 (high priority msgs)
UART1            = Teletype output
*/

#include <math.h>
#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
extern int errno;

#include "pinconfig_all.h"
#include "SENSORpinconfig.h"

//#include "libopenstm32/adc.h"
#include "libopenstm32/can.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/gpio.h"
#include "libopenstm32/usart.h"
#include "libopenstm32/scb.h"

#include "libusartstm32/usartallproto.h"

//#include "libmiscstm32/printf.h"

#include "libmiscstm32/clockspecifysetup.h"
#include "libmiscstm32/DTW_counter.h"

#include "libsensormisc/canwinch_setup_F103_pod.h"

#include "../../../../svn_common/trunk/common_can.h"

#include "../../../../svn_discoveryf4/sw_discoveryf4/trunk/lib/libopencm3/cm3/common.h"

#include "poly_compute.h"

//#include "canwinch_pod_common_systick2048_printerr.h"
#include "cansender_idx_v_struct.h"
#include "cansender_function.h"
#include "cansender_printf.h"
#include "CAN_poll_loop.h"
#include "libmiscstm32/DTW_counter.h"
#include "can_nxp_setRS.h"
//#include "p1_initialization.h"
#include "can_driver.h"
#include "can_driver_filter.h"
#include "can_msg_reset.h"
#include "can_gps_phasing.h"
#include "sensor_threshold.h"
#include "panic_leds.h"
#include "db/gen_db.h"
#include "CAN_poll_loop.h"
#include "../../../../svn_common/trunk/common_highflash.h"

#include "cansender_idx_v_struct.h"
#include "tim3_ten2.h"
#include "can_filter_print.h"
#include "can_driver_errors_printf.h"

/* ############################################################################# */
#define USEDEFAULTPARAMETERS	0	// 0 = Initialize sram struct with default parameters
/* ############################################################################# */

int dummy;

/* Make block that can be accessed via the "jump" vector (e.g. 0x08005004) that points
   to 'unique_can_block', which then starts Reset_Handler in the normal way.  The loader
   program knows the vector address so it can then get the address of 'unique_can_block'
   and get the crc, size, and data area via a fixed offset from the 'unique_can_block'
   address. 

   The .ld file assures that the 'unique_can_block' function is is followed by the data
   by using the sections.  */
//#include "startup_deh.h"
//__attribute__ ((section(".ctbltext")))
//void unique_can_block(void)
//{
//	Reset_Handler();
//	while(1==1);
//}

/* The order of the following is important. */
//extern const struct FLASHP_SE4 __highflashp;
//__attribute__ ((section(".ctbldata")))
//const unsigned int flashp_size = sizeof (struct FLASHP_SE4);
//__attribute__ ((section(".ctbldata1")))
//const struct FLASHP_SE4* flashp_se3 = (struct FLASHP_SE4*)&__highflashp;

/* Subroutine declarations. */
void ciccalibrateprint(int n);

/* Easy way for other routines to access via 'extern'*/
struct CAN_CTLBLOCK* pctl0;

/* Specify msg buffer and max useage for CAN1: TX, RX0, and RX1. */
const struct CAN_INIT msginit = { \
180,	/* Total number of msg blocks. */
140,	/* TX can use this huge ammount. */
64,	/* RX0 can use this many. */
8	/* RX1 can use this piddling amount. */
};

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
//void putc ( void* p, char c)
//	{
//		p=p;	// Get rid of the unused variable compiler warning
//		CANascii_putc(c);
//		USART1_txint_putc(c);
//	}

void double_to_char(double f,char * buffer){
    gcvt(f,12,buffer);
}

/*#################################################################################################
And now for the main routine 
  #################################################################################################*/
int main(void)
{
//	int i;
//	int j;
	int ret;

/* $$$$$$$$$$$$ Relocate the interrupt vectors from the loader to this program $$$$$$$$$$$$$$$$$$$$ */
extern void relocate_vector(void);
	relocate_vector();
/* --------------------- Begin setting things up -------------------------------------------------- */ 
	// Start system clocks using parameters matching CAN setup parameters for POD board
	clockspecifysetup(canwinch_setup_F103_pod_clocks() );
/* ---------------------- Set up pins ------------------------------------------------------------- */
	SENSORgpiopins_Config();	// Now, configure pins
	LED19RED_off;
	LED20RED_off;
/* ---------------------- Set up 32b DTW system counter ------------------------------------------- */
	DTW_counter_init();

setbuf(stdout, NULL);
//setvbuf( stdout, 0, _IONBF, 0 ); 
//	init_printf(0,putc);	// This one-time initialization is needed by the tiny printf routine
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
	USART1_txint_puts("\n\rCANSENDER 09-09-2016\n\r");USART1_txint_send();

	/* Display things so's to entertain the hapless op */
	printf ("  hclk_freq (MHz) : %9u\n\r",  hclk_freq/1000000);
	printf (" pclk1_freq (MHz) : %9u\n\r", pclk1_freq/1000000);
	printf (" pclk2_freq (MHz) : %9u\n\r", pclk2_freq/1000000);
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
	can_nxp_setRS_sys(0,1); // (1st arg) 0 = high speed mode; 1 = standby mode (Sets yellow led on)

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

//	/* Setup unit CAN ID in CAN hardware filter. */
	u32 id;

	/* Go through table and load "command can id" into CAN hardware filter. */
	//                     (CAN1, even, bank 2)
	can_driver_filter_setbanknum(0, 0, 2);
	jj = pcanidtbl->size;
	if (jj > 16) jj = 16;	// Check for bogus size
	for (ii = 0; ii < jj; ii++) 
	{
		id = pcanidtbl->slot[ii].canid;
		//  Add one 32b CAN id (     CAN1, CAN ID, FIFO)
		ret = can_driver_filter_add_one_32b_id(0,id,0);
		if (ret < 0)
		{
			printf("FLASHH CAN id table load failed: %d\n\r",ret);USART1_txint_send(); 
			while (1==1);
		}
	}
/* ---------------- tension A ------------------------------------------------------------------------ */
	ret = cansender_function_init_all();
	if (ret <= 0)
	{
		printf("cansender_function: table size mismatch count: %d\n\r", ret);USART1_txint_send(); 
		while(1==1);
	}
	printf("cansender_function: table size : %d\n\r", ret);USART1_txint_send();

/* ----------------- CAN filter registers ------------------------------------------------------------- */
	can_filter_print(14);	// Print the CAN filter registers
/* ------------------------ Various and sundry ------------------------------------------------------- */

	cansender_printf(&send_f.send_a);	// Print parameter list
/* ------------------------ CAN msg loop (runs under interrupt) --------------------------------------- */
	ret = CAN_poll_loop_init();
	if (ret != 0)
	{ // Here the init failed (e.g. malloc)
		printf("CAN_poll_loop_init: failed %d\n\r",ret);USART1_txint_send(); 
		while (1==1);		
	}
/* ---------------- When CAN interrupts are enabled reception of msgs begins! ------------------------ */
	can_driver_enable_interrupts();	// Enable CAN interrupts

/* ---------------- Some vars associated with endless loop ------------------------------------------- */
#define FLASHTIMEINC (64000000/1)	// Yellow LED flash tick duration
u32 ledtime = DTWTIME + FLASHTIMEINC;	// Init the first timeout


#define CANDRIVERPRINTFCT 10	// Number of LED ticks between printing can_driver error counts
unsigned int cdect = 0;		// Counter for timing printing of can_driver errors

/* --------------- Start TIM3 CH1 and CH2 interrupts ------------------------------------------------- */
	/* The low level CAN polling rate is timer 3 rate divided by: TIM3LLTHROTTLE = 4  */
	tim3_ten2_rate = 4096;//2048;
	tim3_ten2_init(pclk1_freq/2048);	// 64E6/2048

/* --------------------- Endless Stuff ----------------------------------------------- */
	while (1==1)
	{
		/* ---------- Tick time flashing (in case 'ad7799_ten_flag' not working) ---------- */
		if ( ((int)ledtime - (int)DTWTIME) < 0) 
		{
			ledtime += FLASHTIMEINC;
			TOGGLE_GREEN;

			/* Display can_driver error counts periodically. */
			cdect += 1;
			if (cdect >= CANDRIVERPRINTFCT)
			{
				cdect = 0;
				can_driver_errors_printf(pctl0);
				can_driver_errortotal_printf(pctl0);
			}
		}

	/* ---------- Trigger a pass through 'CAN_poll' to poll msg handling & sending. ---------- */
	CAN_poll_loop_trigger();
	}
	return 0;	
}

