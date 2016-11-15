/******************************************************************************
* File Name          : tension.c
* Date First Issued  : 07/03/2015
* Board              : POD board
* Description        : Tension sensor: POD board w two AD7799
*******************************************************************************/
/* 
02-05-2014 Hack of se4.c routine
02-13-2015 Hack of se4_h.c w reference to adcpod.c
02-14-2015 Hack of tilt.c (which is not debugged)

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
UART2            = Teletype output
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
#include "libsensormisc/canwinch_setup_F103_Olimex.h"
#include "../../../../svn_common/trunk/common_can.h"
#include "../../../../svn_discoveryf4/sw_discoveryf4/trunk/lib/libopencm3/cm3/common.h"
#include "poly_compute.h"
#include "temp_calc_param.h"
#include "libmiscstm32/DTW_counter.h"
#include "adcsensor_yogurt.h"
#include "panic_leds_pod.h"
#include "printf.h"
#include "adcsensor_yogurt.h"
#include "tim3_yog.h"
#include "tim4_yog.h"
#include "yogurt_a_function.h"
#include "OlimexLED.h"
#include "fpprint.h"
#include "loopctl.h"
#include "loopctl_print.h"
#include "idx_v_struct_print.h"
#include "yogPC_cmd.h"

double  therm[NUMBERADCCHANNELS_TEN];		// Doubles of thermistors readings
float  thermf[NUMBERADCCHANNELS_TEN];		// Floats of thermistors readings
int    thermi[NUMBERADCCHANNELS_TEN];		// ints of thermistors readings * 1000

static u32 ticks = 0;

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
	USART2_txint_putc(c);
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
extern void relocate_vector(void);
	relocate_vector();
/* --------------------- Begin setting things up -------------------------------------------------- */ 
	// Start system clocks using parameters matching CAN setup parameters for POD board
	clockspecifysetup(canwinch_setup_F103_Olimex_clocks() );
/* ---------------------- Set up pins ------------------------------------------------------------- */
//	PODgpiopins_default();	// Set gpio port register bits for low power
//	PODgpiopins_Config();	// Now, configure pins

	OlimexLED_init();	// Setup the one and only LED

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
	USART2_rxinttxint_init(115200,32,2,128,8); // Initialize USART and setup control blocks and pointers

	/* Announce who we are */
	USART2_txint_puts("\n\rYOGURT CONTROLLER 08-11-2015 deg F v1\n\r");USART2_txint_send();

	/* Display things so's to entertain the hapless op */
	printf ("  hclk_freq (MHz) : %9u\n\r",  hclk_freq/1000000);
	printf (" pclk1_freq (MHz) : %9u\n\r", pclk1_freq/1000000);
	printf (" pclk2_freq (MHz) : %9u\n\r", pclk2_freq/1000000);
	printf ("sysclk_freq (MHz) : %9u\n\r",sysclk_freq/1000000);	USART2_txint_send();
/* ---------------- Parameter ------------------------------------------------------------------------ */
	ret = yogurt_a_function_init();
	if (ret <= 0)
	{
		printf("yogurt_a_function: table size mismatch: %d\n\r", -ret);USART2_txint_send(); 
		while(1==1);
	}
	printf("yogurt_a_function: table size : %d\n\r", ret);USART2_txint_send();
/* --------------------- Get Loader CAN ID -------------------------------------------------------------- */
	/* Pick up the unique CAN ID stored when the loader was flashed. */
	u32 canid_ldr = *(u32*)((u32)((u8*)*(u32*)0x08000004 + 7 + 0));	// First table entry = can id
	printf(  "CANID_LDR  : 0x%08X\tCAN loader CAN ID\n\r", (unsigned int)canid_ldr );USART2_txint_send();
/* --------------------- CAN setup ---------------------------------------------------------------------- */
	/* Configure and set MCP 2551 driver: RS pin (PD 11) on POD board */
//	can_nxp_setRS_sys(0,0); // (1st arg) 0 = high speed mode; 1 = standby mode (Sets yellow led on)
	/* Initialize CAN for POD board (F103) and get control block */
	// Set hardware filters for FIFO1 high priority ID & mask, plus FIFO1 ID for this UNIT
	pctl1 = canwinch_setup_F103_Olimex(&msginit, canid_ldr); // ('can_ldr' is fifo1 reset CAN ID)

	/* Check if initialization was successful. */
	if (pctl1 == NULL)
	{
		printf("CAN1 init failed: NULL ptr\n\r");USART2_txint_send(); 
		while (1==1);
	}
	if (pctl1->ret < 0)
	{
		printf("CAN init failed: return code = %d\n\r",pctl1->ret);USART2_txint_send(); 
		while (1==1);
	}
/* ------------------ Get ADC initialized and calibrated ---------------------------------------------- */
	adcsensor_yogurt_sequence();	// Initialization sequence for the adc
/* ------------------ TIM3 for PWM triac pin PB0 ------------------------------------------------------ */
	tim3_yog_init();	//Initialize TIM3 for PWM and other timing (1 frame per sec)
	tim3_yog_setpwm(32000);	// Set PWM tick count full off
/* ------------------ TIM4 for PWM fan pin PB8 -------------------------------------------------------- */
	tim4_yog_init();	//Initialize TIM4 for PWM fan voltage control (5 or more Khz)
	tim4_yog_setpwm(000);	// Set PWM tick count full off
/* ------------------ Heat/cool ----------------------------------------------------------------------- */
	loopctl_init(78.0);	// Init it
/* ----------------- Debug parameters ----------------------------------------------------------------- */
	idx_v_struct_print(&thm);
/* ---------------- Some vars associated with endless loop ------------------------------------------- */
printf("PROGRESS CHECK 1\n\r");USART2_txint_send(); 
	loopctl_print_hdr();

extern uint32_t	adc_readings_cic[2][NUMBERADCCHANNELS_TEN]; // Filtered/decimated ADC readings
int adc_temp_flag_prev = adc_temp_flag[0]; 	// Thermistor readings ready counter
//char s[32];					// Float to ascii (since %f is not working)
double dscale = 1.0 / (1 << 18);		// ADC filtering scale factor (reciprocal multiply)
double temptemp;	// Temperature temporary (or temporary temperature...)

/* SEQUENCE: 
	0 PC0
	1 PC1
	2 PC2
	3 PC3 */

double dtmp;

const struct THERMPARAM2* ptp[] = {\
	&thm.tp[0],
	&thm.tp[1],
	&thm.tp[2],
	&thm.tp[3],
};
struct LOOPCTL_STATE* ploopctl;
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


/* --------------------- Endless Stuff ----------------------------------------------- */
	while (1==1)
	{
tdiff = (int)DTWTIME - (int)t0;
if (tdiff > tdiff_prev)
{
 tdiff_prev = tdiff;
 if ((tdiff > tmax) || (qflag != 0) )
 {
   printf("T %d %d %d %d %d %d\n\r",tdiff, q1max, q2max, q3max, q4max, q5max);USART2_txint_send();
   tmax = tdiff; qflag = 0;
 }
}
t0 = DTWTIME;
		/* Poll loop flashing of LED. */
		OlimexLED_togglepoll();

		/* Deal with PC incoming lines w commands. */
		yogPC_cmd_poll();

		/* Handler thermistor-to-temperature conversion, and AD7799 temperature correction. */
		if (adc_temp_flag[0] != adc_temp_flag_prev)
		{ // Here, a new set of thermistor readings are ready
			j = (0x1 & adc_temp_flag_prev);	// Get current double buffer index
			adc_temp_flag_prev = adc_temp_flag[0];
ticks += 1;
q5 = DTWTIME;
			for (i = 0; i < NUMBERADCCHANNELS_TEN; i++)
			{
q0 = DTWTIME;
				dtmp = adc_readings_cic[j][i]; // Convert to double
				dtmp *= dscale;	// Scale to 0-4095
	
				/* Convert ADC readings into uncalibrated degrees Centigrade. */
				temptemp = temp_calc_param_dbl(dtmp, (struct THERMPARAM2*)ptp[i]);
q1 = DTWTIME;
				/* Apply calibration to temperature. */
				therm[i] = compensation_dbl((float*)&ptp[i]->poly[0], 4, temptemp);
				therm[i] = (therm[i] * (9.0/5.0)) + 32.0; // Convert to F
q2 = DTWTIME;
q3 = DTWTIME;
				thermf[i] = therm[i]; 	// Save as float
				thermi[i] = (therm[i] * 1000.0);	// Save as int * 1000

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
			ploopctl = loopctl_poll(therm[thm.thmidx_pot], therm[thm.thmidx_shell], therm[thm.thmidx_airin] );	// Update control loop
			loopctl_print(ploopctl);

q6 = DTWTIME;
qdiff = (int)q6 -(int)q5;
if (qdiff > q5max) {q5max = qdiff; qflag = 1;}

			USART2_txint_send(); // Print the foregoing
		}
	}
	return 0;	
}


