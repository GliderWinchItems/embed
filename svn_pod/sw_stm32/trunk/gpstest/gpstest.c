/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : gpstest.c
* Hackeroo           : deh
* Date First Issued  : 08/09/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Test program for timer input capture with gps 1_pps & RTC alarm
*******************************************************************************/

/* NOTE: 
1.  Open minicom on the PC with 115200 baud and 8N1 and this routine

2.  This is a hack of 32KHztest.c on 07/05/2011.  It uses 32KHz stuff for delays

3.  This routine measures the 32 KHz osc frequency by comparing the 1 PPS pulse from
    the gps with the 32 Khz osc.  The "anti-tamper" is configured to present the 
    Scef/Alarm (or RTC/16) of 32 KHz osc, and the POD board has this pin connected
    to PE9 which is TIM1 CH1* (or TIM8 CH1).  The GPS 1 PPS is connected to TIM3 CH1*.
    With both of these timers configured for input capture the time difference between
    the two rising edges can be computed with a resolution of the APB1 bus.
*/

/*
Peripherals of interest in this routine driven by bus clocks--
SYSCLK:
  AHB: (this is driven by SYSCLK)
    APB1: (max 36 MHz)
      TIM 2-7 (if APB1 prescale = 0, 1x; else x2)
      USART4,5
      USART2,3
      BKP
      RTC
    APB2: 
      TIM 1,8 (if APB2 prescale = 0, 1x; else x2)
      ADC1-3 (max 14 MHz)      
      USART1
      AFIO
*/

#include <math.h>
#include <string.h>

#include "libopenstm32/rcc.h"
#include "libopenstm32/gpio.h"
#include "libopenstm32/usart.h"
#include "libopenstm32/bkp.h"	// Used for #defines of backup register addresses
#include "libopenstm32/timer.h"	// 


/* USART/UART */
#include "libusartstm32/usartallproto.h"

/* mostly startup related */
#include "libopenstm32/systick.h"
#include "libmiscstm32/systick1.h"// System tick counter handling
#include "libmiscstm32/printf.h"// Tiny printf
#include "libmiscstm32/clockspecifysetup.h"// Setup clocks

/* hardware defines */
#include "libopenstm32/bkp.h"	// #defines for addresses of backup registers

/* ../devices library */
#include "PODpinconfig.h"	// gpio configuration for board
#include "32KHz.h"		// RTC & BKP routines
#include "pwrctl.h"		// Low power routines
#include "rtctimers.h"		// RTC countdown timers
#include "adcpod.h"		// ADC1 routines
#include "calibration.h"	// calibrations for various devices on pod board
#include "gps_time_convert.h"	// GPS extract time 
#include "gps_1pps.h"		// Timer routines for this app


void toggle_PA0_led (void);
void output_rtc_cnt (unsigned int uiT);

/* STANDBY Stuff */
/* When 'x' is typed in an <enter> hit it goes into STANDBY mode for the following defined time period */
#define STANDBYTIME	100	// Time for next wakeup in tenth seconds
#define SLEEPDEEPTICKS	(ALR_INCREMENT*STANDBYTIME)/10	// TR_CLK tick count added to current RTC CNT counter for wakeup



/* ======== Setup clocks for the highest speeds (can't use ADC) ================= */

/* 'struct CLOCKS clocks' is used to setup the clock source, PLL, dividers, and bus clocks 
See P 84 of Ref Manual for a useful diagram.
../lib/libmiscstm32/clockspecifysetup.h has the 'enum' values that may help for making mistakes(!) */
/* NOTE: Bus for ADC (APB2) must not exceed 14 MHz */
struct CLOCKS clocks = { \
HSOSELECT_HSE_XTAL,	/* Select high speed osc 			*/ \
PLLMUL_6X,		/* Multiplier PLL: 0 = not used 		*/ \
1,			/* Source for PLLMUL: 0 = HSI, 1 = PLLXTPRE (1 bit predivider)	*/ \
0,			/* PLLXTPRE source: 0 = HSE, 1 = HSE/2 (1 bit predivider on/off)	*/ \
APBX_4,			/* APB1 clock = SYSCLK divided by 1,2,4,8,16; freq <= 36 MHz */ \
APBX_4,			/* APB2 prescalar code = SYSCLK divided by 1,2,4,8,16; freq <= 72 MHz */ \
AHB_1,			/* AHB prescalar code: SYSCLK/[2,4,8,16,32,64,128,256,512] (drives APB1,2) */ \
8000000			/* Oscillator source frequency, e.g. 8000000 for an 8 MHz xtal on the external osc. */ \
};

/* gps_time_convert.c: Extraction of time from gps lines */
struct TIMESTAMPG strTS1;	// GPS time (See '..devices/gps_time_convert.h')
extern char cGPS[10];



/* Default calibrations for various devices */
extern struct CALBLOCK strDefaultCalib;		// Default calibration
void output_calibrated_adc (unsigned int uiT);		// Routine (near bottom) for outputting calibrated

/* RTC timers - tick count is the RTCCLK (32768 Hz) divided by the prescalar (16) (2048 ticks make one second) */
extern void 	(*rtc_timerdone_ptr)(void);		// Address of function to call at end of 'rtctimers_countdown'


/* RTC callback routines (for test) */
void rtc_call_timer0(void);
void rtc_call_timer1(void);
void rtc_call_timer2(void);

/* RTC system counter (mirrors RTC_CNT (32b TR_CLK rtc counter, see p 449) */
extern unsigned int uiRTCsystemcounter;	// When system comes out of RESET this count is initialized with RTC_CNT

/* See 32KHz.c */
extern unsigned int RTC_debug2;	// Type of reset "we think we had"

/* RTC registers: PRL, CNT, ALR initial values (see ../devices/32KHz.h, also p 448 Ref Manual) (see ../devices/32KHz.c) */
struct RTCREG strRtc_reg_init = {PRL_DIVIDER,0,ALR_INCREMENT}; // SECF = 1/2 sec, CNT
struct RTCREG 	strRtc_reg_read;	// Readback of registers
unsigned int	 uiRCC_CSR_init;	// RCC_CSR saved at beginning of RTC initialization (see 32KHz.c)
extern void 	(*rtc_secf_ptr)(void);	// Address of function to call during RTC_IRQHandler of SECF (Seconds flag)
extern char	cResetFlag;		// Out of a reset: 1 = 32 KHz osc was not setup; 2 = osc setup OK, backup domain was powered
extern unsigned int	uiSecondsFlag;	// 1 second tick: 0 = not ready, + = seconds count (see 32KHz.c)
unsigned int	uiSecondsFlagPrev;	// Previous for checking ticks

		
/* The following variables are used by peripherals in their 'init' routines to set dividers 	*/
/* We might use them here just to display how the clock was set up */
extern unsigned int	hclk_freq;  	/* 	SYSCLKX/HPREDIV	 	E.g. 72000000 	*/
extern unsigned int	pclk1_freq;  	/*	SYSCLKX/PCLK1DIV	E.g. 72000000 	*/
extern unsigned int	pclk2_freq;	/*	SYSCLKX/PCLK2DIV	E.g. 36000000 	*/
extern unsigned int	sysclk_freq;	/* 	SYSCLK freq		E.g. 72000000	*/

/* This if for test sprintf */
char vv[180];	// sprintf buffer


/* Timer tinkering variables */
// Input capture extended counts (see gps_1pps.c)
extern volatile union TIMCAPTURE64	strTim1;	// 64 bit extended TIM1 CH1 capture
extern volatile union TIMCAPTURE64	strTim3;	// 64 bit extended TIM3 CH1 capture

extern volatile unsigned short 		usTim1ch1_Flag;	// TIM1 Input capture flag counter
extern volatile unsigned short 		usTim3ch1_Flag;	// TIM3 Input capture flag counter
unsigned short  usTim1ch1_FlagPrev;	// TIM1 Input capture flag counter previous count
unsigned short  usTim2ch2_FlagPrev;	// TIM2 Input capture flag counter previous count
unsigned short  usTim3ch1_FlagPrev;	// TIM3 Input capture flag counter previous count

struct TIMCAPTRET32 strTIM1ic;		// This holds the return from the call for getting IC time
struct TIMCAPTRET32 strTIM2ic;
struct TIMCAPTRET32 strTIM3ic;

extern volatile unsigned int nTim1Debug1;
extern volatile unsigned int nTim1Debug2;

unsigned int uiTim1cnt,uiTim1cntPrev;
unsigned int uiTim2cnt,uiTim2cntPrev;
unsigned int uiTim3cnt,uiTim3cntPrev;
unsigned int ui1, ui2, ui3;


unsigned short usTim1cnt,usTim1cntPrev;
unsigned short usTim2cnt,usTim2cntPrev;
unsigned short usTim3cnt,usTim3cntPrev;
unsigned short us1, us2, us3;

unsigned int uisystick;
unsigned int uisystickPrev;


unsigned int TIM2cntPrev;
int TIM2cntAccum;
int first;

/* PE9 checkout */
unsigned int gpio_b9;
unsigned int gpio_b9_ctr;
unsigned int gpio_tim0,gpio_tim0Prev;
unsigned int gpio_tim1;

/* For checkingout intput capture */
unsigned int uiIC1, uiIC1Prev, uiIC1ctr, uiIC1diff;
unsigned int uiIC2, uiIC2Prev, uiIC2ctr, uiIC2diff;
unsigned int uiIC3, uiIC3Prev, uiIC3ctr, uiIC3diff;
unsigned int uiDiffCt1,uiDiffCt2;

extern unsigned int nTim1Debug0;

/*-------------------------------------------------------------------------------*/
/* This is for the tiny printf */
// Note: the compiler will give a warning about conflicting types
// for the built in function 'putc'.
void putc ( void* p, char c)
	{
		p=p;	// Get rid of the unused variable compiler warning
		USART1_txint_putc(c);
	}
/* LED identification

|-number on pc board below LEDs
|   |- color
v vvvvvv  macro
3 green   
4 red
5 green
6 yellow
*/

/* ****************** POD *************************************
Turn the LEDs on in sequence, then turn them back off 
***************************************************************/
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
//	lednum += 1;		// Step through all four LEDs (comment out for single LED)
	if (lednum > LED6) lednum = LED3;
}
/* ****************** Olimex *********************************/
/* Stupid routine for toggling the gpio pin for the LED	     */
/* ***********************************************************/
#define LEDBIT	12
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
/* ***********************************************************/
/* Setup the gpio for the LED on the Olimex stm32 P103 board */
/* ***********************************************************/
void gpio_setup(void)
{
	/* Setup GPIO pin for LED (PC12) (See Ref manual, page 157) */
	// 'CRH is high register for bits 8 - 15; bit 12 is therefore the 4th CNF/MODE position (hence 4*4)
	GPIO_CRH(GPIOC) &= ~((0x000f ) << (4*4));	// Clear CNF reset bit 01 = Floating input (reset state)
	GPIO_CRH(GPIOC) |=  (( (GPIO_CNF_OUTPUT_PUSHPULL<<2) | (GPIO_MODE_OUTPUT_2_MHZ) ) << (4*4));
	
}
/* *******************************************************************************/
/* RTC, Seconds, interrupt routine comes here from isr: RTC_IRQHandler (32KHz.c) */
/* *******************************************************************************/
unsigned short usTick;		// Counter of RTCCLK (32768 Hz)divided by prescalar (PRL)
unsigned char ucTflag;		// Flag to mainline for 1/2 sec tick

void RTC_secf_stuff(void)
{
	usTick += 1;		// 32768 Hz divided by 16
	if (usTick > 1024)	// Count 1/2 seconds worth then toggle
	{
		usTick = 0;	
		ucTflag = 1;	// Signal mainline program
	}
	return;
}
/***********************************************************************************************************/
/* And now for the main routine */
/***********************************************************************************************************/
int main(void)
{
	/* ADC related */
	unsigned int 	uiADCt0,uiADCt0b;	// Time delay ending count for ADC regulator startup
	unsigned int 	uiADCt1,uiADCt1b;	// Time delay start & end counts for CR2_ADON (adc power up)
	unsigned int 	uiADCt2;		// Time delay start & end counts for calibration register reset
	unsigned int 	uiADCt3;		// Time delay start & end counts for calibration 
	unsigned int 	uiADCt4;		// Time delay start & end counts for battery switches
	unsigned int	uiTopCellCal;
	unsigned int	uiBotCellCal;
	unsigned int	uiTopCellDif;

	/* USART related */
	struct USARTLB strlb;	// Holds the return from 'getlineboth' of char count & pointer

	
	int i;		// Nice FORTRAN variables
	u16 temp;
//	char* p;		// Temp getline pointer
	u32 ui32;
	u32 uiGPSrtc;
//	int ggi;

	unsigned int uiF1,uiF2;
 
/* ---------- Initializations ------------------------------------------------------ */
	clockspecifysetup(&clocks);	// Get the system clock and bus clocks running

	/* The following is necessary for turning off the EWUP bit so that PA0 can be used as 
           an output pin to drive the LED on the external box.  The pin is used for both Wake-up
           (WKUP) and gpio */
	RCC_APB1ENR |= (RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN);
	PWR_CSR &= ~(1<<8);		// Disable EWUP so pin can be used as gpio (p 71)

	PODgpiopins_default();	// Set gpio port register bits for low power
	PODgpiopins_Config();	// Now, configure pins


	gpio_setup();		// Olimex gpio setup for LED

	MAX3232SW_on		// Turn on RS-232 level converter (if doesn't work--no RS-232 chars seen)

	init_printf(0,putc);	// This one-time initialization is needed by the tiny printf routine

	SYSTICK_init(0);	// Set SYSTICK for interrupting and to max count (24 bit counter)
	/* Initialize usart 
	USARTx_rxinttxint_init(...,...,...,...);
	Receive:  rxint	rx into line buffers
	Transmit: txint	tx with line buffers
	ARGS: 
		baud rate, e.g. 9600, 38400, 57600, 115200, 230400, 460800, 921600
		rx line buffer size, (long enough for the longest line)
		number of rx line buffers, (must be > 1)
		tx line buffer size, (long enough for the longest line)
		number of tx line buffers, (must be > 1)
	*/
	/* USART1 is TX/RX comm with the PC */
	// Initialize USART and setup control blocks and pointer
	if ( (temp = USART1_rxinttxint_initRTC(115200,96,2,48,4)) != 0 )
	{ // Here, something failed 
		return temp;
	}
	/* Remap: but code 0 = no remapping, so this is only for demonstration */
	if ( (temp = USARTremap(USART1,0)) != 0 )
	{ // Here, something failed, so hang and use JTAG to find location
		while (1==1);
	}

	/* UART4 (USART3 remapped) is RX-only comm with the GPS */
	// Initialize USART and setup control blocks and pointer
	if ( (temp = UART4_rxinttxint_initRTC(4800,96,2,48,4)) != 0 )
//	if ( (temp = USART3_rxinttxint_initRTC(4800,96,2,48,4)) != 0 )
	{ // Here, something failed, so hang and use JTAG to find location 
		return temp;
	}
	/* Remap: Partial remap = 1, (p 166) */
//	if ( (temp = USARTremap(USART3,0x01)) != 0 )	// Remap to PC 10,11
//	{ // Here, something failed, so hang and use JTAG to find location
//		while (1==1);
//	}

	/* Announce who we are */
	USART1_txint_puts("\n\rgpstest 08-06-2011 r0\n\r");  USART1_txint_send();// Start the line buffer sending

	/* Display the bus speeds we have set up */
	printf ("hclk %u pclk1 %u pclk2 %u sysclk %u\n\r",hclk_freq,pclk1_freq,pclk2_freq,sysclk_freq);USART1_txint_send();

	/* See what we have in the RTC registers before we do anything with them */	
	RTC_reg_read(&strRtc_reg_read);		// Read-back of register settings
	printf ("RTC reg before   : PRL 0x%08x, CNT 0x%08x, ALR 0x%08x\n\r",strRtc_reg_read.prl,strRtc_reg_read.cnt,strRtc_reg_read.alr);	USART1_txint_send();

	/* Setup addresses for additional RTC (Secf) interrupt handling (under interrupt)--
		RTC_ISRHandler (../devices/32KHz.c) goes to 'rtctimers_countdown' (../devices/rtctimers.c).
		'rtctimers_countdown', then goes to 'RTC_secf_stuff' (in this routine).  The consecutive 'returns'
		then unwind back to the RTC ISR routine and return from interrupt.  */
	rtc_secf_ptr  = &rtctimers_countdown;	// RTC_ISRHandler goes to 'rtctimers_countdown' for further work
	rtc_timerdone_ptr = &RTC_secf_stuff;	// 'rtctimer_countdown' goes to this address for further work

	/* Setup the RTC osc and bus interface */	
	uiRCC_CSR_init = Reset_and_RTC_init();	// Save register settings (mostly for debugging)

	/* Show what the power up initialization saw as the cause of the reset */
	ui32 = cResetFlag;	// tiny printf only does unsigned ints
	printf ("debug2: 0x%08x  cFlag: %02x\n\r", RTC_debug2,ui32);	USART1_txint_send();
	printf ("RCC_CSR: 0x%08x\n\r",uiRCC_CSR_init);	USART1_txint_send();	// RCC_CSR before routine did anything

	/* See what we have in the RTC registers after 'init routine  */	
	RTC_reg_read(&strRtc_reg_read);		// Read-back of register settings
	printf ("RTC reg after init: PRL 0x%08x, CNT 0x%08x, ALR 0x%08x\n\r",strRtc_reg_read.prl,strRtc_reg_read.cnt,strRtc_reg_read.alr);	USART1_txint_send();

	if (cResetFlag == 1)	// After reset was the 32KHz osc up & running? (in 32KHz.c see Reset_and_RTC_init)
	{ // Here, no.  It must have been a power down/up reset, so we need to re-establish the back domain
		/* Load the RTC registers */
		RTC_reg_load (&strRtc_reg_init);	// Setup RTC registers
		USART1_txint_puts("Re-establishing backup domain\n\r");	USART1_txint_send();	// Something for the hapless op
		BKP_DR1 = 0;				// Extended seconds counter
	}
	else
	{
		/* Compute and set the next alarm register */
		strRtc_reg_read.alr =	((strRtc_reg_read.cnt & ~((1<<11)-1)) +(1<<11));
		RTC_reg_load_alr(strRtc_reg_read.alr);
		USART1_txint_puts("Backup domain was good\n\r");	USART1_txint_send();	// Something for the hapless op
	}

	/* This counter mirrors RTC_CNT register.  Updated by RTC Secf interrupt (32768Hz/16) */
	uiRTCsystemcounter = RTC_reg_read_cnt();	// Read current RTC time counter

	/* Compute elapsed seconds */
	ui32 = ((strRtc_reg_read.cnt>>ALR_INC_ORDER) | (BKP_DR1>>PRL_DIVIDE_ORDER));	// Combine high order bits with CNT register
	printf ("Elapsed seconds: %9u\n\r",ui32);

	/* Check if the RTC register setup OK */	
	RTC_reg_read(&strRtc_reg_read);		// Read-back of register settings
	printf ("RTC reg after lod:PRL 0x%08x, CNT 0x%08x, ALR 0x%08x\n\r",strRtc_reg_read.prl,strRtc_reg_read.cnt,strRtc_reg_read.alr);	USART1_txint_send();

	/* Test rtctimers: countdown with looping wait */	
#define TIMERWAIT	1	// Seconds to wait
	printf("====== Timer start %u seconds, loop on timer count\n\r",TIMERWAIT);	USART1_txint_send();
	for (i = 0; i < NUMBERRTCTIMERS; i++)	// Set each timer to 1 second more than the previous
	{
		nTimercounter[i] = 2048*(TIMERWAIT+i);	// Set tick count
	}
	for (i = 0; i < NUMBERRTCTIMERS; i++)
	{
		while (nTimercounter[i] != 0);		// Wait for timer to countdown to zero
		printf("====== Timer %2u end ======\n\r",i);	USART1_txint_send();
	}

	/* Wait for USART to complete so that we don't have USART interrupt messing up the timing in the following */
	nTimercounter[0] = 2000;			// Each tick is 1/2048 second
	while (nTimercounter[0] != 0);		// Wait for timer to countdown to zero

/* --------------------- ADC measurement startup ---------------------------------------------------- */
	uiADCt4 = adc_battery_sws_turn_on();		// Turn on switches to connect resistor dividers to battery
	uiADCt0 = adc_regulator_turn_on();		// Turn on ADC 3.2v regulator and get time (systicks)
	while ( (uiADCt0b = SYSTICK_getcount32()) > uiADCt0 );	// Wait for regulator voltage to settle
	uiADCt1 = adc_init((volatile struct ADCDR*)&strADC1dr);	// Initialize the ADC for DMA reading of all POD adc pins
	while ( (uiADCt1b = SYSTICK_getcount32()) > uiADCt1 );	// Wait for ADC to power up (1st write of CR2_ADON bit)
	uiADCt2 = adc_start_cal_register_reset();	// Start reset of calibration registers
	uiADCt3 = adc_start_calibration();		// Start calibration 
	while ( (SYSTICK_getcount32()) > uiADCt4 );	// Wait for battery voltage divider capacitors to charge

	/* Now let's look at the timing for this mess */
	USART1_txint_puts("ADC setup timing\n\r");		USART1_txint_send();
	printf ("Reg  settle: %8u \n\r",uiADCt0-uiADCt0b);	USART1_txint_send();
	printf ("Batt switch: %8u \n\r",uiADCt4-SYSTICK_getcount32());	USART1_txint_send();

printf ("struct %08x \n\r",(unsigned int)&strADC1dr);	USART1_txint_send();

	printf ("Bot %u Top %u \n\r",BOTCELLMVNOM,TOPCELLMVNOM);	USART1_txint_send();

	/* Start ADC conversions */
	uiADCt0 = adc_start_conversion((volatile struct ADCDR*)&strADC1dr);	// Start conversion into struct via DMA

/* --------------- Setup timers and RTC to feed TIM1 CH1* and gps 1 PPS feeds TIM3 CH1* --------------*/
//	ENCODERGPSPWR_on;	// Turn on 5v power supply to GPS

	GPS_1pps_init();	// Setup of the timers and pins for TIM1CH1* (* = remapped) driven from RTC
	Tim2_pod_init();	// Setup TIM2CH1  driven by 1_PPS via PA1 (spare RJ-45 pin)
//	Tim3_pod_init();	// Setup TIM3CH1* driven by Encoder_2B header pin with flying lead to 1_PPS or RTC alarm

/* ---------------------Endless loop -------------------------------------------------------------------------------- */
	i = 0;
	while (1==1)
	{
		/* ----------------------- Echo back GPS lines ------------------------------------------------------ */
		/* The following gets a pointer and count to a buffer and steps the pointer in the routine to the next buffer
		   if the line in the buffer is complete. */
		strlb = UART4_rxint_getlineboth();	// Get both char count and pointer
		if (strlb.ct > 0)
		{	
			USART1_txint_puts(strlb.p);	// Echo back the line just received
			USART1_txint_puts ("\n");	// Add line feed to make things look nice
		}


		/* Check for a valid time/fix line and extract time and RTC counter at EOL.  */
		if ( (uiGPSrtc = gps_time_stamp(&strTS1, strlb) ) > 0 )
		{ // Here we got a valid time/fix $GPRMC record and extracted the time fields */
//			USART1_txint_puts(strlb.p);	// Echo back the line just received
//			USART1_txint_puts ("\n");	// Add line feed to make things look nice
			USART1_txint_send();		// Start the line buffer sending
			printf ("%14u%10u %4d %9d", uiGPSrtc, strTIM2ic.cnt,TIM2cntAccum, uiGPSrtc-strTIM2ic.cnt); USART1_txint_send();
			if (first < 2) first += 1; else	TIM2cntAccum += strTIM2ic.cnt-TIM2cntPrev-2048; // Sum tick drop/adds
			TIM2cntPrev = strTIM2ic.cnt;	// This to get number RTC ticks between GPS input captures.
			output_rtc_cnt (strTS1.cnt);	// Setup rtc_cnt as whole.fraction
			printf ("%6u %04u ",strTS1.cnt/2048, strTS1.cnt-(strTS1.cnt/2048)*2048 );
		/* GPS time as MM/DD/YY HH:MM:SS */
			printf ("%c%c/%c%c/%c%c %c%c:%c%c:%c%c ",strTS1.g[8],strTS1.g[9],strTS1.g[10],strTS1.g[11],strTS1.g[6],strTS1.g[7],                          strTS1.g[0],strTS1.g[1],strTS1.g[2],strTS1.g[3],strTS1.g[4],strTS1.g[5]); USART1_txint_send();
			USART1_txint_puts("#\n\r");	
			USART1_txint_puts("\n\r");	
			USART1_txint_send();	// Show that we have valid time
		}

		/* ------------- GPS 1 PPS or RTC drives input capture to TIM3 CH1* -------------------------------- */
//		strTIM3ic = Tim3_inputcapture_ui();		// Get latest GPS IC time & flag counter
//		uiF3 = (strTIM3ic.flg - usTim3ch1_FlagPrev);	// Has the flag changed?
//		if (uiF3 != 0)
//		{
//			if (uiF3 > 1)
//			{
//				printf("Flag3 %5u\n\r",uiF3);	USART1_txint_send();
//			}
//			usTim3ch1_FlagPrev = strTIM3ic.flg;		// Update previous count
//			uiIC3ctr++;
//			if (uiIC3ctr >= 1)
//			{
//				uiIC3ctr = 0;
//				uiIC3 = strTIM3ic.ic;			// Retrieve IC as 32 bits
//				uiIC3diff = uiIC3 - uiIC3Prev;		// Time between captures
//				uiIC3Prev = uiIC3;			// 
//				printf ("T3: %10u %9u \n\r",uiIC3, uiIC3diff); 	USART1_txint_send();
//			}
//		}

		/* ------------- GPS 1 PPS drives input capture to TIM2 CH1* -------------------------------- */
		strTIM2ic = Tim2_inputcapture_ui();		// Get latest GPS IC time & flag counter
		uiF2 = (strTIM2ic.flg - usTim2ch2_FlagPrev);	// Has the flag changed?
		if (uiF2 != 0)
		{
			if (uiF2 > 1)	// Check for skipped flags
			{ 
				printf("Flag2 %5u\n\r",uiF2);	USART1_txint_send();
			}
			uiIC2ctr += uiF2;			// Count IC flag changes
			usTim2ch2_FlagPrev = strTIM2ic.flg;	// Update previous count
			uiIC2 = strTIM2ic.ic;			// Retrieve IC as 32 bits
			uiIC2diff = uiIC2 - uiIC2Prev;		// Time between captures
			uiIC2Prev = uiIC2;			// Update previous
			if (uiIC2diff == 0) uiDiffCt2 += 1;
			printf ("T2: %10u %9u %6u %4u \n\r",uiIC2, uiIC2diff,strTIM2ic.flg, uiDiffCt2);
 			USART1_txint_send();
		}


		/* -------- RTC ISR updates the ALR register to give one second input captures on TIM1 CH1* --------- */
		strTIM1ic = Tim1_inputcapture_ui();		// Get latest RTC IC time
		uiF1 = (strTIM1ic.flg - usTim1ch1_FlagPrev);	// Has the flag changed?
		if ( uiF1 != 0)			// Has the flag changed?
		{ // Yes.
			if (uiF1 > 1)		// Did we get behind?
			{ // Here, yes.  Let the hapless Op know about it.
				printf("Flag1 %5u\n\r",uiF1);	USART1_txint_send();
			}
			usTim1ch1_FlagPrev = strTIM1ic.flg;		// Update previous count
			uiIC1ctr += uiF1;		// Count number of IC flags
//			if (uiIC1ctr >= 1)	// Have counted enough interrupts?
			{ // Here, yes.
//				uiIC1ctr = 0;				// Reset flag counter
				uiIC1 = strTIM1ic.ic;			// Copy from struct is easier hack
				uiIC1diff = uiIC1 - uiIC1Prev;		// Time between captures
				uiIC1Prev = uiIC1;			// Update previous
				toggle_4leds();				// Nice to have green LED flashing
toggle_PA0_led();
//				printf ("%9d ",uiIC1diff); 		// Ticks between IC interrupts for RTC
//				printf ("%9d ",uiIC2diff); 		// Ticks between IC interrupts for GPS
//				printf ("%9d ",uiIC1-uiIC2);		// Difference in "positions" between RTC & 1_PPS
//				printf (" %9d ",(uiIC2diff-uiIC1diff) );// Difference between the two one seconds
//				printf (" %4u %4u ",uiIC1ctr, uiIC2ctr); 
				if (uiIC1diff == 0) uiDiffCt1 += 1;
				printf ("T1: %10u %9u %6u %4u\n\r",uiIC1, uiIC1diff,strTIM1ic.flg, uiDiffCt1);// 	USART1_txint_send();

				USART1_txint_puts ("\n\r"); USART1_txint_send();
			}
		}


		/* Echo incoming chars back to the sender */
		strlb = USART1_rxint_getlineboth();	// Get both char count and pointer
		/* Check if a line is ready.  Either 'if' could be used */
		if (strlb.p > (char*)0)			// Check if we have a completed line
		{ // Here we have a pointer to the line and a char count
			if (*strlb.p == 's') break;		// Quit when x is the first char of the line
			if (*strlb.p == 'h') while(1==1);	// Hang
			if (*strlb.p == 'b')			// Battery
			{
				USART1_txint_puts ("\r\n");	USART1_txint_send();
				USART1_txint_puts(strlb.p);	USART1_txint_send();	// Echo back the line just received
				USART1_txint_puts ("\n");	// Add line feed to make things look nice
				USART1_txint_send();		// Start the line buffer sending

			/* Output current battery voltages Top, Bottom cell, and difference, which is the Top cell */
				uiBotCellCal = ((strDefaultCalib.adcbot * strADC1dr.in[0][1]) >> 16 );
				uiTopCellCal = ((strDefaultCalib.adctop * strADC1dr.in[0][2]) >> 16 );
				uiTopCellDif = uiTopCellCal - uiBotCellCal;
				output_calibrated_adc(  uiBotCellCal );	// Top of battery
				output_calibrated_adc(  uiTopCellCal );	// Bottom cell
				output_calibrated_adc(  uiTopCellDif );	// Top cell
				USART1_txint_puts ("\n\r"); USART1_txint_send();
			}
		}
	}

	Powerdown_to_standby( SLEEPDEEPTICKS  );	// Set ALR wakeup, setup STANDBY mode. (..devices/pwrctl.c)
	USART1_txint_puts("WFE should not come here\n\r");	USART1_txint_send();	// Something for the hapless op
	while (1==1);	// Should not come here
	return 0;	

}
/*****************************************************************************************
'rtctimer_countdown' will call these when the timer times out
*****************************************************************************************/
void rtc_call_timer0(void)
{
	printf("====== Timer 0 end ======\n\r");	USART1_txint_send();	
	return;
}
void rtc_call_timer1(void)
{
	printf("====== Timer 1 end ======\n\r");	USART1_txint_send();	
	return;
}
void rtc_call_timer2(void)
{
	printf("====== Timer 2 end ======\n\r");	USART1_txint_send();	
	return;
}
/*****************************************************************************************
Setup an int for a floating pt output
*****************************************************************************************/
void output_calibrated_adc (unsigned int uiT)
{
	unsigned int uiX,uiZ;
	double dT = uiT;

	uiX = uiT /1000;	// uiX = whole 
	dT = uiT;		// Float the integer
	dT = (dT/1000) - uiX;	// dT = fraction
	uiZ = dT * 1000;	// Scale by number of decimals
	printf (" %8u.%03u ",uiX,uiZ);	// Setup the output
	return;
}
/*****************************************************************************************
Setup an RTC_CNT for a floating pt output
*****************************************************************************************/
void output_rtc_cnt (unsigned int uiT)
{
	unsigned int uiX,uiZ;
	double dT = uiT;

	uiX = uiT /2048;	// uiX = whole 
	dT = uiT;		// Float the integer
	dT = (dT/2048) - uiX;	// dT = fraction
	uiZ = dT * 10000;	// Scale by number of decimals
	printf (" %8u.%04u ",uiX,uiZ);	// Setup the output
	return;
}
/* ****************** PA0/WKUP *******************************/
/* Stupid routine for toggling the PA0 gpio pin for the LED  */
/* ***********************************************************/
static char cPA0;
void toggle_PA0_led (void)
{
//	if ((GPIO_ODR(GPIOA) & 0x01) == 0)
	if (cPA0 == 0)
	{
		GPIO_BSRR(GPIOA) = 0x01;	// Set bit
		cPA0 = 0x01;
	}
	else
	{
		GPIO_BRR(GPIOA) = 0x01;	// Reset bit
		cPA0 = 0x00;
	}
}

