/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : linear_batt_chgr.c
* Person	     : deh
* Date First Issued  : 01-20-2012
* Board              : STM32F103VxT6_pod_mm, HC11 batchgr, hack board
* Description        : Linear charge controller for BOX
*******************************************************************************/

/* NOTE: 

- This is a gross hack of 'adctest.c', hence don't get exciet about a lot of superfluous code.

- Open minicom on the PC with 115200 baud and 8N1 and this routine

- The hardware lashes together the STM32 POD board with the HC11 batt charger board
  along with glue-parts on a "hack-board" to control the charging of the the two cell
  Li-Po battery.  

 Relevant hardware files--
  . /MD0/home/deh/eagle/STM32F102VxT6_pod_mm/VET6_POD.sch,brd
  . /MD0/home/deh/eagle/LiPo_linear/hack,sch,brd
  . /MD0/home/deh/eagle/LiPo_linear/PODhack_board_pins.txt
  . /windows/F/cad/Eagle4_0/Eagle/Eagle/BatChg/BChg3/h11kc.sch,brd

  In the event of low battery voltage it removes the power drain on
  the battery.  If the charger (external wall-wart) is not powered, then the GPS and
  other BOX circuits will continue to drain the POD battery.  By sensing low
  battery and no charging input the STM32 switches the BOX power off.

- This routine uses the STM32 to adjust the current to:
  . Measure the top and bottom cell voltages via P/N switches
  . Measure the top and charge current via hack-board resistors
  . Measures the charging source voltage to detect power off
  . Controls the top of battery (HC11 board) P-FET current for charging
  . Controls the bottom cell charge (HC11 board) P-FET on/off
  . Controls the bottom cell discharge (on-board) N-FET on/off

Strategy for charging:

The bottom cell is in series with the top cell so the charging current charges
both cells.  To maintain balance, the bottom cell can be switched to a current from
the charger which increases the bottom cell charging current a small amount, or
switched to a resistor across the cell which discharges it a small amount.  When
the top cell reaches 4.18 v the charge is stopped.

If the battery voltage is below 2.96v a low level current, e.g. 20 ma, is applied
until the voltage reaches 3.5v, at which time the full charge current is then
applied.

PIN usage: see '../eagle/LiPo_linear/PODhack_board_pins.txt'.

Usage of ADC's with this board hack--
Port- ADC  - POD function -> charger function
PA3 - IN3  - Thermister -> Thermistor
PB0 - IN8  - Bottom Cell V
PB1 - IN9  - 7v+ (Top Cell)
PC0 - IN10 - Accel X -> Raw DC source voltage
PC1 - IN11 - Accel Y -> Charge current sense R, V1
PC2 - IN12 - Accel Z -> Charge current sense R, V2

Basic formula (scaling not shown)--
Bottom cell voltage = IN8
Top cell voltage    = (IN9 - IN8)
Charge current      = (IN11 - IN12)/Rsense

ON/OFF--
Bottom cell shunt = PE2 (EXT LED) high
Bottom cell charge/top cell shunt = PA2 (BEEPER FET) high

Timer--
Current drive = PWM using Encoder_1 chan B:TIM4_CH1*

Strategy--

Charge current/voltage:
If both cell voltages are below the cutoff (4.15v), increment the PWM slowly
until the max charge current is reached.  If either cell voltage is greater than 4.15v
then decrement the PWM.

Over-discharge recovery:
If the voltage of either cell is less than 2.95v, set the current rate at about 20 ma.
If the voltage is greater, set the current rate at 200 ma(?, or some reasonable value).

Cell balance:
If the upper cell voltage is greater than the lower cell voltage by more than some small amount
(to be determined) turn on the switch to charge the shunt the upper cell (and turn off the lower
cell shunt).  If the lower cell is greater by some small amount turn on the lower cell shunt 
(turn off the upper cell shunt).

*/



#include <math.h>
#include <string.h>

#include "libopenstm32/rcc.h"
#include "libopenstm32/gpio.h"
#include "libopenstm32/usart.h"
#include "libopenstm32/bkp.h"	// Used for #defines of backup register addresses

#include "libopenstm32/systick.h"


#include "libusartstm32/usartallproto.h"
#include "libmiscstm32/systick1.h"

#include "adctherm_cal.h"	// Conversion of thermistor adc reading to temp (deg C * 100)


#include "libmiscstm32/printf.h"// Tiny printf
#include "libmiscstm32/clockspecifysetup.h"

#include "PODpinconfig.h"	// gpio configuration for board

#include "spi1ad7799.h"		// spi1 routines 
#include "ad7799_comm.h"	// ad7799 routines (which use spi1 routines)
#include "32KHz.h"		// RTC & BKP routines
#include "pwrctl.h"		// Low power routines
#include "rtctimers.h"		// RTC countdown timers
#include "adcpod.h"		// ADC1 routines
#include "calibration.h"	// calibrations for various devices on pod board

#include "libopenstm32/bkp.h"	// #defines for addresses of backup registers

/* When 'x' is typed in an <enter> hit it goes into STANDBY mode for the following defined time period */
#define STANDBYTIME	100	// Time for next wakeup in tenth seconds
#define SLEEPDEEPTICKS	(ALR_INCREMENT*STANDBYTIME)/10	// TR_CLK tick count added to current RTC CNT counter for wakeup

/* These are debugging and checking things */
extern unsigned int RTC_debug0;	// Debugging bogus interrupt
extern unsigned int RTC_debug1;	// Debugging 32 KHz osc not setup as expected
extern unsigned int RTC_debug2;	// Type of reset "we think we had"
unsigned int RTC_debug0x;	// Previous value
unsigned int tempb;

static void output_calibrated_adc (unsigned int uiT);
static void output_100_scale(int uiS);
static void exitthismessviasleep(void);
static void output_calibrated_adc (unsigned int uiT);

/* 'struct CLOCKS clocks' is used to setup the clock source, PLL, dividers, and bus clocks 
See P 84 of Ref Manual for a useful diagram.
../lib/libmiscstm32/clockspecifysetup.h has the 'enum' values that may help for making mistakes(!) */
/* NOTE: Bus for ADC (APB2) must not exceed 14 MHz */
struct CLOCKS clocks = { \
HSOSELECT_HSE_XTAL,	/* Select high speed osc 			*/ \
0, /* PLLMUL_4X,*/	/* Multiplier PLL: 0 = not used 		*/ \
1,			/* Source for PLLMUL: 0 = HSI, 1 = PLLXTPRE (1 bit predivider)	*/ \
0,			/* PLLXTPRE source: 0 = HSE, 1 = HSE/2 (1 bit predivider on/off)	*/ \
APBX_1,			/* APB1 clock = SYSCLK divided by 1,2,4,8,16; freq <= 36 MHz */ \
APBX_1,			/* APB2 prescalar code = SYSCLK divided by 1,2,4,8,16; freq <= 72 MHz */ \
AHB_1,			/* AHB prescalar code: SYSCLK/[2,4,8,16,32,64,128,256,512] (drives APB1,2) */ \
8000000			/* Oscillator source frequency, e.g. 8000000 for an 8 MHz xtal on the external osc. */ \
};

/* Default calibrations for various devices */
extern struct CALBLOCK strDefaultCalib;		// Default calibration
void output_calibrated_adc (unsigned int uiT);		// Routine (near bottom) for outputting calibrated

/* RTC timers - tick count is the RTCCLK (32768 Hz) divided by the prescalar (16) (2048 ticks make one second) */
extern void 	(*rtc_timerdone_ptr)(void);		// Address of function to call at end of 'rtctimers_countdown'

/* RTC callback routines (for test) */
void rtc_call_timer0(void);
void rtc_call_timer1(void);
void rtc_call_timer2(void);


/* RTC registers: PRL, CNT, ALR initial values (see ../devices/32KHz.h, also p 448 Ref Manual) (see ../devices/32KHz.c) */
struct RTCREG strRtc_reg_init = {PRL_DIVIDER,0,ALR_INCREMENT}; // SECF = 1/2 sec, CNT
struct RTCREG 	strRtc_reg_read;	// Readback of registers
unsigned int	 uiRCC_CSR_init;	// RCC_CSR saved at beginning of RTC initialization (see 32KHz.c)
extern void 	(*rtc_secf_ptr)(void);	// Address of function to call during RTC_IRQHandler of SECF (Seconds flag)
extern char	cResetFlag;		// Out of a reset: 1 = 32 KHz osc was not setup; 2 = osc setup OK, backup domain was powered
extern unsigned int	uiSecondsFlag;	// 1 second tick: 0 = not ready, + = seconds count (see 32KHz.c)


		
/* The following variables are used by peripherals in their 'init' routines to set dividers 	*/
/* We might use them here just to display how the clock was set up */
extern unsigned int	hclk_freq;  	/* 	SYSCLKX/HPREDIV	 	E.g. 72000000 	*/
extern unsigned int	pclk1_freq;  	/*	SYSCLKX/PCLK1DIV	E.g. 72000000 	*/
extern unsigned int	pclk2_freq;	/*	SYSCLKX/PCLK2DIV	E.g. 36000000 	*/
extern unsigned int	sysclk_freq;	/* 	SYSCLK freq		E.g. 72000000	*/

/* This if for test sprintf */
char vv[180];	// sprintf buffer

long long llX;	// Big integer for slowing pause loop


unsigned int uihdct;	// Header throttle count

float fVraw;
float fV1;
float fV2; 
float fVtop;
float fVbot;
float fIchg;
float fV1cal;
float fV2cal;
unsigned int uiAdcTherm;
unsigned int uiAdcTherm_m;
unsigned int uiThermtmp;
unsigned int uiVraw;
unsigned int uiVbotcal;
unsigned int uiVtopcal;
unsigned int uiTopCellDif;
unsigned int uiIchgmax;
unsigned int uiIchg;
unsigned int uiCton,uiCtoff,uiCtna,uiCttot;
int nV1cal;
int nV2cal;

unsigned int uiVrawAve;
unsigned int uiVbotcalAve;
unsigned int uiTopCellDifAve;
unsigned int uiVtopcalAve;
unsigned int uiAveCt;
unsigned int uiShuntLo, uiShuntHi;
int nV1calAve;
int nV2calAve;

/* Calibrations for ADCs */
#define TMPOFF	-380	// Temperature calibration offset
#define CALIBRATIONVraw	7.264935	// Volts per count * 1000 Raw DC input (after diode)
#define CALIBRATIONV1	3.754657	// Volts per count * 1000 Current sense high side
#define CALIBRATIONV2	3.739559	// Volts per count * 1000 Current sense low side
#define CALIBRATIONVbot	1.167017	// Volts per count * 1000 Battery top
#define CALIBRATIONVtop	2.184703	// Volts per count * 1000 Battery bottom cell
#define CURRENTSENSECONDUCTANCE	.3773584906	// Millamp (* 1000) (1000/Rsense)

/* Thresholds for determination states */
#define THRESHOLDVraw	 8100		// Calibration volts (* 1000) when OK to charge
#define THRESHOLDcellmax 4160		// Cell voltage maximum
#define THRESHOLDcelllow 2600		// Cell voltage minimum for normal charge rate
#define THRESHOLDbalance 4		// Cell voltage difference (millivolts) kick in balancing
#define	THRESHOLDv2	 9100		// V2 voltage to shutoff P-FET

/* Charge rate control */
#define CHARGERATEHI	260		// Millamp charging current limit (normal charge rate)
#define CHARGERATELO	 22		// Millamp charging current limit (low rate for overly discharge cells)

/* Prevent startup, initial zero readings from putting it into sleep mode */
#define STARTUPDELAY	3000*6		// Delay while adc counts and differences get settled
unsigned int uiStartupDelayCtr;

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
	unsigned int	uiTopCellDif;
	unsigned int	uiAdclocal[6];

	/* Some temps */
	unsigned int	uiTemp;
	float f1,f2;
	u16 u16temp;
	u32 ui32;
	
	int i;			// Nice FORTRAN variables
	char* p;		// Temp getline pointer
 
/* ---------- Initializations ------------------------------------------------------ */
	clockspecifysetup(&clocks);		// Get the system clock and bus clocks running

	PODgpiopins_default();	// Set gpio port register bits for low power
	PODgpiopins_Config();	// Now, configure pins

	/* Configure pins not covered in the foregoing */	
	configure_pin ((volatile u32 *)GPIOD, 14);	// 5v regulator supplying GPS and BOX LED
	GPIO_BSRR(GPIOD) = (1<<14);			// Set bit

	GPIO_BRR(GPIOA)  = (1<<2);			// Reset bit
	configure_pin ((volatile u32 *)GPIOA, 2);	// N-FET that drives Lower cell shunt resistor

	GPIO_BRR(GPIOD)  = (1<<12);			// Reset bit
	configure_pin ((volatile u32 *)GPIOD, 12);	// N-FET switches P-FET drive for charge current (BEEPER PA2)


//	gpio_setup();	// Olimex gpio setup for LED

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
	// Initialize USART and setup control blocks and pointer
	if ( (u16temp = USART1_rxinttxint_init(115200,32,2,192,3)) != 0 )
	{ // Here, something failed 
		return u16temp;
	}
	
	/* Announce who we are */
	USART1_txint_puts("\n\r#### linear_batt_chgr.c 01-26-2012\n\r");  USART1_txint_send();// Start the line buffer sending

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
		BKP_DR1 = 0;	// Extended seconds counter
	}
	else
	{
		/* Compute and set the next alarm register */
		strRtc_reg_read.alr =	((strRtc_reg_read.cnt & ~((1<<11)-1)) +(1<<11));
		RTC_reg_load_alr(strRtc_reg_read.alr);
		USART1_txint_puts("Backup domain was good\n\r");	USART1_txint_send();	// Something for the hapless op
	}
	/* Compute elapsed seconds */
	ui32 = ((strRtc_reg_read.cnt>>ALR_INC_ORDER) | (BKP_DR1>>PRL_DIVIDE_ORDER));	// Combine high order bits with CNT register
	printf ("Elapsed seconds: %9u\n\r",ui32);

	/* Check if the RTC register setup OK */	
	RTC_reg_read(&strRtc_reg_read);		// Read-back of register settings
	printf ("RTC reg after lod:PRL 0x%08x, CNT 0x%08x, ALR 0x%08x\n\r",strRtc_reg_read.prl,strRtc_reg_read.cnt,strRtc_reg_read.alr);	USART1_txint_send();

	/* Direct backup register test */
	BKP_DR3 = 0x1234;
	BKP_DR4 = 0x5678;
	tempb = (BKP_DR4 & 0xffff) | (BKP_DR3 << 16);
	printf ("TEST:BKP_DR3 DR4 reads as %08x and should be 0x12345678\n\r",tempb);	USART1_txint_send();

	/* Test rtctimers: countdown with looping wait */
/* NOTE: besides testing the 32 KHz stuff, the following gives some time to execute 'jtag' to halt the processor before
it sees a low battery and goes into sleep (whereupon a new program cannot be loaded). */
	
#define TIMERWAIT	1	// Seconds to wait
	printf("====== Timer start %u seconds, loop on timer count\n\r",TIMERWAIT);	USART1_txint_send();
	for (i = 0; i < NUMBERRTCTIMERS; i++)	// Set each timer to 1 second more than the previous
	{
		nTimercounter[i] = 1024*(TIMERWAIT+i);	// Set tick count (2048 = 1 sec; 1/4 sec)
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
	uiADCt1 = adc_init((volatile struct ADCDR*)&strADC1dr);				// Initialize the ADC for DMA reading of all POD adc pins
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

/* --------------------- Endless loop only broken by powrer off, or low cell sleep ----------------------------------------- */
	i = 0;
	while (1==1)
	{
		/* Fetch recent ADC readings into local buffer (don't worry about which double buffer to read */
		for (i = 0; i < 6; i++)	// Make a local copy since these are not synch'ed to the ADC cycle
			uiAdclocal[i] = strADC1dr.in[0][i];

		/* Convert to floats for the lazy way of doing the calibration */
		uiAdcTherm_m = uiAdclocal[0];	// Thermistor on 32 KHz xtal
		fVbot = uiAdclocal[1];	// Bottom cell of battery
		fVtop = uiAdclocal[2];	// Top of battery	
		fVraw = uiAdclocal[3];	// PC 0 ADC12 -IN10	Raw input Volts (Accelerometer X)
		fV1   = uiAdclocal[4];	// PC 1 ADC12 -IN11	Current sense V1 (Accelerometer Y)
		fV2   = uiAdclocal[5];	// PC 2 ADC12 -IN12	Current sense V2 (Accelerometer Z)

		/* Calibration temperature */
		uiAdcTherm = uiAdcTherm_m;		// 
		uiThermtmp = adctherm_cal(uiAdcTherm);	// Convert ADC reading to temp (deg C)

		/* Calibrate & scale voltages to (volts * 1000) */
		uiVraw    = (fVraw * CALIBRATIONVraw);
		uiVbotcal = (fVbot * CALIBRATIONVbot);
		uiVtopcal = (fVtop * CALIBRATIONVtop);
		fV1cal    = (fV1 * CALIBRATIONV1);
		fV2cal    = (fV2 * CALIBRATIONV2);
		nV1cal    = fV1cal;	// For fixed pt later
		nV2cal    = fV2cal;

		/* Compute top cell voltage */
		uiTopCellDif = uiVtopcal - uiVbotcal;	// Top cell = (Top of battery volts - lower cell volts)

		/* Compute charge current in milliamps * 1000 */
		fIchg = (fV1cal - fV2cal) * CURRENTSENSECONDUCTANCE;	// Current = (V1 - V2)/Rsense
		if (fIchg < 0 ) fIchg = 0;	// Zilch current might compute negative, so zero it out
		uiIchg = fIchg;	// Do the rest in fixed pt.
		
        if (fV2cal < THRESHOLDv2) // Is there a condition where the V2 voltage is too high? (might occur if POD unplugged)
	{ // Here no.  
		/* Turn on P-FET to charge if DC input is on, and each cell is below max */
		if (uiVraw > THRESHOLDVraw)
		{
			/* If either cell is low, set the low charge current until the voltage comes up */
			if ((uiVbotcal < THRESHOLDcelllow) || (uiTopCellDif < THRESHOLDcelllow) )
				uiIchgmax = CHARGERATELO;	// Very low current
			else
				uiIchgmax = CHARGERATEHI;	// Normal current

			/* Switch the charging P-FET on if the voltage is low, or off if high, and do cell balancing */
			/* 
				NOTE: the P-FET has a rc filter on the gate, and the speed of this loop is fast enough that it
				essentially creates a PWM that adjusts the gate voltage to acheive the target current
			*/
			if ( (uiVbotcal < THRESHOLDcellmax) && (uiTopCellDif < THRESHOLDcellmax) && (uiIchg < uiIchgmax) )
			{ // Here, the conditions are met to turn charging P-FET on 
				GPIO_BSRR(GPIOD)  = (1<<12);	// Set bit 
				uiCton += 1;	// Counter to check on duty cycle

				uiShuntLo = 0; uiShuntHi = 0;
				GPIO_BRR(GPIOA) = (1<<2); // Reset bit: N-FET that drives Lower cell shunt resistor (PA2 BEEPER)
				GPIO_BRR(GPIOE) = (1<<2); // Reset bit: N-FET that drives Upper cell shunt resistor (PE2 EXTERNAL LED FET)

				/* Check the cell balance and turn on the resistor shunt across the high cell (if the difference sufficient) */
				if ( ((signed int)(uiVbotcal - uiTopCellDif)) > 0)
				{ // Here bottom cell is higher than top cell
					uiTemp = uiVbotcal - uiTopCellDif;	// Absolute diff
					if (uiTemp > THRESHOLDbalance)
					{
						GPIO_BRR(GPIOA)  = (1<<2);// Reset bit: N-FET that drives upper cell shunt resistor (PA2 BEEPER)
						GPIO_BSRR(GPIOE) = (1<<2);// Set bit: N-FET that drives lower cell shunt resistor (PE2 EXTERNAL LED FET)
						uiShuntLo = 0; uiShuntHi = 1;
					}
				}
				else
				{ // Here top cell is lower (or equal) to bottom cell
					uiTemp = uiTopCellDif - uiVbotcal; 	// Absolute diff
					if (uiTemp > THRESHOLDbalance)
					{
						GPIO_BSRR(GPIOA) = (1<<2);// Set bit: N-FET that drives upper cell shunt resistor (PA2 BEEPER)
						GPIO_BRR(GPIOE)  = (1<<2);// Reset bit: N-FET that drives lower cell shunt resistor (PE2 EXTERNAL LED FET)
						uiShuntLo = 1; uiShuntHi = 0;
					}
				}				
			}
			else
			{ // Here, either there is a cell at max voltage, or charging current is above max
				GPIO_BRR(GPIOD)  = (1<<12);	// Reset bit to turn off charging
				uiCtoff += 1;	// Counter to check on duty cycle
				/* If a cell has reached max, then turn off the cell balancing */
				if ( (uiVbotcal >= THRESHOLDcellmax) || (uiTopCellDif >= THRESHOLDcellmax) )
				{ // Here, there one or both of the cells are at or over max voltage, and charge current is above max
					uiShuntLo = 0; uiShuntHi = 0;
					GPIO_BRR(GPIOA) = (1<<2); 	// Reset bit: N-FET that drives Lower cell shunt resistor (PA2 BEEPER)
					GPIO_BRR(GPIOE) = (1<<2); 	// Reset bit: N-FET that drives Upper cell shunt resistor (PE2 EXTERNAL LED FET)
				}
			}
		}
		else
		{ // Here, there is no input DC so we cannot charge
			uiShuntLo = 0; uiShuntHi = 0;
			GPIO_BRR(GPIOA) = (1<<2); 	// Reset bit: N-FET that drives Lower cell shunt resistor (PA2 BEEPER)
			GPIO_BRR(GPIOE) = (1<<2); 	// Reset bit: N-FET that drives Upper cell shunt resistor (PE2 EXTERNAL LED FET)
			GPIO_BRR(GPIOD) = (1<<12);	// Reset bit: N-FET that drives P-FET charging FET	

			/* The following prevents the GPS et al. from over-discharging the battery */
			if ( uiStartupDelayCtr < STARTUPDELAY )
			{
				uiStartupDelayCtr += 1;
			}	
			else
			{		
				if ( (uiVbotcal < THRESHOLDcelllow) || (uiTopCellDif < THRESHOLDcelllow) )
				{ // Here, one or both of the cells is below the minimum voltage and charger is off.  We MUST power down.
					printf("\n\r@@@ Cell voltage is below %u @@@\n\r",THRESHOLDcelllow);	// Something for the hapless op
					output_calibrated_adc(uiVbotcalAve/uiAveCt);
					output_calibrated_adc(uiTopCellDifAve/uiAveCt);
					output_calibrated_adc(uiVtopcalAve/uiAveCt);
					USART1_txint_send();	

					exitthismessviasleep();		// Put unit in deepsleep mode				
				}
			}
		}
	}
	else
	{ // Here V2 is too high.  This would happen if the POD were unplugged and the charger powered.  Vraw will not drop fast because of the filter
  	  // capacitors.  V2 will go up quickly and to prevent an overvoltage to the 3.3v regulator on the stm32 board the P-FET needs to be shutdown
	  // as quickly as possible, so sensing the condition and turning off the P-FET drive as soon as possible is done here.
		GPIO_BRR(GPIOD)  = (1<<12);	// Reset bit to turn off charging
		GPIO_BRR(GPIOA) = (1<<2); 	// Reset bit: N-FET that drives Lower cell shunt resistor (PA2 BEEPER)
		GPIO_BRR(GPIOE) = (1<<2); 	// Reset bit: N-FET that drives Upper cell shunt resistor (PE2 EXTERNAL LED FET)
	}      
		uiCttot += 1;	// Counter to check on duty cycle and above counters
	
		/* ------------------------ serial output for monitoring and debugging -------------------------------*/

		/* Build accumulation for averaging */
		uiVrawAve       += uiVraw;
		uiVbotcalAve    += uiVbotcal;
		uiTopCellDifAve += uiTopCellDif;
		uiVtopcalAve    += uiVtopcal;
		nV1calAve       += nV1cal;
		nV2calAve       += nV2cal;
		uiAveCt += 1;		

		if (nTimercounter[0] == 0)	// Time to do a printf?
		{ // Here, yes.
			nTimercounter[0] = 1024; 	// 1/2 second

			if (uihdct++ >= 9)
			{
				uihdct = 0;
				USART1_txint_puts("   DC in     Bot      Top     Total     Iv1      Iv2      Vdif   I(ma)  ON   OFF  TOTAL    Duty     Temp(C) lo hi\n\r");	USART1_txint_send();
			}
// Raw ADC values
//for (i = 1; i < 6; i++)
//  printf ("%5u ",uiAdclocal[i]);
//USART1_txint_puts("\n\r");	USART1_txint_send();

			/* Output the following voltages are in xxx.xxx (volts) form */
			output_calibrated_adc(uiVrawAve/uiAveCt);
			output_calibrated_adc(uiVbotcalAve/uiAveCt);
			output_calibrated_adc(uiTopCellDifAve/uiAveCt);
			output_calibrated_adc(uiVtopcalAve/uiAveCt);
			output_calibrated_adc(nV1calAve/uiAveCt);
			output_calibrated_adc(nV2calAve/uiAveCt);
			if ((nV1calAve-nV2calAve) < 0) // Sense resistor voltage drop
			{ // When the P-FET is off sometimes the difference can be a few counts negative due to ADC noise
				USART1_txint_puts ("   -.--- ");
			}
			else
			{ // Here, this is the good stuff
				output_calibrated_adc( (nV1calAve-nV2calAve) /uiAveCt);
			}
			printf ("%5u ",uiIchg);	// Output in milliamps

			f1 = uiCton; f2 = uiCttot; uiCtna = 1000 * f1/f2;
			printf ("%5u %5u %5u ", uiCton, uiCtoff, uiCttot); // Duty cycle counts
			output_calibrated_adc(uiCtna);	// Duty cycle as .xxx
			uiCton = 0; uiCtoff = 0; uiCttot = 0; uiCtna = 0;		

			output_100_scale(uiThermtmp + TMPOFF);		// Output as XXXX.XX
			printf (" %2u%2u",uiShuntLo,uiShuntHi);	// Cell shunt switch settings
			USART1_txint_puts("\n\r");	USART1_txint_send();

			/* Reset accumulators for averaging */
			uiVrawAve = 0;
			uiVbotcalAve = 0;
			uiTopCellDifAve = 0;
			uiVtopcalAve = 0;
			nV1calAve = 0;
			nV2calAve = 0;	
			uiAveCt = 0;	

		}
		

		/* Echo incoming chars back to the sender */
		if ((p=USART1_rxint_getline()) != 0)	// Check if we have a completed line
		{ // Here, pointer points to line buffer with a zero terminated line
			if (*p == 's') break;
		}

	}
	USART1_txint_puts("\n\r@@@ Unexpected end of loop! @@@\n\r");	USART1_txint_send();	// Something for the hapless op
	exitthismessviasleep();		// Put unit in deepsleep mode
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
Setup for output an unsigned with a 100* scale as xxxx.xx
*****************************************************************************************/
static void output_100_scale(int uiS)
{
	printf ("%8d.%02u",uiS/100,(uiS % 100) );

	return;
}
/*****************************************************************************************
Setup an int for a floating pt type output
*****************************************************************************************/
static void output_calibrated_adc (unsigned int uiT)
{
	unsigned int uiX = uiT/1000;	// Whole part
	unsigned int uiR = uiT % 1000;	// Fractional part

	printf (" %3u.%03u ",uiX,uiR);	// Setup the output

	return;
}
/*****************************************************************************************
Put unit into deepsleep mode.  Pushbutton required to wake it up
*****************************************************************************************/
static void exitthismessviasleep(void)
{
volatile unsigned long ull = 0;

	/* Turn off some stuff, jic */
	GPIO_BRR(GPIOA) = (1<<2); // Reset bit: N-FET that drives Lower cell shunt resistor (PA2 BEEPER)
	GPIO_BRR(GPIOE) = (1<<2); // Reset bit: N-FET that drives Upper cell shunt resistor (PE2 EXTERNAL LED FET)
	GPIO_BRR(GPIOD)  = (1<<12);			// Reset bit

	USART1_txint_puts ("### Exit to deepsleep. Press pushbutton smartly to wake up unit ###\n\r"); USART1_txint_send();
	while (ull++ < 4000);	// Wait for USART message to complete

	Powerdown_to_standby( -100  );	// Set ALR wakeup, setup STANDBY mode.
	USART1_txint_puts("WFE should not come here\n\r");	USART1_txint_send();	// Something for the hapless op
	while (1==1);	// Should not come here
	
	return;
}


