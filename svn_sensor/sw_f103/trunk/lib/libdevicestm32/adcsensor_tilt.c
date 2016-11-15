/******************************************************************************
* File Name          : adcsensor_tilt.h
* Date First Issued  : 02/12/2015
* Board              : POD
* Description        : ADC routines for f103 pod board accelerometer for tilt
*******************************************************************************/
/* 


04/20/2014
CAN messages:


11/11/2012 This is a hack of svn_pod/sw_stm32/trunk/devices/adcpod.c
See p214 of Ref Manual for ADC section 
02/12/2015 This is a hack of adcsensor_tilt.c

*/
/*
NOTE: Some page number refer to the Ref Manual RM0008, rev 11 and some more recent ones, rev 14.

Strategy--
Three ADC pins are read at a rapid rate with the DMA storing the sequence.  The DMA wraps around
at the end of the buffer.  The code for DMA interrupts is not used.  The 2048 per sec SYSTICK
interrupts in 'canwinch_pod_common_systick2048.c' comes (under high priority interrupt) to 
'adcsensor_tilt_bufadd' where the latest adc readings read are stored in a buffer.  Next, under
low priority interrupt the readings are entered into a CIC filter.

When the SYSTICK routine ends a cycle of 32, which is the end of a 1/64th sec duration, a flag counter
increments so that cic filter readings are then loaded into a CAN message the message setup for sending.
 
*/
#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/gpio.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/adc.h"
#include "libopenstm32/dma.h"
#include "libusartstm32/commonbitband.h"
#include "common.h"
#include "common_can.h"
#include "pinconfig_all.h"
#include "adcsensor_tilt.h"
#include "canwinch_pod_common_systick2048.h"
#include "rpmsensor.h"
#include "CANascii.h"

/* ADC usage sensor board
PA0 ADC123-IN0	Throttle potentiometer
PA1 ADC123-IN1  Thermistor
PA3 ADC123-IN2	Pressure sensor  

/* ADC usage on POD board--
This routine is based on only reading the three axis
accelerometer.

PA 3 ADC123-IN3	Thermistor on 32 KHz xtal
PB 0 ADC12 -IN8	Bottom cell of battery
PB 1 ADC12 -IN9	Top cell of battery 
PC 0 ADC12 -IN10	Accelerometer X	
PC 1 ADC12 -IN11	Accelerometer Y
PC 2 ADC12 -IN12	Accelerometer Z	
PC 3 ADC12 -IN13	Op-amp
-- - ADC1  -IN16	Internal temp ref
-- - ADC1  -IN17	Internal voltage ref (Vrefint)


*/
/*
Bits 29:0 SMPx[2:0]: Channel x Sample time selection for less than 1/4 LSB of 12 bit conversion.
           These bits are written by software to select the sample time individually for each channel.
           During sample cycles channel selection bits must remain unchanged.
           000: 1.5 cycles	@14MHz  0.4K max input R (p 123 datasheet)
           001: 7.5 cycles	@14MHz  5.9K max input R 
           010: 13.5 cycles	@14MHz 11.4K max input R
           011: 28.5 cycles	@14MHz 25.2K max input R
           100: 41.5 cycles	@14MHz 37.2K max input R
           101: 55.5 cycles	@14MHz 50.0K max input R
           110: 71.5 cycles	50K max
           111: 239.5 cycles	50K max

With system clock of 64 MHz and p2 clock of 32 MHz, the ADC clock
will be 32/4 = 8 MHz (max freq must be less than 14 MHz)
With an ADC clock of 4 MHz the total conversion time--
12.5 + SMPx time
At 0.125 us per adc clock cycle 
(1) Accelerometer X: 	12.5 + 55.5 = 68; * 0.125 us =  8.50 us per conversion
(2) Accelerometer Y: 	12.5 + 55.5 = 68; * 0.125 us =  8.50 us per conversion
(3) Accelerometer Z:	12.5 + 55.5 = 68; * 0.125 us =  8.50 us per conversion
Total for sequence of three .......................  = 25.50 us
For 16 sequences per DMA interrupt................. = 408.00 us
The logging/message sample rate (1/2048)........... = 488.28125 us

*/
// *CODE* for number of cycles in conversion on each adc channel
#define SMP1	5
#define SMP2	5
#define SMP3	5
#define SMP4	3
#define SMP5	3
#define SMP6	3
#define SMP7	3
#define SMP8	6
#define SMP9	6
#define SMP10	5
#define SMP11	5 ~32K accelerometer RC
#define SMP12	5 ~32K accelerometer RC
#define SMP13	3 ~32K accelerometer RC
#define SMP14	3
#define SMP15	3
#define SMP16	7
#define SMP17	7

/* ********** Static routines **************************************************/
static void adc_init_se_tilt(void);
static void adc_start_conversion_se_tilt(void);
static void adc_start_cal_register_reset_se_tilt(void);
static void adc_start_calibration_se_tilt(void);
static void adcsensor_tilt_bufadd(void);


/* CIC routines buried somewhere below */
static void adc_cic_filtering_tilt(void);
static void adc_cic_init_tilt(u32 iamunitnumber);

/* Pointers to functions to be executed under a low priority interrupt */
// These hold the address of the function that will be called
void 	(*systickHIpriority3_ptr)(void) = 0;	// SYSTICK handler (very high priority) continuation
void 	(*systickLOpriority3_ptr)(void) = 0;	// SYSTICK handler (low high priority) continuation--1/2048th
void 	(*systickLOpriority3X_ptr)(void) = 0;	// SYSTICK handler (low high priority) continuation--1/64th


/* APB2 bus frequency is needed to be able to set ADC prescalar so that it is less than 14 MHz, 
   but as high as possible. */
extern unsigned int	pclk2_freq;	// APB2 bus frequency in Hz (see 'lib/libmiscstm32/clockspecifysetup.c')

/* SYSCLK frequency is used to convert millisecond delays into SYSTICK counts */
extern unsigned int	sysclk_freq;	// SYSCLK freq in Hz (see 'lib/libmiscstm32/clockspecifysetup.c')

struct ADCDR_TILT  strADC1dr;	// Double buffer array of ints for ADC readings, plus count and index
struct ADCDR_TILT *strADC1resultptr;	// Pointer to struct holding adc data stored by DMA

/* ----------------------------------------------------------------------------- 
 * void timedelay (u32 ticks);
 * ticks = processor ticks to wait, using DTW_CYCCNT (32b) tick counter
 ------------------------------------------------------------------------------- */
static void timedelay (unsigned int ticks)
{
	/* Start time count */
	u32 t0 = *(volatile unsigned int *)0xE0001004; // DWT_CYCNT
	t0 += ticks;
	while (( (int)t0 ) - (int)(*(volatile unsigned int *)0xE0001004) > 0 );
	return;
}
/******************************************************************************
 * void adc_init_se_tilt_sequence(u32 iamunitnumber);
 * @brief 	: Call this routine to do a timed sequencing of power up and calibration
 * @param	: iamunitnumber = CAN unit id for this unit, see 'common_can.h'
*******************************************************************************/
void adc_init_se_tilt_sequence(u32 iamunitnumber)
{
	/* Setup for lower level filtering of ADC readings buffered by DMA. */
	adc_cic_init_tilt(iamunitnumber);

	u32 ticksperus = (sysclk_freq/1000000);	// Number of ticks in one microsecond

	/* Initialize ADC1 */
	adc_init_se_tilt();			// Initialize ADC1 registers.
	timedelay(50 * ticksperus);	// Wait

	/* Start the reset process for the calibration registers */
	adc_start_cal_register_reset_se_tilt();
	timedelay(10 * ticksperus);	// Wait

	/* Start the register calibration process */
	adc_start_calibration_se_tilt();
	timedelay(80 * ticksperus);	// Wait

	/* ADC is now ready for use.  Start ADC conversions */
	adc_start_conversion_se_tilt();	// Start ADC conversions and wait for ADC watchdog interrupts

	return;
}
/******************************************************************************
 * static void adc_init_se_tilt(void);
 * @brief 	: Initialize adc for dma channel 1 transfer
*******************************************************************************/
const struct PINCONFIGALL pin_accel_x = {(volatile u32 *)GPIOC, 0, IN_ANALOG, 0};
const struct PINCONFIGALL pin_accel_y = {(volatile u32 *)GPIOC, 1, IN_ANALOG, 0};
const struct PINCONFIGALL pin_accel_z = {(volatile u32 *)GPIOC, 2, IN_ANALOG, 0};


static void adc_init_se_tilt(void)
{
	u32 ticksperus = (sysclk_freq/1000000);	// Number of ticks in one microsecond

	/*  Setup ADC pins for ANALOG INPUT (p 148) */
	pinconfig_all( (struct PINCONFIGALL *)&pin_accel_x);
	pinconfig_all( (struct PINCONFIGALL *)&pin_accel_y);
	pinconfig_all( (struct PINCONFIGALL *)&pin_accel_z);

	/* Find prescalar divider code for the highest permitted ADC freq (which is 14 MHz) */
	unsigned char ucPrescalar = 0;			// Division by 2	
	if (pclk2_freq/8 < 14000000) ucPrescalar = 3;	// Division by 8
	if (pclk2_freq/6 < 14000000) ucPrescalar = 2;	// Division by 6
	if (pclk2_freq/4 < 14000000) ucPrescalar = 1;	// Division by 4

	/* ==>Override the above<==  32 MHz pclk2, divide by 4 -> 8 MHz.  This is a good compromise for
           the thermistor and throttle pot offsets due to source impedance. */
	ucPrescalar = 1;

	/* Set APB2 bus divider for ADC clock */
	RCC_CFGR |= ( (ucPrescalar & 0x3) << 14)	; // Set code for bus division (p 98)

	/* Enable bus clocking for ADC */
	RCC_APB2ENR |= RCC_APB2ENR_ADC1EN;	// Enable ADC1 clocking (see p 104)
	
	/* Scan mode (p 236) 
"This bit is set and cleared by software to enable/disable Scan mode. In Scan mode, the
inputs selected through the ADC_SQRx or ADC_JSQRx registers are converted." 
	AWDEN - enable watchdog register p 229,30. */
	//         (   scan      | watchdog enable | watchdog interrupt enable | watchdog channel number )
	ADC1_CR1 = (ADC_CR1_SCAN );

	/*               use DMA   |  Continuous   | Power ON 	*/
	ADC1_CR2  = (  ADC_CR2_DMA | ADC_CR2_CONT  | ADC_CR2_ADON	); 	// (p 240)
	/* 1 us Tstab time is required before writing a second 1 to ADON to start conversions 
	(see p 98 of datasheet) */
	timedelay(2 * ticksperus);	// Wait

	/* Set sample times for channels used on POD board (p 236) */	
	ADC1_SMPR2 = ( (SMP12 << 6) | (SMP11 << 3) | (SMP10 << 0) );

	/* Setup the number of channels scanned (p 238) */
	ADC1_SQR1 =  ( (NUMBERADCCHANNELS_SE-1) << 20) ;	// Chan count, (p 217, 238)

	/* This maps the ADC channel number to the position in the conversion sequence */
	// Load channels IN10, IN11, IN12, for conversions sequences (p 240)
	ADC1_SQR3 = (10 << 0) | (11 << 5) | (12 << 10); 

	/* Setup DMA for storing data in the ADC_DR as the channels in the sequence are converted (p 199) */
	RCC_AHBENR |= RCC_AHBENR_DMA1EN;			// Enable DMA1 clock (p 102)
	DMA1_CNDTR1 = (NUMBERSEQUENCES * NUMBERADCCHANNELS_SE * 2 );// Number of data items before wrap-around
	DMA1_CPAR1 = (u32)&ADC1_DR;				// DMA channel 1 peripheral address (adc1 data register) (p 211, 247)
	DMA1_CMAR1 = (u32)&strADC1dr.in[0][0][0];		// Memory address of first buffer array for storing data (p 211)

	// Channel configurion reg (p 209)
	//          priority high  | 32b mem xfrs | 16b adc xfrs | mem increment | circular mode | half xfr     | xfr complete   | dma chan 1 enable
	DMA1_CCR1 =  ( 0x02 << 12) | (0x02 << 10) |  (0x01 << 8) | DMA_CCR1_MINC | DMA_CCR1_CIRC |DMA_CCR1_HTIE | DMA_CCR1_TCIE  | DMA_CCR1_EN;

/* NOTE: This implementation does not need deal with the DMA interrupt */
//	DMA1_CCR1 =  ( 0x02 << 12) | (0x02 << 10) |  (0x01 << 8) | DMA_CCR1_MINC | DMA_CCR1_CIRC | DMA_CCR1_EN;


	/* Set and enable interrupt controller for DMA transfer complete interrupt handling */
	NVICIPR (NVIC_DMA1_CHANNEL1_IRQ, DMA1_CH1_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_DMA1_CHANNEL1_IRQ);
	return;
}
/******************************************************************************
 * static void adc_start_cal_register_reset_se_tilt(void);
 * @brief 	: Start calibration register reset
 * @return	: Current 32 bit SYSTICK count
*******************************************************************************/
static void adc_start_cal_register_reset_se_tilt(void)
{
	/* Reset calibration register  */
	ADC1_CR2 |= ADC_CR2_RSTCAL;			// Turn on RSTCAL bit to start calibration register reset
	
	/* Wait for register to reset */
	while ((ADC1_CR2 & ADC_CR2_RSTCAL) != 0);

	return;
}
/******************************************************************************
 * static void adc_start_calibration_se_tilt(void);
 * @brief 	: Start calibration
*******************************************************************************/
static void adc_start_calibration_se_tilt(void)
{
	/* Start calibration  */
	ADC1_CR2 |= ADC_CR2_CAL;				// Turn on RSTCAL bit to start calibration register reset

	/* Wait for calibration to complete (about 7 us */
	while ((ADC1_CR2 & ADC_CR2_CAL) != 0);

	return;
}
/******************************************************************************
 * static unsigned int adc_start_conversion_se_tilt(void);
 * @brief 	: Start a conversion of the regular sequence
*******************************************************************************/
static void adc_start_conversion_se_tilt(void)
{	
	/* Save pointer to struct where data will be stored.  ISR routine will use it */
	strADC1resultptr = &strADC1dr;		// Pointer to struct with data array and busy flag
	strADC1resultptr->flg = 1;		// We start filling buffer 0, so show previous buffer

/* After the initial write of this bit, subsequent writes start the ADC conversion--(p 241)
"Conversion starts when this bit holds a value of 1 and a 1 is written to it. The application."
should allow a delay of tSTAB between power up and start of conversion. Refer to Figure 27 */
	ADC1_CR2 |= ADC_CR2_ADON;		// Writing a 1 starts the conversion (see p 238)

	return;
}
/*#######################################################################################
 * ISR routine for DMA1 Channel1 reading ADC regular sequence of adc channels
 *####################################################################################### */
void DMA1CH1_IRQHandler_tilt(void)
{
/* Double buffer the sequence of channels converted.  When the DMA goes from the first
to the second buffer a half complete interrupt is generated.  When it completes the
storing of two buffers full a transfer complete interrut is issued, and the DMA address
pointer is automatically reloaded with the beginning of the buffer space (i.e. circular). 

'flg' is the high order index into the two dimensional array. 

'cnt' is a running counter of sequences converted.  Maybe not too useful except for debugging */

	if ( (DMA1_ISR & DMA_ISR_TCIF1) != 0 )	// Is this a transfer complete interrupt?
	{ // Here, yes.  The second sequence has been converted and stored in the second buffer
		strADC1resultptr->flg  = 1;	// Set the index to the second buffer.  It is ready.
		strADC1resultptr->cnt += 1;	// Running count of sequences completed
		DMA1_IFCR = DMA_IFCR_CTCIF1;	// Clear transfer complete flag (p 208)
	}
	if ( (DMA1_ISR & DMA_ISR_HTIF1) != 0 )	// Is this a half transfer complete interrupt?
	{ // Here, yes.  The first sequence has been converted and stored in the first buffer
		strADC1resultptr->flg  = 0;	// Set index to the first buffer.  It is ready.
		strADC1resultptr->cnt += 1;	// Running count of sequences completed
		DMA1_IFCR = DMA_IFCR_CHTIF1;	// Clear transfer complete flag (p 208)
	}

	/* Trigger a pending interrupt that will handle filter the ADC readings. */
	NVICISPR(NVIC_FSMC_IRQ);	// Set pending (low priority) interrupt for  ('../lib/libusartstm32/nvicdirect.h')

	return;
}
/*#######################################################################################
 * ISR routine for handling lower priority procesing
 *####################################################################################### */
/* Pointer to functions to be executed under a low priority interrupt, forced by DMA interrupt. */
void 	(*dma_ll_ptr)(void) = 0;		// DMA -> FSMC  (low priority)

void FSMC_IRQHandler(void)
{
	/* Call other routines if an address is set up */
	if (dma_ll_ptr != 0)	// Having no address for the following is bad.
		(*dma_ll_ptr)();	// Go do something
	return;
}
/* $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
         Below is CIC Filtering of DMA stored ADC readings 
 $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ */
/* **** CIC filtering stuff **** */
#include "cic_filter_l_N2_M3.h"

long	adc_last_filtered[NUMBERADCCHANNELS_SE];	// Last computed & filtered value for each channel
long	adc_temperature;		// Thermistor filter/decimate to 2/sec
int	adc_temp_flag;			// Signal main new filtered reading ready
int	adc_calib_temp;			// Temperature in deg C 

static struct CICLN2M3 adc_cic[NUMBERADCCHANNELS_SE];	// CIC intermediate storage adc readings
static struct CICLN2M3 adc_cic_temp;			// CIC intermediate storage for further temp filter/decimiation

/* Sensor CAN messages */
static struct CANRCVBUF can_msg_TT;	// Throttle & Thermistor
static struct CANRCVBUF can_msg_PR;	// Pressure & RPM
static struct CANRCVBUF can_msg_TR;	// Throttle
static struct CANRCVBUF can_msg_T;	// Temperature (derived from thermistor)


/******************************************************************************
 * static void adc_cic_init_tilt(u32 iamunitnumber);
 * @brief 	: Setup for cic filtering of adc readings buffered by dma
 * @param	: iamunitnumber = CAN unit id for this unit, see 'common_can.h'
*******************************************************************************/
/*
struct CICLN2M3
{
	unsigned short	usDecimateNum;	// Downsampling number
	unsigned short	usDiscard;	// Initial discard count
	int		nIn;		// New reading to be filtered
	long 		lIntegral[3];	// Three stages of Z^(-1) integrators
	long		lDiff[3][2];	// Three stages of Z^(-2) delay storage
	long		lout;		// Filtered/decimated data output
	unsigned short	usDecimateCt;	// Downsampling counter
	unsigned short	usFlag;		// Filtered/decimated data ready counter
};
*/

static void adc_cic_init_tilt(u32 iamunitnumber)
{
	int i,j;

	/* CAN message id's and data types. */
	can_msg_PR.id = iamunitnumber | (CAN_DATAID_SE1_PRESSUREandRPM        << CAN_DATAID_SHIFT); // Two ints
	can_msg_TT.id = iamunitnumber | (CAN_DATAID_SE1_THROTTLEandTHERMISTOR << CAN_DATAID_SHIFT); // Two ints
	can_msg_T.id  = iamunitnumber | (CAN_DATAID_SE1_TEMPERATURE           << CAN_DATAID_SHIFT); // One short
	can_msg_TR.id = iamunitnumber | (CAN_DATAID_SE1_THROTTLE	      << CAN_DATAID_SHIFT); // One short
	
	/* Initialize the structs that hold the CIC filtering intermediate values. */
	strADC1dr.cnt = 0;
	strADC1dr.flg = 0;
	for (i = 0; i < NUMBERADCCHANNELS_SE; i++)	
	{
		adc_cic[i].usDecimateNum = DECIMATION_SE; // Decimation number
		adc_cic[i].usDecimateCt = 0;		// Decimation counter
		adc_cic[i].usDiscard = DISCARD_SE;	// Initial discard count
		adc_cic[i].usFlag = 0;			// 1/2 buffer flag
		for (j = 0; j < 3; j++)
		{ // Very important that the integrators begin with zero.
			adc_cic[i].lIntegral[j] = 0;
			adc_cic[i].lDiff[j][0] = 0;
			adc_cic[i].lDiff[j][1] = 0;
		}	
	}
	/* adc_cic_temp filters/decimates the 64/sec thermistor adc reading to 2/sec */
	adc_cic_temp.usDecimateNum = DECIMATION_SE; // Decimation number
	adc_cic_temp.usDecimateCt = 0;		// Decimation counter
	adc_cic_temp.usDiscard = DISCARD_SE;	// Initial discard count
	adc_cic_temp.usFlag = 0;			// 1/2 buffer flag
	for (j = 0; j < 3; j++)
	{ // Very important that the integrators begin with zero.
		adc_cic_temp.lIntegral[j] = 0;
		adc_cic_temp.lDiff[j][0] = 0;
		adc_cic_temp.lDiff[j][1] = 0;
	}	

	/* This is a low priority post SYSTICK interrupt call. */
	systickLOpriority2_ptr = &adc_cic_filtering_tilt;

	/* This is a very high priority post SYSTICK interrupt call. */
	systickHIpriority_ptr = &adcsensor_tilt_bufadd;

	return;
}
/* ######################### UNDER HIGH PRIORITY SYSTICK INTERRUPT ############################### 
 * Enter from SYSTICK interrupt each 2048 per sec, and buffer latest ADC sequence of readings
 * ############################################################################################### */
/* This stores the latest sequence of adc readings, for low priority filtering/sync'ing through a
   cic filter. 

   This routine is entered from the SYSTICK interrupt handler at high priority.
*/
static int adc2048[NUMBERADCCHANNELS_SE];	// Holds the latest sequence of ADC readings

static void adcsensor_tilt_bufadd(void)
{
	int *p1 = &adc2048[0];
	int *p2 = &strADC1dr.in[strADC1dr.flg][0][0];
	
	while (p1 < &adc2048[NUMBERADCCHANNELS_SE]) *p1++ = *p2++;

	/* Call other routines if an address is set up */
	if (systickHIpriority3_ptr != 0)	// Having no address for the following is bad.
		(*systickHIpriority3_ptr)();	// Go do something	
	return;
}
/* ########################## UNDER LOW PRIORITY SYSTICK INTERRUPT ############################### 
 * Run the latest adc readings through the cic filter.
 * ############################################################################################### */
/* 
   This routine is entered from the SYSTICK interrupt handler triggering I2C2_EV low priority interrupt. 
*/
unsigned int cicdebug0,cicdebug1;

static u32 stk_64flgctr_prev;
u32 cic_sync_err[NUMBERADCCHANNELS_SE];	// CIC sync errors

static void adc_cic_filtering_tilt(void)
{
	int i;
	int flag = 0;


	/* Add the latest buffered sequence of readings to the cic filter */
	for (i = 0; i < NUMBERADCCHANNELS_SE; i++)	
	{
		adc_cic[i].nIn = adc2048[i]; 	// Load reading to filter struct
		if (cic_filter_l_N2_M3 (&adc_cic[i]) != 0) // Filtering complete?
		{ // Here, yes.
			adc_last_filtered[i] = adc_cic[i].lout >> CICSCALE; // Scale and save
			flag = 1;
cicdebug0 += 1;
		}
	}
	/* Run the thermistor reading through another filter/decimation */
	if (flag != 0)
	{
		adc_cic_temp.nIn = adc_last_filtered[1];
		if (cic_filter_l_N2_M3 (&adc_cic_temp) != 0) // Filtering complete?
		{ // Here, yes.
			adc_temperature = adc_cic_temp.lout; // Save for mainline
			adc_temp_flag = 1;	// Signal main that reading is ready for computation
		}
	}


	/* Was this one the 1/64th sec demarcation? 'canwinch_pod_common_systick2048.c' increments
           'stk_64flgctr' at the end of each 1/64th sec interval. */
	if (stk_64flgctr != stk_64flgctr_prev)
	{ // Here, yes.  Time to setup for sending
	/* ========= Here is where adc readings get setup for sending ============ */
		stk_64flgctr_prev = stk_64flgctr;

		/* Check that CIC filtering is sync'ed to the 1/64th tick */
		for (i = 0; i < NUMBERADCCHANNELS_SE; i++)	
		{
			if (adc_cic[i].usDecimateCt != 0)
			{
				cic_sync_err[i] += 1;	// Count errors
				adc_cic[i].usDecimateCt = 0; // Re-sync
			}
		}
		/* Setup Pressure and RPM readings for sending */
		canmsg_send(&can_msg_PR, adc_last_filtered[2], rpm); // Send as two signed ints

		/* Setup Throttle and Thermistor readings for sending */
//		canmsg_send(&can_msg_TT, adc_last_filtered[0],adc_last_filtered[1]);

		/* Setup Throttle only */
		canmsg_send_sys_n(&can_msg_TR, (u8*)&adc_last_filtered[0], 2); // Send as a unsigned short


		/* Send calibrated temperature (see 'temp_calc.c') */
		if (adc_temp_flag == 2) // Did mainline (temp_calc.c) place a new temperature reading?
		{ // Here, yes.
			adc_temp_flag = 0;	// Reset flag for next round
			canmsg_send_sys_n(&can_msg_T, (u8*)&adc_calib_temp, 2); // Send as a signed short
		}
		
		/* Send printf if send switch is on. */
		while ( CANascii_send() != 0)

		/* Call other routines if an address is set up--64 per sec. */
		if (systickLOpriority3X_ptr != 0)	// Having no address for the following is bad.
			(*systickLOpriority3X_ptr)();	// Go do something
	}

	return;
}


