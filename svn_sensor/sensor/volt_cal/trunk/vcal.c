/******************************************************************************
* File Name          : vcal.c
* Date First Issued  : 09/27/2015
* Board              : POD board
* Description        : Voltage calibrator using POD board w AD7799
*******************************************************************************/
/* 

Strategy--
After intialization--
  main: endless loop
    computes temperatures from thermistor readings
    computes tension offset adjustment factor
    printf for monitoring
    triggers a low level interrupt to poll send/handle CAN msgs
  timer3: 

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
//#include <stdio.h>
#include <stdlib.h>
extern int errno;

#include "PODpinconfig.h"
#include "pinconfig_all.h"
#include "SENSORpinconfig.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/gpio.h"
#include "libopenstm32/usart.h"
#include "libopenstm32/scb.h"
#include "libusartstm32/usartallproto.h"
#include "libmiscstm32/clockspecifysetup.h"
#include "libmiscstm32/DTW_counter.h"
#include "libsensormisc/canwinch_setup_F103_pod.h"
#include "../../../../svn_common/trunk/common_can.h"
#include "../../../../svn_discoveryf4/sw_discoveryf4/trunk/lib/libopencm3/cm3/common.h"
#include "poly_compute.h"
#include "temp_calc_param.h"
#include "libmiscstm32/DTW_counter.h"
#include "adcsensor_vcal.h"
#include "panic_leds_pod.h"
#include "printf.h"
#include "tim2_vcal.h"
#include "fpprint.h"
#include "loopctl.h"
#include "loopctl_print.h"
//#include "idx_v_struct_print.h"
#include "yogPC_cmd.h"
#include "iir_1.h"
#include "p1_initialization.h"
#include "ad7799_vcal_filter.h"
#include "ad7799_vcal_comm.h"
#include "recursive_variance.h"
#include "kalman_scalar.h"
#include "i2c1.h"
#include "hd44780.h"
#include "vcal_cmd.h"

static double comp_dbl(double c[], int n, double t);

double  therm[NUMBERADCCHANNELS_TEN];		// Doubles of thermistors readings
float  thermf[NUMBERADCCHANNELS_TEN];		// Floats of thermistors readings

static u32 ticks = 0;

/* Parameters for each channel. */
struct AD7799CHAN chn[AD7799NUMCHANNELS]; 

/* Easy way for other routines to access via 'extern'*/
struct CAN_CTLBLOCK* pctl1;

/* Specify msg buffer and max useage for CAN1: TX, RX0, and RX1. */
const struct CAN_INIT msginit = { \
64,	/* Total number of msg blocks. */
40,	/* TX can use this huge ammount. */
30,	/* RX0 can use this many. */
8	/* RX1 can use this piddling amount. */
};
/* **************************************************************************************
 * void putc ( void* p, char c); // This is for the tiny printf
 * ************************************************************************************** */
void putc (void* p, char c)
{
	p=p;	// Get rid of the unused variable compiler warning
	USART1_txint_putc(c);
	return;
}
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
/*#################################################################################################
And now for the main routine 
  #################################################################################################*/
int main(void)
{
	int i,j;
	int ret;

/* $$$$$$$$$$$$ Relocate the interrupt vectors from the loader to this program $$$$$$$$$$$$$$$$$$$$ */
//extern void relocate_vector(void);
//	relocate_vector();
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
	STRAINGAUGEPWR_on;	// Turn on 5.0v regulator for AD7799s

	init_printf(0,putc);	// This one-time initialization is needed by the tiny printf routine

	/* Use DTW_CYCCNT counter for startup timing */
	DTW_counter_init();
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
	USART1_rxinttxint_init(115200,16,4,96,4); // Initialize USART and setup control blocks and pointers

	/* Announce who we are */
	USART1_txint_puts("\n\rVOLTAGE CALIBRATOR 11-02-2015 v0\n\r");USART1_txint_send();

	/* Display things so's to entertain the hapless op */
	printf ("  hclk_freq (MHz) : %9u\n\r",  hclk_freq/1000000);
	printf (" pclk1_freq (MHz) : %9u\n\r", pclk1_freq/1000000);
	printf (" pclk2_freq (MHz) : %9u\n\r", pclk2_freq/1000000);
	printf ("sysclk_freq (MHz) : %9u\n\r",sysclk_freq/1000000);	USART1_txint_send();
/* ---------------- I2C1 display  --------------------------------------------------------------------- */
	hd44780_init(0x27);
//                    ................
//	hd44780_puts("vcal 151102", 0);
/* ---------------- Parameter ------------------------------------------------------------------------- */
	vcal_idx_v_struct_init();	// Setup parameters
/* ------------------ Get ADC initialized and calibrated ---------------------------------------------- */
	adcsensor_vcal_sequence();	// Initialization sequence for the adc
/* ------------------ TIM2 for PWM FET pin PA2 OVEN #1 HEATER ----------------------------------------- */
	tim2_vcal_init();	//Initialize TIM2 for PWM and other timing (1 frame per sec)
	tim2_vcal_setpwm(8000);	// Set PWM tick count at 1/8th
/* ------------------ Heat/cool ----------------------------------------------------------------------- */
	loopctl_init_oven(&thm.oven[0]);	// Init control loop for oven #1
/* ----------------- AD7799 initialization ------------------------------------------------------------ */
	ret = p1_initialization_vcal();
	if (ret != 0) {printf("AD7799 init err: %d\n\r",ret);USART1_txint_send();while(1==1);}
	ad7799_vcal_filter_start(&chn[0]);// Start interrupt driven ad7799 readings; sequence though channels
/* ----------------- Debug parameters ----------------------------------------------------------------- */

/* ---------------- Some vars associated with endless loop -------------------------------------------- */
printf("PROGRESS CHECK 1\n\r");USART1_txint_send(); 

extern uint32_t	adc_readings_cic[2][NUMBERADCCHANNELS_TEN]; // Filtered/decimated ADC readings
int adc_temp_flag_prev = adc_temp_flag[0]; 	// ADC readings ready counter
char s[32];
char s2[32];
char s3[32];

// Scaling for cic filtering in adcsensor_vcal.[ch]
#define SCALESHIFT  19					// Float to ascii (since %f is not working)
#define DSCALE (1.0 / (1 << SCALESHIFT))		// ADC filtering scale factor (reciprocal multiply)


/* Reciprocal of temperature sensor slope. */
double dtslope = (1E3 / thm.tsens_slope); // (degC/volt) temp sensor slope

/* ADC usage
Usage for Voltage calibrator (POD) board
PA3  - IN3 AD7799 temperature
PC 0 - IN10 Voltage source temperature
     - Internal temperature reference
     - Internal voltage source
PC1  - 2.5v precision voltage source
PC2  - Direct measurement pin
*/
double dtmp;
double dvref;
double dtsens;
double dvadj;
double d2p5;
double dcal;	// ADC volts/ADC_ct
double dvext;
double temptemp;	// Temperature temporary (for temporary temperature...)
uint32_t count = 1;

struct LOOPCTL_STATE* ploopctl;

/* AD7799 reading-ready flag increments. */
uint16_t ad7799_ready_flag_prev = ad7799_ready_flag;
double dsum_save[AD7799NUMCHANNELS];


/* Time to compute check */
u32 t0 = DTWTIME;
int tdiff = 0;
int tdiff_prev = 0;
int tmax = -1;
u32 q0,q1,q2,q3,q4,q5,q6;
int qdiff;
int q1max = 0;
int q2max = 0;
int q3max = 0;
int q4max = 0;
int q5max = 0;
int qflag = 0;

// local save of reads
extern int dbg;
extern uint8_t ad7799id;				// Save ID (Debug)
extern union SHORTCHAR	ad7799_vcal_16bit_save;		// Save (Debug)
extern union SHORTCHAR	ad7799_vcal_16bit_save2;	// Save (Debug)
extern uint8_t		ad7799_vcal_8bit_save;
int sumtmp;

extern uint32_t adt0; 
extern uint32_t adt1; 
extern uint32_t adt2; 

/* AD7799 usage
AIN2 +/- 
AIN2 +/- White / Yellow
AIN3 +/- +2.5 volt ref / ground
*/

/* Variance tinkering */
struct RECURSIVE rvar;
rvar.oto = 8;	// Number readings to discard before startup

struct RECURSIVE rvar1;
rvar1.oto = 8;	// Number readings to discard before startup

struct RECURSIVE rvar2;
rvar2.oto = 8;	// Number readings to discard before startup

/* Kalman tinkering */
struct KALMANSC kal;
// Initial conditions for filter
kal.P = 1.0;
kal.R = 01E-2;
kal.k = 0; 
double xret;
uint32_t skipr = 0;

#define VPRECREF 2.5000		// Precision reference voltage
#define RATIOLOW  (1.0/0.279875)	// Resistor divider (0 - 17v)
#define RATIOHIGH (1.0/0.075777)	// Resistor divider (0 - 65v)
#define OFFSETLOW  (2048.767/READSUMCT)	// Zero volts input, offset
#define OFFSETHIGH (2015.932/READSUMCT)	// Zero volts input, offset
#define OFFSETREF  (1507.877/READSUMCT)	// Zero volts input, offset

#define ADCCALRAW  (1.0/169.524)	// Volts per tick


for (i = 0; i < AD7799NUMCHANNELS; i++)
{
	printf("%d %04x %04x %04x %04x %2d %2d\n\r",i, chn[i].mode, chn[i].calibzero, chn[i].calibfs,chn[i].config, chn[i].ct, chn[i].mux);
USART1_txint_send();
}

/* --------------------- Endless Stuff ----------------------------------------------- */
	while (1==1)
	{
tdiff = (int)DTWTIME - (int)t0;
if (tdiff > tdiff_prev)
{
 tdiff_prev = tdiff;
 if ((tdiff > tmax) || (qflag != 0) )
 {
   printf("T %d %d %d %d %d %d\n\r",tdiff, q1max, q2max, q3max, q4max, q5max);USART1_txint_send();
   tmax = tdiff; qflag = 0;
 }
}
t0 = DTWTIME;

		if ((GPIO_IDR(GPIOA) & (1<<2)) == 0)
		{ // Here, PWM pin is OFF
			GPIO_BSRR(GPIOE) = (1<<LED4);	// Set bits = LED off
		}
		else
		{ // HEre, PWM pin is ON
			GPIO_BRR(GPIOE) = (1<<LED4);	// Reset bits = LED on
		}

		/* Deal with PC incoming lines w commands. */
//		yogPC_cmd_poll();

//	if (dbg > 1)
//	{
//		printf("\n\r#### %d %04x %04x %016x %016x\n\r",dbg,ad7799id, ad7799_vcal_8bit_save, ad7799_vcal_16bit_save.us, ad7799_vcal_16bit_save2.us);
//		USART1_txint_send();
//		dbg = -1;
//	}

		/* Check for keyboard command */
		ret = vcal_cmd_poll();
		if ((ret & 0x1) != 0)
		{ // Here, 'R' command to reset recursive averaging was given
			recursive_variance_reset(&rvar);
			recursive_variance_reset(&rvar1);
			recursive_variance_reset(&rvar2);
		}

		/* AD7799 readings. */
		if (ad7799_ready_flag  != ad7799_ready_flag_prev)
		{ // Here: all AD7799 channels have been read/summed
			ad7799_ready_flag_prev = ad7799_ready_flag;
			for (i = 0; i < AD7799NUMCHANNELS; i++)
			{ // Here, the sequence has completed
sumtmp = sum_save[i];
printf(" %11d",sumtmp);
				dsum_save[i] = sum_save[i]; 	// Convert long to double

			}

			/* Apply zero volts offset. */
			dsum_save[0] -= OFFSETLOW;	
			dsum_save[1] -= OFFSETHIGH;	
			dsum_save[2] -= OFFSETREF;	

			dtmp = VPRECREF/dsum_save[2];
			fpformat(s, dsum_save[0] * dtmp * 1E3 * RATIOLOW);	printf(" %s",s);
			fpformat(s, dsum_save[1] * dtmp * 1E3 * RATIOHIGH);	printf(" %s",s);

			/* Recursive average and variance. */
			recursive_variance(&rvar, dsum_save[0] * dtmp * 1E3 * RATIOLOW );
			fpformat(s, rvar.xbnp1);	printf(" %sL",s);
			sprintf(s2,"%sL",s);
			if ((skipr & 0x1) == 0)		// Alternate lines on display
				hd44780_puts(s2, 0);

			recursive_variance(&rvar1, dsum_save[1] * dtmp * 1E3 * RATIOHIGH );
			fpformat(s, rvar1.xbnp1);	printf(" %sH",s); 
			sprintf(s2,"%sH",s);
			if ((skipr++ & 0x1) == 1)	// Alternate lines on display
				hd44780_puts(s2, 1);

			printf(" %d",rvar.n);

//!			recursive_variance(&rvar2, dsum_save[2] );
//!			fpformat(s, rvar2.xbnp1 * 1E0);	printf(" %sC",s); 

//	printf(" %d",rvar.n);

			/* Kalman filtering. */
			kalman_scalar_filter(&kal, dsum_save[0] / dsum_save[2]);
//			fpformat(s, 1E6 * kal.K);	printf(" %s",s); 
//			fpformat(s, 1E6 * kal.xhat);	printf(" %s %d",s, kal.k);
		//                    ................

			 
			

USART1_txint_puts("\n\r"); USART1_txint_send(); // Print the foregoing	
		}

		/* Handle thermistor-to-temperature conversion, and AD7799 temperature correction. */
		if (adc_temp_flag[0] != adc_temp_flag_prev)
		{ // Here, a new set of ADC readings are ready
		/* Poll loop flashing of LED. */
//$ printf("%2d %02x %02x %04x %04x",dbg,ad7799id, ad7799_vcal_8bit_save, ad7799_vcal_16bit_save.us, ad7799_vcal_16bit_save2.us);
			toggle_1led(LED3);

			j = (0x1 & adc_temp_flag_prev);	// Get current double buffer index
			adc_temp_flag_prev = adc_temp_flag[0];
ticks += 1;
q5 = DTWTIME;
//$			printf("%5d",count++);

			/* Convert ADC readings to temperature: AD7799 & voltage source oven #1. */
			for (i = 0; i < 2; i++)
			{
q0 = DTWTIME;
				dtmp = adc_readings_cic[j][i] * DSCALE; // Convert to double and scale
	
				/* Convert ADC readings into uncalibrated degrees Centigrade. */
				if (i == 1) 
				{ 
					temptemp = temp_calc_param_dbl(dtmp, &thm.oven[0].tp); // oven (PC0)
					therm[1] = comp_dbl( &thm.oven[0].tp.poly[0], 4, temptemp);
				}
				if (i == 0)
				{
					temptemp = temp_calc_param_dbl(dtmp, &thm.tp); // AD7799 (PA3)
					therm[0] = comp_dbl( &thm.tp.poly[0], 4, temptemp);
				}

q1 = DTWTIME;
				/* Apply calibration to temperature. */
				
				thermf[i] = (therm[i] * (9.0/5.0)) + 32.0; // Convert to F							
				fpformat(s, thermf[i]);	 printf(" %s",s);
q2 = DTWTIME;
q3 = DTWTIME;
q4 = DTWTIME;

qdiff = (int)q1 -(int)q0;
if (qdiff > q1max) {q1max = qdiff; qflag = 1;}

qdiff = (int)q2 -(int)q1;
if (qdiff > q2max) {q2max = qdiff; qflag = 1;}

qdiff = (int)q3 -(int)q2;
if (qdiff > q3max) {q3max = qdiff; qflag = 1;}

qdiff = (int)q4 -(int)q3;
if (qdiff > q1max) {q4max = qdiff; qflag = 1;}


			}

			/* 2.5v precision source measured with processor ADC. */
			d2p5 = iir_1 (((double)adc_readings_cic[j][4] * DSCALE), &thm.v2p5_iir);

			/* ADC volts/ADC_tick */
			dcal = (thm.v2p5_exact / d2p5);
//$			fpformat(s, dcal * 1000000.0);	printf(" %s",s);

			/* Convert ADC readings for Vrefint and internal temperature sensor. */
			dtsens = iir_1 (((double)adc_readings_cic[j][2] * DSCALE), &thm.tsens_iir);
			dvref  = iir_1 (((double)adc_readings_cic[j][3] * DSCALE), &thm.vref_iir);

			/* Convert internal temperature sensor ADC to deg C. */
			dtsens = ( (thm.tsens_v25 - dcal * dtsens) ) * dtslope + 25.0; // Internal temp (degC)
//$			fpformat(s, dtsens);	printf(" %s",s);

			/* Adjust internal voltage sensor for temperature. */
			dvadj = (dvref * dcal) * (1.0 + (dtsens - 25.0) * thm.vref_tempco);

//$			fpformat(s, dvadj * 1000); 	printf(" %s",s); // millivolts

			/* Internal voltage reference. */
			dvref = (dvref * dcal);
//$			fpformat(s, dvref * 1000.0);	printf(" %s",s); // millivolts

			/* External input to ADC pin PC2 */
			dvext = iir_1 (((double)adc_readings_cic[j][5] * DSCALE), &thm.vext_iir);
//$			fpformat(s, dvext * dcal * 1000.0);	printf(" %s",s); // millivolts
			
for (i = 0; i < NUMBERADCCHANNELS_TEN; i++)
{
  printf(" %d",adc_readings_cic[j][i] >> SCALESHIFT);
//  dtmp = adc_readings_cic[j][i] * DSCALE; // Convert to double and scale
//  fpformat(s, dtmp);	printf(" %s",s); // 
			


}
			ploopctl = loopctl_poll(therm[1], &thm.oven[0] );	// Update control loop: oven1
//$			loopctl_print(ploopctl);

			/* Take a look at the raw input voltage to this board. */
			dtmp = adc_readings_cic[j][4] * (DSCALE * ADCCALRAW); 
			fpformat(s, dtmp);	printf(" %sRAW",s); 

q6 = DTWTIME;
qdiff = (int)q6 -(int)q5;
if (qdiff > q5max) {q5max = qdiff; qflag = 1;}
			
			USART1_txint_puts("\n\r");
			USART1_txint_send(); // Print the foregoing
		}
	}
	return 0;	
}

static double comp_dbl(double c[], int n, double t)
{
	double x = c[0];
	double tt = t;
	int i;

	for (i = 1; i < n; i++)
	{
		x += (c[i] * tt);
		tt *= t;
	}
	return x;
}
