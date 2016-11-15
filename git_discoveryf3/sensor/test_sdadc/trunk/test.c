/******************************************************************************
* File Name          : test_sdadc/test.c
* Date               : 01/20/2015
* Board              : F3-Discovery w STM32F373 processor
* Description        : Initial testing of SDADC 
*******************************************************************************/
/* 



*/
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>


#include <math.h>
#include <string.h>
#include <stdio.h>
#include "xprintf.h"

#include <malloc.h>

#include "libopencm3/stm32/rcc.h"
#include "libopencm3/stm32/gpio.h"
#include "libopencm3/cm3/scb.h"

#include "../../../lib/libusartstm32f3/bsp_uart.h"
#include "systick1.h"
#include "clockspecifysetup_F3.h"

#include "f3DISCpinconfig.h"	// Pin configuration for STM32 Discovery board
#include "can_driver.h"
#include "common_can.h"
#include "panic_ledsDf3.h"
#include "PC_gateway_comm.h"
#include "USB_PC_gateway.h"
#include "systick.h"
//#include "CAN_test_msgs.h"
//#include "CAN_error_msgs.h"
//#include "canwinch_setup_F3_discovery.h"
#include "DTW_counter_F3.h"
#include "canwinch_setup_F3.h"
#include "sdadc_discovery.h"
#include "libopencm3/stm32/f3/sdadc.h"
#include "f3Discovery_led_pinconfig.h"	// DiscoveryF3 board LEDs
#include "sdadc_filter.h"
#include "sdadc_poly_correct.h"
#include "sdadc_poly_correct_f.h"
#include "sdadc_recalib_stddev.h"

#include "cic_filter_l_N8_M3_f3.h"

struct STDDEV 
{
	int64_t sum;
	int64_t sum2;	
	int64_t sumb;
	int64_t sum2b;	
};
struct STDDEVCTL
{
	uint16_t flag;
	uint16_t idx;
	uint16_t accum;
};

#define NUMDEVACCUM1	400
#define NUMDEVACCUM2	400
#define NUMDEVACCUM3	400

struct STDDEVCTL stdevctl[NUMBERSDADCS];

/* Vars for computing ave and std dev */
extern int16_t buff1[2][NUMBERSEQ1][NUMBERSDADC1];	/* Double DMA buffer for #1 */
extern int16_t buff2[2][NUMBERSEQ2][NUMBERSDADC2];	/* Double DMA buffer for #2 */
extern int16_t buff3[2][NUMBERSEQ3][NUMBERSDADC3];	/* Double DMA buffer for #3 */

static struct STDDEV stddev1[NUMBERSDADC1];
static struct STDDEV stddev2[NUMBERSDADC2];
static struct STDDEV stddev3[NUMBERSDADC3];

/* Fixed parameters and pointers for SDADCx */
extern const struct SDADCFIX sdadcfix[NUMBERSDADCS];

#define NUMBUFFSAVE1	256	// Number if half buffs in 1st std dev calc
#define NUMBUFFSAVE2	256	// Number if half buffs in 1st std dev calc
#define NUMBUFFSAVE3	256	// Number if half buffs in 1st std dev calc

#define NUMDEVBUFF1 2		// Interrupt overlap buffering
#define NUMDEVBUFF2 2		// Interrupt overlap buffering
#define NUMDEVBUFF3 2		// Interrupt overlap buffering

struct STDDEVPARM
{
const	struct SDADCFIX *pfix;
	struct STDDEVCTL *pdevctl;
	struct STDDEV *pdvbase;
	int16_t *dmabase0;
	int16_t *dmabase1;
	uint16_t ACCUM;
	uint16_t NUMBUFF;
	uint16_t NUMSAVE;
	

};
const struct STDDEVPARM stddevparm[NUMBERSDADCS] = {\
  {
	&sdadcfix[0],
	&stdevctl[0],
	&stddev1[0],
	&buff1[0][0][0],
	&buff1[1][0][0],
	NUMDEVACCUM1,
	NUMDEVBUFF1,
	NUMBUFFSAVE1,
  },{
	&sdadcfix[1],
	&stdevctl[1],
	&stddev2[0],
	&buff2[0][0][0],
	&buff2[1][0][0],
	NUMDEVACCUM2,
	NUMDEVBUFF2,
	NUMBUFFSAVE2,
  },{
	&sdadcfix[2],
	&stdevctl[2],
	&stddev3[0],
	&buff3[0][0][0],
	&buff3[1][0][0],
	NUMDEVACCUM3,
	NUMDEVBUFF3,
	NUMBUFFSAVE3,
  }
};


/* Fixed parameters and pointers for SDADCx */
extern const struct SDADCFIX sdadcfix[NUMBERSDADCS];

/* Number of sums for computing ave and std dev. */
static const int16_t sdadcDevN[NUMBERSDADCS] = {4095,3500,2400};
const char sdchar[3] = {'A','B','C'};

void stddevsum(const struct SDADCFIX *p, struct STDDEV *pdev);

extern unsigned int	sysclk_freq;	/* 	SYSCLK freq		E.g. 72000000	*/

#ifndef NULL 
#define NULL	0
#endif

/* Circular buffer for passing CAN BUS msgs to PC */
#define CANBUSBUFSIZE	64	// Number of incoming CAN msgs to buffer
//C static struct CANRCVBUF canbuf[CANBUSBUFSIZE];
//C static u32 canmsgct[CANBUSBUFSIZE]; // Msg seq number for CAN-to-PC.
//C static int canbufidxi = 0;	// Incoming index into canbuf
//C static int canbufidxm = 0;	// Outgoing index into canbuf

//C static void canbuf_add(struct CANRCVBUF* p);
//C void canin(struct CANRCVBUF* p);		// Debug
//C void caninB(struct CANRCVBUF* p);  	// Debug


/* USART|UART assignment for xprintf and read/write */
#define UXPRT	1	// Uart number for 'xprintf' messages
#define USTDO	1	// Uart number for gateway (STDOUT_FILE, STDIIN_FILE)

/* &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& */
/* ------------- Each node on the CAN bus gets a unit number -------------------------- */
#define IAMUNITNUMBER	CAN_UNITID_GATE2	// PC<->CAN bus gateway
/* &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& */

/* Specify msg buffer and max useage for CAN1: TX, RX0, and RX1. */
const struct CAN_INIT msginit1 = { \
180,	/* Total number of msg blocks. */
80,	/* TX can use this huge ammount. */
80,	/* RX0 can use this many. */
16	/* RX1 can use this piddling amount. */
};

/* Easy way for other routines to access via 'extern'*/
struct CAN_CTLBLOCK* pctl1;	// CAN1 control block pointer

static struct PCTOGATEWAY pctogateway; 	// CAN->PC
static struct PCTOGATEWAY pctogateway1; // PC->CAN
//C static struct CANRCVBUF canrcvbuf;

//C static struct CANRCVBUF* 	pfifo0;	// Pointer to CAN driver buffer for incoming CAN msgs, low priority
//C static struct CANRCVBUF*	pfifo1;	// Pointer to CAN driver buffer for incoming CAN msgs, high priority

/* Put sequence number on incoming CAN messages that will be sent to the PC */
u8 canmsgctr = 0;	// Count incoming CAN msgs

/* file descriptor */
int fd;

char vv[128];	// sprintf buf

static u32	t_cmdn;


/* Parameters for setting up clock. (See: "libmiscstm32/clockspecifysetup.h" */
const struct CLOCKSF3 clocks1 = { \
HSOSELECT_HSE_XTAL,	/* Select high speed osc: 0 = internal 8 MHz rc; 1 = external xtal controlled; 2 = ext input; 3 ext remapped xtal; 4 ext input */ \
1,			/* Source for main PLL: 0 = HSI, 1 = HSE selected */ \
0,			/* HSE predivider: 0 = not divided, 1 = 2x...15 = 16x */ \
APBX_2,			/* APB1 clock = SYSCLK divided by 1,2,4,8,16; freq <= 36 MHz */ \
APBX_1,			/* APB2 prescalar code = SYSCLK divided by 1,2,4,8,16; freq <= 72 MHz */ \
AHB_1,			/* AHB prescalar code: SYSCLK/[2,4,8,16,32,64,128,256,512] (drives APB1,2 and HCLK) */ \
8000000,		/* External Oscillator source frequency, e.g. 8000000 for an 8 MHz xtal on the external osc. */ \
8,			/* M PLL Multiplier: 2-16 */ \
};

/* **************************************************************************************
 * static void panic_leds(uint32_t n);
 * @brief	: call panic routine
 * @param	: n = coode
 * ************************************************************************************** */
static void panic_leds(uint32_t n)
{
	panic_ledsDf3(n);

}
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
/*******************************************************************************
 * void can_nxp_setRS(int rs, volatile u32 * p, u16 pinnumber );
 * @brief 	: Set RS input to NXP CAN driver (TJA1051) (on some PODs) (SYSTICK version)
 * @param	: rs: 0 = NORMAL mode; not-zero = SILENT mode 
 * @param	: p = pointer to port
 * @param	: pinnumber = port pin number
 * @return	: Nothing for now.
*******************************************************************************/
//  mode output alfternate function, pushpull, not applicable, no pull up/dn, alternative function AFRLy & AFRHy selection */
static const struct PINCONFIG	outputpp = { \
	GPIO_MODE_OUTPUT, 	// mode: Output
	GPIO_OTYPE_PP, 		// output type: push pull	
	GPIO_OSPEED_100MHZ, 	// speed: fastest 
	GPIO_PUPD_NONE, 	// pull up/down: none
	0 };			// AFRLy & AFRHy selection: not applicable

void can_nxp_setRS(int rs, volatile u32 * p, u16 pinnumber )
{
	/* RS (S) control PB7 (on sensor board) PD11 on pod board */
	// Floating input = resistor controls slope
	// Pin HI = standby;
	// Pin LO = high speed;

	/* Setup the i/o pin as output push-pull */
	f3gpiopins_Config ( p, pinnumber, (struct PINCONFIG*)&outputpp);


	if (rs == 0)
		GPIO_BSRR(p) = (1<<(pinnumber+16));	// Set bit LO for SILENT mode
	else
		GPIO_BSRR(p) = (1<<pinnumber);	// Set bit HI for NORMAL mode

	return;
}

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
//	int tmp;
//u32 xctr = 1;
//u32 yctr = 0;
//u32 zctr = 0;

//	int temp;


/* --------------------- Begin setting things up -------------------------------------------------- */ 
	clockspecifysetup_F3((struct CLOCKSF3*)&clocks1); // Get the system clock and bus clocks running
/* ---------------------- Set up pins ------------------------------------------------------------- */
	f3Discovery_led_pinconfig_init();	// Setup LED pins
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

   USART2 and USART2 are shown below.  Select one, or make up one for the USART/UART this will be used.  
   Either DMA or CHAR-BY-CHAR interrupt driven can be used.  DMA for faster high volume loads.

*/
/*	DMA DRIVEN  (note: non-blocking, i.e. discard chars if port buff is full.) */
// int bsp_uart_dma_init_number(u32 iuart, u32 baud, u32 rxbuffsize, u32 txbuffsize, u32 dma_tx_int_priority, u8 block, u8 nstop);
//	bsp_uart_dma_init_number(USTDO,2000000, 2048, 1024, 0xd0, 1, 0); // Flashing LED's means failed and you are screwed.
//	bsp_uart_dma_init_number(USTDO, 921600, 2048, 1024, 0xd0, 1, 0); // Flashing LED's means failed and you are screwed.
//	bsp_uart_dma_init_number(USTDO, 460800,  256,  256, 0xd0, 1, 0); // Flashing LED's means failed and you are screwed.
//	bsp_uart_dma_init_number(USTDO, 230400, 1024,  256, 0xd0, 1, 0); // Flashing LED's means failed and you are screwed.
//	bsp_uart_dma_init_number(USTDO, 115200,  256,  256, 0xd0, 1, 0); // Flashing LED's means failed and you are screwed.
	bsp_uart_dma_init_number(UXPRT, 115200,   64,  128, 0x30, 1, 0); // Flashing LED's means failed and you are screwed.

/*	CHAR-BY-CHAR INTERRUPT DRIVEN  (note: non-blocking, i.e. discard chars if port buff is full.) */
// int bsp_uart_init_number(u32 iuart, u32 baud, u32 txbuffsize, u32 rxbuffsize,  u32 uart_int_priority	
//	u8 block, u8 nstop);
//	bsp_uart_int_init_number(USTDO, 460800,  256,  256, 0x30, 1, 0);
//	bsp_uart_int_init_number(USTDO, 230400, 4096, 1024, 0x40, 1, 0);
//	bsp_uart_int_init_number(USTDO, 921600, 4092, 1024, 0x40, 1, 0);
//	bsp_uart_int_init_number(USTDO, 115200,  256,  256, 0x10, 1, 0);

//	bsp_uart_int_init_number(UXPRT, 115200,  128,  512, 0x30, 0, 0);

/* Setup STDOUT, STDIN (a shameful sequence until we sort out 'newlib' and 'fopen'.)  The following 'open' sets up 
   the USART/UART that will be used as STDOUT_FILENO, and STDIN_FILENO.  Don't call 'open' again!  */
//	fd = open("tty2", 0,0); // This sets up the uart control block pointer versus file descriptor ('fd')
	fd = open("tty1", 0,0); // This sets up the uart control block pointer versus file descriptor ('fd')
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
		xprintf(UXPRT,  "X\n\rDISCOVERYF3 WITH F373: test.c \n\r");
		/* Make sure we have the correct bus frequencies */
		xprintf (UXPRT, "   hclk_freq (MHz) : %9u...............................\n\r",  hclk_freq/1000000);	
		xprintf (UXPRT, "  pclk1_freq (MHz) : %9u...............................\n\r", pclk1_freq/1000000);	
		xprintf (UXPRT, "  pclk2_freq (MHz) : %9u...............................\n\r", pclk2_freq/1000000);	
		xprintf (UXPRT, " sysclk_freq (MHz) : %9u...............................\n\r",sysclk_freq/1000000);
	}

//volatile int idelay = 90000;
//while (idelay-- > 0);

u32 freqMhz = sysclk_freq/1000000;
volatile float a = .14159265;
volatile float x;
volatile unsigned int f0 = DTWTIME;
x = a * 12.7 + 10.1;
volatile unsigned int f1 = DTWTIME;
//int ix = x * 1000000;
//xprintf (UXPRT, "fp test int: %d",ix);
xprintf (UXPRT, "fp test: %10.6f",(double)x);
volatile unsigned int f2 = DTWTIME;
xprintf (UXPRT, "  dur0 (tick): %d  dur1 (usec): %d\n\r",(f1-f0),(f2-f1)/freqMhz);

//idelay = 90000;
//while (idelay-- > 0);

f0 = DTWTIME;
x = atanf(a);
f1 = DTWTIME;
xprintf (UXPRT, "atanf test: %10.6f",(double)x);
f2 = DTWTIME;
xprintf (UXPRT, "  dur0 (tick): %d  dur1 (usec): %d\n\r",(f1-f0),(f2-f1)/freqMhz);

f0 = DTWTIME;
volatile double aa = .1415926535897932;
double xx = atan(aa);
f1 = DTWTIME;
xprintf (UXPRT, "atan  test: %10.6f",xx);
f2 = DTWTIME;
xprintf (UXPRT, "  dur0 (tick): %d  dur1 (usec): %d\n\r",(f1-f0),(f2-f1)/freqMhz);

f0 = DTWTIME;
volatile long double aaa = .1415926535897932;
long double xxx = atanl(aaa);
f1 = DTWTIME;
xprintf (UXPRT, "atanl test: %10.6f",xxx);
f2 = DTWTIME;
xprintf (UXPRT, "  dur0 (tick): %d  dur1 (usec): %d\n\r",(f1-f0),(f2-f1)/freqMhz);

volatile int idelay = 900000; // Big delay to let USART complete
while (idelay-- > 0);

/* ----------------- SDADC setup ---------------------------------------------------------------------- */
	sdadc123_filter_init();		// Filtering initialization
	sdadc_discovery_init();		// SDADC and DMA setup
	sdadc_set_recalib_ct(3, 4); 	// (SDADC number, DMA interrupts per re-calibration)
	sdadc_set_recalib_ct(2, 41); 	// (SDADC number, DMA interrupts per re-calibration)
	sdadc_set_recalib_ct(1, 52); 	// (SDADC number, DMA interrupts per re-calibration)

struct CICLN8M3 cic8;
cic_filter_l_N8_M3_f3_init (&cic8, 8);// TEST that init doesn't bomb

// Check if time for SDADC calibration
extern uint32_t sdbug0;
extern uint32_t sdbug1;
int sd01 = sdbug1-sdbug0;;
xprintf(UXPRT, "CALIB: ticks %d usec %d\n\r",sd01,sd01/64);
/* ------------------ Blinking lights ----------------------------------------------------------------- */
//int j;
int k = 0;
int btmp;
int btmp1;
int btmp2;
int btmp3;
int otmp[3];
//u32 t0,t1;
//int t2;
u32 alt = 1;
f0 = DTWTIME+4000000;
//extern uint32_t sdbugI0;
//extern uint32_t sdbugI1;

//7 #define SDADCNOTSKIPRAWNUMBERS	// Show raw numbers from DMA buffer
//#define STDDEVNOTSKIPCHAN2		// Show std dev for SDADC2
//#define SKIPPY			// Show Offset registers

/* Vars for computing ave and std dev */
#ifdef SDADCNOTSKIPRAWNUMBERS
extern int16_t buff1[2][NUMBERSEQ1][NUMBERSDADC1];	/* Double DMA buffer for #1 */
extern int16_t buff2[2][NUMBERSEQ2][NUMBERSDADC2];	/* Double DMA buffer for #2 */
extern int16_t buff3[2][NUMBERSEQ3][NUMBERSDADC3];	/* Double DMA buffer for #3 */

static struct STDDEV stddev1[NUMBERSDADC1];
static struct STDDEV stddev2[NUMBERSDADC2];
static struct STDDEV stddev3[NUMBERSDADC3];


int j1 = 0;
#endif

int ii;



/* Counters for stdev display throttling. */
//int32_t num1 = 0;
//int32_t num2 = 0;
//int32_t num3 = 0;

//int32_t numb1 = 0;
//int32_t numb2 = 0;
//int32_t numb3 = 0;

/* Intermediate for stddev computation. */
//int64_t xb;
//int j;
//int16_t *pm;
//struct STDDEV *pdv;
//int64_t llvar,llmean;
//float fvar;
//double dmean;
//uint32_t stmp;
//double dtmp;

//extern uint16_t buff1sv_flag;
//#define DISTRIBUTIONCHECK
#ifdef DISTRIBUTIONCHECK
int tmp;
int imean;
int jj,kk;
extern int16_t buff1sv[2][NUMBERSEQ1][NUMBERSDADC1];	/* Double DMA buffer for #1 */
int dmpct = 0;
#endif

long long *pfilter;
double dfilt;
extern u32 sdadcDebug0;
extern u32 sdadcDebug1;
extern u32 sdadcDebug2;

extern u32 sdadcDebug20;
extern u32 sdadcDebug21;
extern u32 sdadcDebug22;

extern u32 sdadcDebug30;
extern u32 sdadcDebug31;
extern u32 sdadcDebug32;

extern long long SDADCbug;
double dscale = (1<<23);
float fscale = (1<<23);

int filtct1=0;
int filtct2=0;
int filtct3=0;

double dpoly;
float fpoly;
float ffilt;
volatile int f3;
float ftmp;
double dtmp;

extern uint32_t recalibctr;	// Running count of re-calibrations 

#define STDDEVRUN	128	// Number of data pts in std dev "run"
int16_t stmp;
int32_t itmp;
int sctr = 0;
struct STDDEVSDADC stats[NUMBERSDADCS];
	for (i = 0; i < NUMBERSDADCS; i++) // Zero'em out
		sdadc_recalib_stddev_init(&stats[i]);

/* ====== Be forever occupied and productive ===================================================== */
while(1==1)
{
	/*SDADC3--selected port */
	pfilter = sdadc_filter_get(3,0);
	if (pfilter != NULL)
	{
f2 = DTWTIME;
		dfilt = *pfilter;
		ffilt = dfilt;

		dfilt = dfilt/dscale;
		ffilt = ffilt/fscale;
		
		dtmp = dfilt - ffilt;

//7		filtct1 += 1;if (filtct1 >= 8){xprintf(UXPRT,"\n\r"); filtct1 = 0;}
		fpoly = sdadc_poly_correct_test_f(3, ffilt);
f3 = DTWTIME;
//7		xprintf(UXPRT, "3F:%0.5f V:%0.5f %d\n\r",(double)ffilt,(double)fpoly, f3-f2);
f2 = DTWTIME;
		dpoly = sdadc_poly_correct_test(3, dfilt);
f3 = DTWTIME;
		
//7		xprintf(UXPRT, "3F:%0.5f V:%0.5f %5d X:%0.7f\n\r",dfilt,dpoly, f3-f2, dpoly-(double)fpoly);
	}
	/*SDADC2--selected port */
	pfilter = sdadc_filter_get(2,1);
	if (pfilter != NULL)
	{
		dfilt = *pfilter;
		dfilt = dfilt/dscale;
//		xprintf(UXPRT, "%10lld ",*pfilter/512);
//E		xprintf(UXPRT, "2 %10.1f ",dfilt);
//E		filtct2+= 1;if (filtct2 >= 1){xprintf(UXPRT,"\n\r"); filtct2 = 0;}
	}	
	/*SDADC1--selected port */
	pfilter = sdadc_filter_get(1,0);
	if (pfilter != NULL)
	{
		dfilt = *pfilter;
		dfilt = dfilt/dscale;
//		xprintf(UXPRT, "%10lld ",*pfilter/512);
//E		xprintf(UXPRT, "1 %10.1f ",dfilt);
//E		filtct3 += 1;if (filtct3 >= 1){xprintf(UXPRT,"\n\r"); filtct3 = 0;}
	}

	/* Handle any new re-calibrations and the resulting statistics thereof. */	
//	for (i = 0; i < NUMBERSDADCS; i++)
	i=2;
	{
		if (sdadcfix[i].var->recalib_n != 0)
		{ // Here we have a new calibration
			sdadcfix[i].var->recalib_n = 0;		// Reset run length
			itmp = SDADC_CONF0R(sdadcfix[i].sdbase); // Get 12b signed reading
			if ((itmp & 0x0800) != 0) itmp |= 0xfffff000; // Convert to signed int
			sdadc_recalib_stddev(&stats[i], itmp);	// Build stats
			if (stats[i].n >= 64)	// Time to compute and print?
			{ // Here, yes.  
				sctr = 0;
				dtmp = sqrt(stats[i].var); // Std dev given variance
				xprintf(UXPRT,"Z%d %6d %9.2f %6.3f\n\r",i+1,stats[i].n,(double)stats[i].mean,dtmp);
				sdadc_recalib_stddev_init(&stats[i]); // Re-init for next "run"
			}
		}
	}
	

	if ( ( (int)(DTWTIME)-(int)f0) < 0)
	{
//#define LEDSON
#ifdef LEDSON
		if (alt == 1)
		{
			LED_E_green_off;
			LED_S_red_on;
		}
		else
		{
			LED_E_green_on;
			LED_S_red_off;
		}
#endif
		f0 += 32000000;
		alt ^= ~1;

otmp[2] = SDADC3_CONF0R & 0x0fff;// Offset of SDADC3
if ((otmp[2] & 0x800) != 0) otmp[2] |= 0xfffff000;
otmp[1] = SDADC2_CONF0R & 0x0fff;// Offset of SDADC2
if ((otmp[1] & 0x800) != 0) otmp[1] |= 0xfffff000;
otmp[0] = SDADC1_CONF0R & 0x0fff;// Offset of SDADC1
if ((otmp[0] & 0x800) != 0) otmp[0] |= 0xfffff000;
xprintf(UXPRT,"T1: %d T2: %d T3: %d\t\tr:%5d O: %d %d %d\n\r",sdadcDebug2,sdadcDebug22,sdadcDebug32,recalibctr,otmp[0],otmp[1],otmp[2]);



#ifdef SDADCNOTSKIPRAWNUMBERS
#define OFFSET 32768

		/* Print a sample of the DMA array */
		xprintf(UXPRT,"a%2d",j1);
		for (i = 0; i < NUMBERSDADC1; i++)
		{
			xprintf(UXPRT,"%7d",buff1[sdadcfix[0].var->idx][j1][i]+OFFSET);
		}
		xprintf(UXPRT,"\n\r");

		xprintf(UXPRT,"b%2d",j1);
		for (i = 0; i < NUMBERSDADC2; i++)
		{
			xprintf(UXPRT,"%7d",buff2[sdadcfix[1].var->idx][0][i]+OFFSET);			
		}
		xprintf(UXPRT,"\n\r");

		xprintf(UXPRT,"c%2d", j1);
		for (i = 0; i < NUMBERSDADC3; i++)
		{
			xprintf(UXPRT,"%7d",buff3[sdadcfix[2].var->idx][0][i]+OFFSET);			
		}
		xprintf(UXPRT,"\n\r");
		j1 += 1; if (j1 >= NUMBERSEQ3) j1 = 0;

#endif
	}
//#define DISTRIBUTIONCHECK
#ifdef DISTRIBUTIONCHECK
	/* Raw readings for distribution check */
	if ((buff1sv_flag != 0) && (dmpct++ < 32))
	{
		if (buff1sv_flag != 1)
			xprintf(UXPRT,"\t$$$ %d\n\r",buff1sv_flag);
		buff1sv_flag = 0;
		for (jj = 0; jj < NUMBERSDADC1; jj++)
		jj = 0;
		{
	
			for (kk = 0; kk < 2; kk++)
			{
				imean = 0;
				for (ii = 0; ii < NUMBERSEQ1; ii++)
				{
					tmp = (buff1sv[kk][ii][jj] + 32768);
					imean += tmp;
				}
				imean = imean / (NUMBERSEQ1);
				xprintf(UXPRT,"MEAN: %d ###############\n\r",imean);
				for (ii = 0; ii < NUMBERSEQ1; ii++)
				{
					tmp = (buff1sv[kk][ii][jj] + 32768);
					xprintf(UXPRT,"%1d%2d%4d%7d%7d\n\r",kk,jj,ii,tmp, tmp - imean);
				}
			}
		}
	}
if (dmpct == 32) while(1==1);
#endif



#ifdef  SKIPPY
	t0 = DTWTIME;
	sdadc_request_calib();	// Stop conversions & do calibration
	t1 = DTWTIME;
	t2 = t1-t0;
	xprintf(UXPRT,"ticks: %d usec: %d half-flag: ticks %d usec %d\n\r",t2,t2/64,sdbugI1,sdbugI1/64);

	btmp = SDADC1_CONF0R & 0x0fff;
	xprintf (UXPRT,"Offset A: %5d ",btmp);
	btmp = SDADC2_CONF0R & 0x0fff;
	xprintf (UXPRT,"Offset B: %5d ",btmp);
	btmp = SDADC3_CONF0R & 0x0fff;
	xprintf (UXPRT,"Offset C: %5d\n\r",btmp);
#endif
	k ^= 1; // Alternate buffer halves
}

#ifdef CANLOOPSTUFF
/* --------------------- CAN setup ------------------------------------------------------------------- */
	/* Configure CAN driver RS pin: PC4 LQFP 33, Header P1|20, fo hi speed. */
	can_nxp_setRS(0,(volatile u32 *)GPIOC, 4); // (1st arg) 0 = high speed mode; not-zero = standby mode

	/* Setup CAN registers and initialize routine */
	pctl1 =  canwinch_setup_F3(&msginit1, 1);	// Number msg bufferblocks, CAN1

	/* Check if initialization was successful. */
	if (pctl1 == NULL)
	{
		xprintf(UXPRT,"CAN1 init failed: NULL ptr\n\r");
		panic_leds(6); while (1==1);	// Flash panic display with code 6
	}
	if (pctl1->ret < 0)
	{
		xprintf(UXPRT,"CAN init failed: return code = %d\n\r",pctl1->ret);
		panic_leds(6); while (1==1);	// Flash panic display with code 6
	}
	can_driver_enable_interrupts();	// Enable CAN interrupts

/* --------------------- Hardware is ready, so do program-specific startup ---------------------------- */
#define FLASHCOUNT 21000000;	// LED flash
u32	t_led = DTWTIME + FLASHCOUNT; // Set initial time

	/* Set modes for routines that receive and send CAN msgs */
	pctogateway.mode_link = MODE_LINK;
	pctogateway1.mode_link = MODE_LINK;

/* Test disable/enable global interrupts */
__asm__ volatile ("CPSID I");

__asm__ volatile ("CPSIE I");

t_cmdn = DTWTIME + 168000000; // Set initial time

/* --------------------- Endless Polling Loop ----------------------------------------------- */
	while (1==1)
	{
		/* Flash the LED's to amuse the hapless Op or signal the wizard programmer that the loop is running. */
		if (((int)(DTWTIME - t_led)) > 0) // Has the time expired?
		{ // Here, yes.
			toggle_4leds(); 	// Advance some LED pattern
			t_led += FLASHCOUNT; 	// Set next toggle time

		}

	}

#endif
	return 0;	
}

