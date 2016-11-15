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
#include "tension_a_functionS.h"
#include "tension_a_printf.h"
#include "temp_calc_param.h"
#include "ad7799_ten_comm.h"
#include "ad7799_filter_ten2.h"
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
#include "../../../../svn_common/trunk/common_highflash.h"

#include "fmtprint.h"
#include "iir_filter_l.h"
#include "tension_idx_v_struct.h"
#include "tim3_ten2.h"
#include "can_filter_print.h"

// Float to ascii (since %f is not working)
char s[NUMBERADCTHERMISTER_TEN][32];

/* Print out line count */
static unsigned int linect;


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

void double_to_char(double f,char * buffer){
    gcvt(f,12,buffer);
}

/*#################################################################################################
And now for the main routine 
  #################################################################################################*/
int main(void)
{
	int i;
//	int j;
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
	USART1_txint_puts("\n\rTENSION 05-15-2016\n\r");USART1_txint_send();

	/* Display things so's to entertain the hapless op */
	printf ("  hclk_freq (MHz) : %9u\n\r",  hclk_freq/1000000);
	printf (" pclk1_freq (MHz) : %9u\n\r", pclk1_freq/1000000);
	printf (" pclk2_freq (MHz) : %9u\n\r", pclk2_freq/1000000);
	printf ("sysclk_freq (MHz) : %9u\n\r",sysclk_freq/1000000);	USART1_txint_send();

#ifdef TESTINGTHEFPMESS
unsigned int k1,k2;
char ttbuf[96];
double ttf = -0.49999;

double_to_char(ttf, &ttbuf[0]);
printf("TTF: %s\n\r",ttbuf);USART1_txint_send();

k1 = DTWTIME;fpformat(&ttbuf[0],ttf);k2 = (int)DTWTIME-(int)k1;
printf("TT1: %s %10d\n\r",ttbuf, k2);USART1_txint_send();

ttf = 0.49999;
double_to_char(ttf, &ttbuf[0]);
printf("TTF: %s\n\r",ttbuf);USART1_txint_send();

k1 = DTWTIME;fpformat(&ttbuf[0],ttf);k2 = (int)DTWTIME-(int)k1;
printf("TTF: %s\n\r",ttbuf);USART1_txint_send();


ttf = 5000000000.001;
printf("\n\r");

double_to_char(ttf, &ttbuf[0]);
printf("TTF: %s\n\r",ttbuf);USART1_txint_send();

k1 = DTWTIME;fpformat(&ttbuf[0],ttf);k2 = (int)DTWTIME-(int)k1;
printf("TT1: %s %10d\n\r",ttbuf, k2);USART1_txint_send();

ttf = -45678912345.6789;
printf("\n\r");

double_to_char(ttf, &ttbuf[0]);
printf("TTF: %s\n\r",ttbuf);USART1_txint_send();

k1 = DTWTIME;fpformat(&ttbuf[0],ttf);k2 = (int)DTWTIME-(int)k1;
printf("TT1: %s %10d\n\r",ttbuf, k2);USART1_txint_send();


ttf = 0;

printf("\n\r");

double_to_char(ttf, &ttbuf[0]);
printf("TTF: %s\n\r",ttbuf);USART1_txint_send();

k1 = DTWTIME;fpformat(&ttbuf[0],ttf);k2 = (int)DTWTIME-(int)k1;
printf("TT1: %s %10d\n\r",ttbuf, k2);USART1_txint_send();


printf("DONE\n\r");USART1_txint_send();
while(1==1);

#endif


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
//	u32 canid_ldr = *(u32*)((u32)((u8*)*(u32*)0x08000004 + 7 + 0));	// First table entry = can id
	u32 canid_ldr = pfunc[0].func;
	printf("CANID_LDR  : 0x%08X\n\r", (unsigned int)canid_ldr );
	printf("TBL SIZE   : %d\n\r",(unsigned int)pfunc[0].canid);
	USART1_txint_send();

/* --------------------- CAN setup ---------------------------------------------------------------------- */
	/* Configure and set MCP 2551 driver: RS pin (PD 11) on POD board */
	can_nxp_setRS_sys(0,0); // (1st arg) 0 = high speed mode; 1 = standby mode (Sets yellow led on)
	GPIO_BSRR(GPIOE) = (0xf<<LED3);	// Set bits = all four LEDs off

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
		ret = can_driver_filter_add_one_32b_id(0,id,0);
		if (ret < 0)
		{
			printf("FLASHH CAN id table load failed: %d\n\r",ret);USART1_txint_send(); 
			while (1==1);
		}
	}
/* ---------------- tension A ------------------------------------------------------------------------ */
	ret = tension_a_functionS_init_all();
	if (ret <= 0)
	{
		printf("tension_a_functionS: table size mismatch count: %d\n\r", ret);USART1_txint_send(); 
		while(1==1);
	}
	printf("tension_a_functionS: table size : %d\n\r", ret);USART1_txint_send();

/* ----------------- CAN filter registers ------------------------------------------------------------- */
	can_filter_print(14);	// Print the CAN filter registers
/* ----------------- Debug parameters ----------------------------------------------------------------- */
for (i = 0; i < NUMTENSIONFUNCTIONS; i++)
{
	printf("\n\rAD7799 #%1d values\n\r",i+1);
	tension_a_printf(&ten_f[i].ten_a);	// Printf first AD7799 set of parameters
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

//extern uint32_t	adc_readings_cic[2][NUMBERADCTHERMISTER_TEN]; // Filtered/decimated ADC readings
//int adc_temp_flag_prev = adc_temp_flag[0]; // Thermistor readings ready counter
//double therm[NUMBERADCTHERMISTER_TEN];	// Floating pt of thermistors readings


for (i = 0; i < NUMBERADCTHERMISTER_TEN; i++){s[i][0]= '.'; s[i][1]=0;}

/* Start average */
ten_f[0].ave.run = 1;
ten_f[1].ave.run = 1;

#define CANDRIVERPRINTFCT 10	// Number of LED ticks between printing can_driver error counts
unsigned int cdect = 0;		// Counter for timing printing of can_driver errors


//3 unsigned int tim3_ten2_ticks_prev = 0;
//#define READINGSINC	470	// Number of readings between recalibrations
//uint32_t readingsct_next = ten_f[0].readingsct + READINGSINC;
//unsigned int tsec = 0;

/* --------------- Start TIM3 CH1 and CH2 interrupts ------------------------------------------------- */
	tim3_ten2_init(pclk1_freq/2048);	// 64E6/2048

/* --------------------- Endless Stuff ----------------------------------------------- */
	while (1==1)
	{
		/* ---------- Tick time flashing (in case 'ad7799_ten_flag' not working) ---------- */
		if ( ((int)ledtime - (int)DTWTIME) < 0) 
		{
			ledtime += FLASHTIMEINC;
			toggle_1led(LEDYELLOW);

			/* Display can_driver error counts periodically. */
			cdect += 1;
			if (cdect >= CANDRIVERPRINTFCT)
			{
				cdect = 0;
				can_driver_errors_printf(pctl0);
				can_driver_errortotal_printf(pctl0);
			}

//$			printf("SEC %6d\n\r",tsec++);
//3			printf("TIC %d\n\r",(tim3_ten2_ticks-tim3_ten2_ticks_prev));USART1_txint_send();
//3			tim3_ten2_ticks_prev = tim3_ten2_ticks;
//#ifdef ONCEPERSECONDMONITORING
			/* Once per second output. */
			ciccalibrateprint(0);		// Calibrate and print 1st AD7799
			if ( AD7799_num != 1)	// Just one AD7799?
			{ // No.  Two AD7799s initialized OK
				ciccalibrateprint(1);	// Calibrate and print 2nd AD7799
			}
			/* Add thermistor temperatures and line count (seconds) to line and start it printing. */
			printf("%s %s %s %s",&s[0][0],&s[1][0],&s[2][0],&s[3][0]);
			printf("%8d\n\r", linect++);
			USART1_txint_send();
//#endif
		}

//		/* ------------ Periodically execute a recalibration ------------------------ */
//		if (ten_f[0].readingsct >= readingsct_next)
//		{
//			readingsct_next += READINGSINC;
//			ad7799_poll_rdy_ten2_req_calib(0);	// Request a re-calib sequence
//			ad7799_poll_rdy_ten2_req_calib(1);	// Request a re-calib sequence
//		}
#ifdef OFFSETMONITORINGONTHECONSOLE
		for (i = 0; i < NADCS; i++)
		{
			if (ten_f[i].zero_flag != 0)
			{
				ten_f[i].zero_flag = 0;
				printf ("OFFREG%1d %4d %8d %8d %8d\n\r",i, (int)(ten_f[i].offset_reg - 0x00800000),(int)(ten_f[i].iir_z_recal_w.z / ten_f[i].iir_z_recal_w.pprm->scale),\
				(int)(ten_f[i].offset_reg_filt - 0x00800000), (int)(ten_f[i].offset_reg_rdbk - 0x00800000) );
			}
		}
#endif
		/* ---------- Check and update thermistor temperature readings. ---------- */
		ret =  tension_a_functionS_temperature_poll();
		if (ret != 0)
		{ // Here, there was an update.  Format for a later printf.
#ifdef DEBUGCICFILTERING
extern struct ADCDR_TENSION  strADC1dr;
int w = adc_temp_flag[0] & 0x1;
w ^= 0x1;
static int b;
int r=0;
printf("W %d %d %d %d\n\r",strADC1dr.in[w][b][0],strADC1dr.in[w][b][1],strADC1dr.in[w][b][2],strADC1dr.in[w][b][3]);
b += 1; if (b >= 16) b = 0;
printf("T %d %d %d %d\n\r",adc_readings_cic[r][0]/262144,adc_readings_cic[r][1]/262144,adc_readings_cic[r][2]/262144,adc_readings_cic[r][3]/262144);
r += 1; if ( r >=2) r = 0;
#endif
			/* Setup temperatures in ascii strings for later printf'g. */
			fpformat(&s[0][0],ten_f[0].degC[0]);	// Float-to-ascii (%6.3f)
			fpformat(&s[1][0],ten_f[0].degC[1]);	// Float-to-ascii (%6.3f)
			fpformat(&s[2][0],ten_f[1].degC[0]);	// Float-to-ascii (%6.3f)
			fpformat(&s[3][0],ten_f[1].degC[1]);	// Float-to-ascii (%6.3f)
		}
#ifdef ONCEPERSECONDALTERNATIVE
		/* ---------- CIC Filtered AD7799 readings ----------*/
		if (cic[0][1].Flag != cic[0][1].Flag_prev) // Use 2nd CIC flag
		{
			cic[0][1].Flag_prev = cic[0][1].Flag; // Reset flag
			ciccalibrateprint(0);		// Calibrate and print

			if ( AD7799_num != 1)	// Just one AD7799?
			{ // No.  Two AD7799s initialized OK
				ciccalibrateprint(1);	// Calibrate and print
			}
		}
#endif
	/* ---------- Trigger a pass through 'CAN_poll' to poll msg handling & sending. ---------- */
	CAN_poll_loop_trigger();
	}
	return 0;	
}
/* **************************************************************************************
 * double iir_filtered_calib(struct TENSIONFUNCTION* p, uint32_t n); 
 * @brief	: Convert integer IIR filtered value to offset & scaled double
 * @param	: p = pointer to struct with "everything" for this AD7799
 * @return	: offset & scaled
 * ************************************************************************************** */
static double iir_filtered_calib(struct TENSIONFUNCTION* p, uint32_t n)
{
	double d;
	double s;
	double dcal;

	/* Scale filter Z^-1 accumulator. */
	int32_t tmp = p->iir_filtered[n].z / p->iir_filtered[n].pprm->scale;
	
	/* Apply offset. */
	tmp += p->ten_a.ad.offset;
	
	/* Convert to double and scale. */
	d = tmp; 		// Convert scaled, filtered int to double
	s = p->ten_a.ad.scale; 	// Convert float to double
	dcal = d * s;		// Calibrated reading
	p->dcalib_lgr = dcal;	// Save calibrated last good reading

	/* Set up status byte for reading. */
	p->status_byte = STATUS_TENSION_BIT_NONEW; // Show no new readings
	
	/* New readings flag. */
	if (p->readingsct != p->readingsct_lastpoll)
	{ // Here, there was a new reading since last poll
		p->status_byte = 0;	// Turn (all) flags off
	}

	/* Check reading against limits. */
	if (dcal > p->ten_a.limit_hi)
	{
		p->status_byte |= STATUS_TENSION_BIT_EXCEEDHI;
		return dcal;
	}
	if (dcal < p->ten_a.limit_lo)
	{
		p->status_byte |= STATUS_TENSION_BIT_EXCEEDLO;
		return dcal;
	}
	return dcal;		// Return scaled value
}
/* **************************************************************************************
 * void ciccalibrateprint(int n); 
 * @brief	: CIC reading calibration and printout
 * @param	: n = tension_a instance index 0 or 1
 * ************************************************************************************** */
extern unsigned int debug9;

#define AVEACCUMLIMIT 16
void ciccalibrateprint(int n)
{
	int ntmp;
	long long lltmp;
	double scaled;
	char sss[32];

	lltmp 	= cic[n][1].llout_save; 	// [AD7799 #n][2nd cic stage]
	ntmp 	= lltmp/(1<<19);		// Filter rescaling

	if (ten_f[n].ave.run != 0)
	{
		if ((ten_f[n].ave.run_prev == 0) || (ten_f[n].ave.n >= AVEACCUMLIMIT))
		{ // Here, changed from not run to run. Initialize
			ten_f[n].ave.run_prev = ten_f[n].ave.run;
			ten_f[n].ave.sum = 0;
			ten_f[n].ave.n = 0;
//	printf("AD%1d %8d %8d %8d %8d\n\r",(n+1),(int)ten_f[n].cicraw, ntmp,(int)ten_f[n].ave.a, (int)ten_f[n].ave.n );

		}
		ten_f[n].ave.sum += ntmp;	// Sum readings
		ten_f[n].ave.n += 1;	// Count number
		lltmp = ten_f[n].ave.sum/ten_f[n].ave.n;
		ten_f[n].ave.a = lltmp;	// Convert to int and save in struct
	}

	/* CIC filtered tension reading. */
	ten_f[n].cicraw = ntmp;			// AD7799 #n raw reading
	ntmp 	+= ten_f[n].ten_a.ad.offset;
	scaled 	= ntmp;				// Convert int to double
	scaled *= ten_f[n].ten_a.ad.scale;
	fpformat(&sss[0],scaled);	// Float-to-ascii (%6.3f)

	/* IIR filtered tension reading. */
	char ssd[32];
	double diir;
	diir = iir_filtered_calib(&ten_f[n], 1); // Calibrate IIR (n=1 for slow)
	fpformat(&ssd[0],diir);	// Float-to-ascii (%6.3f)
	
//	printf("-%1d- %8d %8d %s %8d %4d ",(n+1),(int)ten_f[n].cicraw,ntmp,&sss[0], (int)ten_f[n].ave.a, (int)ten_f[n].ave.n );
//	printf("%1d %8d %8d %s \t",(n+1),(int)ten_f[n].cicraw,ntmp,&sss[0]);
//	printf("%1d %8d %8d %s %s\t",(n+1),(int)ten_f[n].cicraw,ntmp,&sss[0],&ssd[0]); // w CIC
	printf("%1d %8d %8d %s %02x\t",(n+1),(int)ten_f[n].cicraw,ntmp,&ssd[0],ten_f[n].status_byte); // w/o CIC

	return;
}
