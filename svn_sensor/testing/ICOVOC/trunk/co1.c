/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : co1.c
* Hackeroo           : deh
* Date First Issued  : 12/25/2012
* Board              : STM32F103VxT6_pod_mm
* Description        : Use pod board to prototype a sensor logger ### DEH OLIMEX 1 ###
*******************************************************************************/
/* 
01/31/2013 rev 114 Tim4_pod_se.c with 1PPS & OC, but not debugged 
rev 119 seemed to work OK.


Open minicom on the PC with 115200 baud and 8N1.

*/
#include <math.h>
#include <string.h>

/* ../svn_pod/sw_stm32/trunk/lib */
#include "PODpinconfig.h"
#include "common.h"
#include "libopenstm32/adc.h"
#include "libopenstm32/can.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/gpio.h"
#include "libopenstm32/usart.h"
#include "libopenstm32/scb.h"
#include "libusartstm32/usartallproto.h"
#include "libmiscstm32/systick1.h"
#include "libmiscstm32/printf.h"
#include "libmiscstm32/clockspecifysetup.h"
#include "sdlog.h"

/* ../svn_sensor/sw_f103/trunk/lib */
#include "adcsensor_pod.h"
#include "canwinch_pod.h"
#include "sensor_threshold.h"
//#include "common_can.h"
//#include "common_time.h"
//#include "can_log.h"
//#include "p1_gps_1pps.h"
//#include "p1_gps_time_convert.h"
//#include "gps_poll.h"
#include "gps_1pps_se.h"

/* Each node on the CAN bus gets a unit number */
#define IAMUNITNUMBER	CAN_UNITID_CO_OLI	// Olimex (DEH board) CO

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
 * void putc ( void* p, char c); // This is for the tiny printf
 * ************************************************************************************** */
// Note: the compiler will give a warning about conflicting types
// for the built in function 'putc'.  Use ' -fno-builtin-putc' to eliminate compile warning.
void putc ( void* p, char c)
	{
		p=p;	// Get rid of the unused variable compiler warning
		USART2_txmin_putc(c);
	}

/*#################################################################################################
  And now---the main routine!
  #################################################################################################*/
int main(void)
{

/* --------------------- Begin setting things up -------------------------------------------------- */ 

	clockspecifysetup((struct CLOCKS*)&clocks);		// Get the system clock and bus clocks running

/* ---------------------- Set up pins ------------------------------------------------------------- */
	gpio_setup();		// Need this to make the LED work

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
//	USART2_rxinttxint_init(115200,32,2,96,4); // Initialize USART and setup control blocks and pointers

	USART2_txmin_init(115200);

	/* Announce who we are */
	USART2_txmin_puts("\n\rco1_OLIMEX ###IC OV OC TEST### 02-05-2013 16:29\n\r");
	// USART2_txint_send();	// Start the line buffer sending

	/* Display things for to entertain the hapless op */
	printf ("  hclk_freq (MHz) : %9u\n\r",  hclk_freq/1000000);	// USART2_txint_send();
	printf (" pclk1_freq (MHz) : %9u\n\r", pclk1_freq/1000000);	// USART2_txint_send();
	printf (" pclk2_freq (MHz) : %9u\n\r", pclk2_freq/1000000);	// USART2_txint_send();
	printf ("sysclk_freq (MHz) : %9u\n\r",sysclk_freq/1000000);	// USART2_txint_send();


/* --------------------- Setup for CO'ing  ------------------------------------------------------------ */

	/* Setup GPS ascii port for reading GPS sentences. */
//	USART1_rxinttxint_init(4800,96,2,48,2);	//  1 Hz GPS 



	/* Setup TIM1 to measure processor clock using GPS 1_PPS */
	Tim4_pod_init();	// Setup of the timers and pins for TIM1CH1* (* = remapped)


/* -------------------- Endless bliss of logging ----------------------------------------------------- */
static unsigned int errctr;

extern unsigned int ticks;
extern volatile int ticks_dev;
extern volatile unsigned int ticks_flg;
unsigned int ticks_flg_prev = 0;

extern volatile unsigned int tim4debug0;
extern volatile unsigned int tim4debug1;
extern volatile unsigned int tim4debug2;
extern volatile unsigned int tim4debug3;
extern volatile unsigned int tim4debug4;
extern volatile unsigned int tim4debug5;
extern volatile unsigned int tim4debug6;
extern volatile unsigned int tim4debug7;

extern volatile unsigned int tim4cyncnt;

unsigned int tim4cycnt_prev = 0;
int ticksvcyncnt = 0;
int cycntHI = -32000000;
int cycntLO = +32000000;


unsigned int tim4debug2_prev = 0;
unsigned int tim4debug3_prev = 0;
unsigned int tim4debug4_prev = 0;
unsigned int tim4debug5_prev = 0;
unsigned int tim4debug6_prev = 0;

int occt = 0;
int ocmiss = 0;
int ocmiss_prev = 0;
int octmp = 0;
int onetime = 0;
unsigned int k = 0;
int diff2 = 0;
int diff3 = 0;
int diff5 = 0;
int diff6 = 0;

ticks_flg_prev = ticks_flg;

	while (1==1)
	{
		/* Some monitoring stuff for Tim4_pod_se.c debugging. */
		if (ticks_flg != ticks_flg_prev)	// A new reading?
		{ // Here yes.
			toggle_led ();
			ticks_flg_prev = ticks_flg;	// Update flag counter

			ticksvcyncnt = (int)(ticks - (int)(tim4cyncnt-tim4cycnt_prev)); 
			tim4cycnt_prev = tim4cyncnt;

		if (onetime++ > 4)
		{
			onetime = 5;

			if (ticksvcyncnt > cycntHI) cycntHI = ticksvcyncnt;
			if (ticksvcyncnt < cycntLO) cycntLO = ticksvcyncnt;
		}

			occt = (int)(tim4debug5 - tim4debug5_prev);
			if (ticksvcyncnt < -32768) errctr += 1;
			octmp = (int)(tim4debug3-tim4debug3_prev);
			if ((octmp > 1282) || (octmp < 1279)) ocmiss += 1;

		if ((ocmiss - ocmiss_prev) != 0)
		{
			ocmiss_prev = ocmiss;
		}
		diff6 = (tim4debug6 - tim4debug6_prev); tim4debug6_prev = tim4debug6;
		diff5 = (tim4debug5 - tim4debug5_prev); tim4debug5_prev = tim4debug5;
		diff3 = (tim4debug3 - tim4debug3_prev); tim4debug3_prev = tim4debug3;
		diff2 = (tim4debug2 - tim4debug2_prev); tim4debug2_prev = tim4debug2;


printf ("%5u ",k++);
printf ("%10d ", ticksvcyncnt);
printf ("%8u %8d %4u %6u %6u ",ticks, ticks_dev,tim4_tickspersec_err, diff2, ticks_flg);
printf ("%8u %5u ", diff5, diff3 );
printf ("%6d %6d ", (int)tim4debug0, tim4debug1);

printf ("%8d %8d %8d %8d %4u ", deviation_oneinterval, phasing_oneinterval, tim4debug4, diff6, tim4debug7);
//printf ("%5u %5u ", diff2, diff3 );
//printf ("%8u ", diff5);

//, tim4debug0,tim4debug2-tim4debug2_prev,tim4debug3-tim4debug3_prev,tim4debug5,occt,errctr, tim4debug1, ocmiss);
//printf (" %6d %6d ",cycntHI,cycntLO);

printf("\n\r");

			// USART2_txint_send();
			
			tim4debug4_prev = tim4debug4;
		}

	}
	return 0;	
}

