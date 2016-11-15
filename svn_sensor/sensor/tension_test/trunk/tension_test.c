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
04/24/2016: I beseech you to read README.tension, and you
may be better informed than just the following...then maybe not.

07/xx/2015:
After intialization--
  main: endless loop
    computes temperatures from thermistor readings
    computes tension offset adjustment factor
    printf for monitoring
    triggers a low level interrupt to poll send/handle CAN msgs
  timer3: interrupts 2048/sec
    Runs "last ad7799 reading" through a filter.
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

#include "PODpinconfig.h"
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
#include "tension_idx_v_struct.h"
#include "tension_a_function.h"
#include "temp_calc_param.h"
#include "ad7799_ten_comm.h"
#include "ad7799_filter_ten.h"
#include "CAN_poll_loop.h"
#include "libmiscstm32/DTW_counter.h"
#include "adcsensor_tension.h"
#include "can_nxp_setRS.h"
#include "p1_initialization.h"
#include "can_driver.h"
#include "can_driver_filter.h"
#include "can_msg_reset.h"
#include "can_gps_phasing.h"
#include "sensor_threshold.h"
#include "panic_leds_pod.h"
#include "db/gen_db.h"
#include "CAN_poll_loop.h"

#include "adcsensor_tension.h"
#include "spi1sad7799_ten.h"

/* ############################################################################# */
#define USEDEFAULTPARAMETERS	0	// 0 = Initialize sram struct with default parameters
/* ############################################################################# */

/* Subroutine prototypes */
void fpformat(char* p, double d);
void fmtprint(int i, float f,char* p);

int dummy;

/* Make block that can be accessed via the "jump" vector (e.g. 0x08004004) that points
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

/* Easy way for other routines to access via 'extern'*/
struct CAN_CTLBLOCK* pctl1;

/* Specify msg buffer and max useage for CAN1: TX, RX0, and RX1. */
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
//void putc ( void* p, char c)
//	{
//		p=p;	// Get rid of the unused variable compiler warning
//		CANascii_putc(c);
//		USART1_txint_putc(c);
//	}
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
	clockspecifysetup(canwinch_setup_F103_pod_clocks() );
/* ---------------------- Set up pins ------------------------------------------------------------- */
	PODgpiopins_default();	// Set gpio port register bits for low power
	PODgpiopins_Config();	// Now, configure pins
	MAX3232SW_on;		// Turn on RS-232 level converter (if doesn't work--no RS-232 chars seen)

	/* Use DTW_CYCCNT counter for startup timing */
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
	USART1_txint_puts("\n\rTENSION 07-25-2015\n\r");USART1_txint_send();

	/* Display things so's to entertain the hapless op */
	printf ("  hclk_freq (MHz) : %9u\n\r",  hclk_freq/1000000);
	printf (" pclk1_freq (MHz) : %9u\n\r", pclk1_freq/1000000);
	printf (" pclk2_freq (MHz) : %9u\n\r", pclk2_freq/1000000);
	printf ("sysclk_freq (MHz) : %9u\n\r",sysclk_freq/1000000);	USART1_txint_send();
/* ---------------- tension A ------------------------------------------------------------------------ */
	ret = tension_a_function_init();
	if (ret <= 0)
	{
		printf("tension_a_function: table size mismatch: %d\n\r", -ret);USART1_txint_send(); 
		while(1==1);
	}
	printf("tension_a_function: table size : %d\n\r", ret);USART1_txint_send();
/* --------------------- Get Loader CAN ID -------------------------------------------------------------- */
	/* Pick up the unique CAN ID stored when the loader was flashed. */
	u32 canid_ldr = *(u32*)((u32)((u8*)*(u32*)0x08000004 + 7 + 0));	// First table entry = can id
	printf(  "CANID_LDR  : 0x%08X\n\r", (unsigned int)canid_ldr );	USART1_txint_send();
/* --------------------- CAN setup ---------------------------------------------------------------------- */
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
/* ------------------------ Setup CAN hardware filters ------------------------------------------------- */
	can_msg_reset_init(pctl1, canid_ldr);	// Specify CAN ID for this unit for msg caused RESET
	
	// Add CAN IDs this function will need *from* the CAN bus.  Hardware filter rejects all others. */
	//                                      ID #1                    ID#2                 FIFO  BANK NUMBER
	ret  = can_driver_filter_add_two_32b_id(CANID_CMD_TENSION_1a,     CANID_MSG_TIME_POLL,   0,     2);
	ret |= can_driver_filter_add_two_32b_id(CANID_CMD_CABLE_ANGLE_1, CANID_DUMMY,           0,     3);
	if (ret != 0)
	{
		printf("filter additions: failed init\n\r");USART1_txint_send(); 
		while (1==1);		
	}
/* ------------------------ Various and sundry ------------------------------------------------------- */
	p1_initialization();
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

extern uint32_t	adc_readings_cic[2][NUMBERADCCHANNELS_TEN]; // Filtered/decimated ADC readings
int adc_temp_flag_prev = adc_temp_flag[0]; // Thermistor readings ready counter
double therm[NUMBERADCCHANNELS_TEN];	// Floating pt of thermistors readings

// Float to ascii (since %f is not working)
char s[NUMBERADCCHANNELS_TEN][32];
for (i = 0; i < NUMBERADCCHANNELS_TEN; i++){s[i][0]= '.'; s[i][1]=0;}

double dscale = 1.0 / (1 << 18);	// ADC filtering scale factor (reciprocal multiply)
double temptemp;	// Temperature temporary (or temporary temperature...)
double tempcalib;	// Calibrated temperature

/* SEQUENCE: For the test unit--
	0 AD7799 #1
	1 red/wht pair (inner most pair)
	2 AD7799 #2
	3 yel/wht pair */
float ten_b_tmp_comp1[] = {5.0, 1.133, 0.0, 0.0};
float ten_b_tmp_comp2[] = {5.0, 1.133, 0.0, 0.0};

float const* pth[] = {\
	&ten_a.ad.comp_t1[0],
	&ten_a.ad.comp_t2[0],
	&ten_b_tmp_comp1[0],
	&ten_b_tmp_comp2[0],
};


/* ----------------- Debug parameters ----------------------------------------------------------------- */
/* DEBUG code: Check values in struct */
i = 0;
printf("%2d	%d	%s\n\r",   i + 0, (unsigned int)ten_a.size,"  0 Number of elements in the following list");
printf("%2d	%d	%s\n\r",   i + 1, (unsigned int)ten_a.crc,"  1 Tension_1: CRC for tension list");
printf("%2d	%d	%s\n\r",   i + 2, (unsigned int)ten_a.version," 2 Version number");
printf("%2d	%d	%s\n\r",   i + 3, (unsigned int)ten_a.ad.offset," 3 TENSION_AD7799_1_OFFSET,	Tension: AD7799 offset");
fmtprint(i + 4, ten_a.ad.scale * 100," 4 TENSION_AD7799_1_SCALE,	Tension: AD7799 #1 Scale (convert to kgf)");
fmtprint(i + 5, ten_a.ad.tp[0].B," 5 TENSION_THERM1_CONST_B,	Tension: Thermistor1 param: constant B");
fmtprint(i + 6, ten_a.ad.tp[0].R0," 6 TENSION_THERM1_R_SERIES,	Tension: Thermistor1 param: Series resistor, fixed (K ohms)");
fmtprint(i + 7, ten_a.ad.tp[0].RS," 7 TENSION_THERM1_R_ROOMTMP,	Tension: Thermistor1 param: Thermistor room temp resistance (K ohms)");
fmtprint(i + 8, ten_a.ad.tp[0].TREF,"  8 TENSION_THERM1_REF_TEMP,	Tension: Thermistor1 param: Reference temp for thermistor");
fmtprint(i + 9, ten_a.ad.tp[0].offset,"  9 TENSION_THERM1_TEMP_OFFSET,Tension: Thermistor1 param: Thermistor temp offset correction (deg C)");
fmtprint(i +10, ten_a.ad.tp[0].scale," 10 TENSION_THERM1_TEMP_SCALE,Tension: Thermistor1 param: Thermistor temp scale correction");
fmtprint(i +11, ten_a.ad.tp[1].B," 11 TENSION_THERM2_CONST_B,	Tension: Thermistor2 param: constant B");
fmtprint(i +12, ten_a.ad.tp[1].RS," 12 TENSION_THERM2_R_SERIES,	Tension: Thermistor2 param: Series resistor, fixed (K ohms)");
fmtprint(i +13, ten_a.ad.tp[1].R0," 13 TENSION_THERM2_R_ROOMTMP,	Tension: Thermistor2 param: Thermistor room temp resistance (K ohms)");
fmtprint(i +14, ten_a.ad.tp[1].TREF," 14 TENSION_THERM2_TEMP_OFFSET,Tension: Thermistor2 param: Thermistor temp offset correction (deg C)");
fmtprint(i +15, ten_a.ad.tp[1].offset," 15 TENSION_THERM2_TEMP_OFFSET,Tension: Thermistor2 param: Thermistor temp offset correction (deg C)");
fmtprint(i +16, ten_a.ad.tp[1].scale," 16 TENSION_THERM2_TEMP_SCALE,Tension: Thermistor2 param: Thermistor temp scale correction");
fmtprint(i +17, ten_a.ad.comp_t1[0]," 17 TENSION_THERM1_LC_COEF_0,	Tension: Thermistor1 param: Load-Cell polynomial coefficient 0 (offset)");
fmtprint(i +18, ten_a.ad.comp_t1[1]," 18 TENSION_THERM1_LC_COEF_1,	Tension: Thermistor1 param: Load-Cell polynomial coefficient 1 (scale)");
fmtprint(i +19, ten_a.ad.comp_t1[2]," 19 TENSION_THERM1_LC_COEF_2,	Tension: Thermistor1 param: Load-Cell polynomial coefficient 2 (x^2)");
fmtprint(i +20, ten_a.ad.comp_t1[3]," 20 TENSION_THERM1_LC_COEF_3,	Tension: Thermistor1 param: Load-Cell polynomial coefficient 3 (x^3)");
fmtprint(i +21, ten_a.ad.comp_t2[0]," 21 TENSION_THERM2_LC_COEF_0,	Tension: Thermistor2 param: Load-Cell polynomial coefficient 0 (offset)");
fmtprint(i +22, ten_a.ad.comp_t2[1]," 22 TENSION_THERM2_LC_COEF_1,	Tension: Thermistor2 param: Load-Cell polynomial coefficient 1 (scale)");
fmtprint(i +23, ten_a.ad.comp_t2[2]," 23 TENSION_THERM2_LC_COEF_2,	Tension: Thermistor2 param: Load-Cell polynomial coefficient 2 (x^2)");
fmtprint(i +24, ten_a.ad.comp_t2[3]," 24 TENSION_THERM2_LC_COEF_3,	Tension: Thermistor2 param: Load-Cell polynomial coefficient 3 (x^3)");
printf("%2d	%d	%s\n\r",   i +25, (unsigned int)ten_a.hbct," 25 TENSION_HEARTBEAT_CT	Tension: hbct: Heart-Beat Count of time ticks between autonomous msgs");
printf("%2d	%d	%s\n\r",   i +26, (unsigned int)ten_a.drum," 26 TENSION_DRUM_NUMBER	Tension: drum: Drum system number for this function instance");
printf("%2d	0x%02X	%s\n\r",   i +27, (unsigned int)ten_a.f_pollbit," 27 TENSION_DRUM_FUNCTION_BIT	Tension: bit: f_pollbit: Drum system poll 1st byte bit for function instance");
printf("%2d	0x%02X	%s\n\r",   i +28, (unsigned int)ten_a.p_pollbit," 28 TENSION_DRUM_POLL_BIT	Tension: bit: p_pollbit: Drum system poll 2nd byte bit for this type of function");
printf("%2d	0x%08X	%s\n\r",   i +29, (unsigned int)ten_a.cid_ten_msg," 29 TENSION_DRUM_POLL_BIT	Tension: CANID: cid_ten_msg:  canid msg Tension");
printf("%2d	0x%08X	%s\n\r",   i +30, (unsigned int)ten_a.cid_ten_poll," 30 TENSION_DRUM_FUNCTION_BIT	Tension: CANID: cid_ten_msg:  canid MC: Time msg/Group polling");
printf("%2d	0x%08X	%s\n\r",   i +31, (unsigned int)ten_a.cid_gps_sync," 31 TENSION_TIMESYNC		Tension: CANID: cid_gps_sync: canid time: GPS time sync distribution");
USART1_txint_send(); 

/* Test stuff */
int ntmp1,ntmp2;
long long lltmp;
unsigned int usPrev = 0;
unsigned int tick = 0;
extern unsigned int tim3_ticks;
/*
struct CANRCVBUF		// Combine CAN msg ID and data fields
{ //                               offset  name:     verbose desciption
	u32 id;			// 0x00 CAN_TIxR: mailbox receive register ID p 662
	u32 dlc;		// 0x04 CAN_TDTxR: time & length p 660
	union CANDATA cd;	// 0x08,0x0C CAN_TDLxR,CAN_TDLxR: Data payload (low, high)
};
*/
struct CANRCVBUF cantest = {0xa4000000,8,{1}};
void cantest1(unsigned int id, int x)
{
	cantest.id = id;	// Set up CAN msg
	cantest.dlc = 4;
	cantest.cd.ui[0] = x;
	can_driver_put(pctl1,&cantest,4,0);	// Add/send to CAN driver
	return;
}

//struct THERMPARAM2 thrm = {3380000,10,10, 290};

/* --------------------- Endless Stuff ----------------------------------------------- */
	while (1==1)
	{
		/* Tick time flashing (in case 'ad7799_ten_flag' not working) */
		if ( ((int)ledtime - (int)DTWTIME) < 0) 
		{
			ledtime += FLASHTIMEINC;
			toggle_1led(LEDYELLOW);
//			printf("TIC %4d %d\n\r",tick++, (tim3_ticks-usPrev));
			usPrev = tim3_ticks;
		}

		/* Filtered AD7799-a readings */
		if (cic[0][1].Flag != cic[0][1].Flag_prev)
		{
			cic[0][1].Flag_prev = cic[0][1].Flag;
			lltmp = cic[0][1].llout_save;
			ntmp1 = lltmp/(1<<19);
cantest1(0xb4000000,ntmp1);	// Add/send to CAN driver

			if ( AD7799_num == 1)	// Just one AD7799?
			{ // Yes.  Only one successfully initialized
				// Raw ADC; AD7799_1 temp; Load-cell #1 temp
				printf("ADC %9d %s %s\n\r",ntmp1,&s[2][0],&s[3][0]);
			}
			else
			{ // No.  Two AD7799s initialized OK
			lltmp = cic[1][1].llout_save;
			ntmp2 = lltmp/(1<<19);
			// Raw ADC; AD7799_2 temp; Load-cell #2 temp
			printf("ADC %9d %s %s %9d %s %s\n\r",ntmp1,&s[2][0],&s[3][0],ntmp2,&s[0][0],&s[1][0]);
cantest1(0xb5000000,ntmp2);	// Add/send to CAN driver

			}
		}

		/* Handler for thermistor-to-temperature conversion, and AD7799 temperature correction. */
		if (adc_temp_flag[0] != adc_temp_flag_prev)
		{ // Here, a new set of thermistor readings are ready
			j = (0x1 & adc_temp_flag_prev);	// Get current double buffer index
			adc_temp_flag_prev = adc_temp_flag[0];
			for (i = 0; i < NUMBERADCCHANNELS_TEN; i++)
			{	/* SEQUENCE: For the test unit--
				i column
				0 AD7799 #1
				1 red/wht pair (inner most pair)
				2 AD7799 #2
				3 yel/wht pair 
				*/
				therm[i] = adc_readings_cic[j][i]; // Convert to double
				therm[i] = therm[i] * dscale;	// Scale to 0-4095
//				fpformat(&s[i][0],therm[i]);	// Float-to-ascii (%6.3f)
//				printf("A%s",&s[i][0]);	// 
	
				/* Convert ADC readings into uncalibrated degrees Centigrade. */
				temptemp = temp_calc_param_dbl(therm[i], &ten_a.ad.tp[0]);
//				fpformat(&s[i][0],temptemp);	// Float-to-ascii (%6.3f)
//				printf("B%s ",&s[i][0]);	// 

				/* Apply calibration to temperature. */
				tempcalib = compensation_dbl(pth[i], 4, temptemp);
				fpformat(&s[i][0],tempcalib);	// Float-to-ascii (%6.3f)
				// NOTE: formated string saved for above ADC printout
//				printf("%s",&s[i][0]);	// 

				// TODO compute AD7799 offset/scale factors
			}
//			printf("\n\r");USART1_txint_send(); // Print the foregoing
			toggle_1led(LEDGREEN2);	// Let the op know the ADC stuff is alive
		}
	/* Trigger a pass through 'CAN_poll' to poll msg handling & sending. */
	CAN_poll_loop_trigger();
	}
	return 0;	
}

void fmtprint(int i, float f, char* p)
{
	char w[64];
	double d = f;
	fpformat(w,d);
	printf("%02d\t%s %s\n\r",i,w,p);
	return;
}
void fpformat(char* p, double d)
{
	int i = d;	// Get whole part
	int j = i;
	sprintf(p, "%5i.",i);	// Convert whole part
	if (j < 0) j = -j;  if (d < 0) d = -d;
	double f = (d * 1000) - (j * 1000); // f = fractional part
	sprintf((p+6),"%03i",(int)f);
	return;
}

