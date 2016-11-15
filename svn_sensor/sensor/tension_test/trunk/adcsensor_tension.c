/******************************************************************************
* File Name          : adcsensor_tension.c
* Date First Issued  : 02/14/2015
* Board              : POD
* Description        : ADC routines for f103 pod board with two AD7799
*******************************************************************************/
/* 


04/20/2014
CAN messages:


11/11/2012 This is a hack of svn_pod/sw_stm32/trunk/devices/adcpod.c
See p214 of Ref Manual for ADC section 
02/12/2015 This is a hack of adcsensor_tension.c

*/
/*
NOTE: Some page number refer to the Ref Manual RM0008, rev 11 and some more recent ones, rev 14.

Strategy--
Four ADC pins are read at a rapid rate with the DMA storing the sequence.  The DMA wraps around
at the end of the buffer.  The code for DMA interrupts is not used.  
 
*/
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/rcc.h"
#include "libopenstm32/adc.h"
#include "libopenstm32/dma.h"
#include "common.h"
#include "common_can.h"
#include "pinconfig_all.h"
#include "adcsensor_tension.h"
#include "DTW_counter.h"
#include "cic_filter_l_N2_M3.h"

/* ADC usage on POD board--
This routine is for measuring two load-cells.  For temp compensation
testing the temp is measured at the load-cell and AD7799 for each of the two.

PA 3 ADC123-IN3	Thermistor on 32 KHz xtal..Thermistor: AD7799 #2
PB 0 ADC12 -IN8	Bottom cell of battery.....Not scanned
PB 1 ADC12 -IN9	Top cell of battery........Not scanned
PC 0 ADC12 -IN10	Accelerometer X....Thermistor: load-cell #1
PC 1 ADC12 -IN11	Accelerometer Y....Thermistor: load-cell #2
PC 2 ADC12 -IN12	Accelerometer Z....Thermistor: AD7799 #1
PC 3 ADC12 -IN13	Op-amp.............Not scanned
-- - ADC1  -IN16	                   Internal temp ref
-- - ADC1  -IN17	                   Internal voltage ref (Vrefint)

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
(1) Thermistor AD7799 #2   : 	12.5 + 55.5 = 68; * 0.125 us =  8.50 us per conversion
(2) Thermistor load-cell #1: 	12.5 + 55.5 = 68; * 0.125 us =  8.50 us per conversion
(3) Thermistor load-cell #2: 	12.5 + 55.5 = 68; * 0.125 us =  8.50 us per conversion
(4) Thermistor AD7799 #1   : 	12.5 + 55.5 = 68; * 0.125 us =  8.50 us per conversion
Total for sequence of three .......................  = 25.50 us
For 16 sequences per DMA interrupt................. = 408.00 us
The logging/message sample rate (1/2048)........... = 488.28125 us

*/
// *CODE* for number of cycles in conversion on each adc channel
#define SMP1	7
#define SMP2	1
#define SMP3	7 // Thermistor AD7799 #2
#define SMP4	1
#define SMP5	1
#define SMP6	1
#define SMP7	1
#define SMP8	1
#define SMP9	1
#define SMP10	7 // Thermistor load-cell #1
#define SMP11	7 // Thermistor load-cell #2
#define SMP12	7 // Thermistor AD7799 #1
#define SMP13	1
#define SMP14	1
#define SMP15	1
#define SMP16	7 // Internal temp ref
#define SMP17	7 // Internal voltage ref (Vrefint)


/* ********** Static routines **************************************************/
static void adcsensor_tension_init(void);
static void adcsensor_tension_start_conversion(void);
static void adcsensor_tension_start_cal_register_reset(void);
static void adcsensor_tension_start_calibration(void);

/* CIC routines buried somewhere below */
static void adc_cic_filtering_tension(void);
static void adc_cic_init_tension(void);

/* Pointers to functions to be executed under a low priority interrupt */
// These hold the address of the function that will be called
void 	(*systickHIpriority3_ptr)(void)  = 0;	// SYSTICK handler (very high priority) continuation
void 	(*systickLOpriority3_ptr)(void)  = 0;	// SYSTICK handler (low high priority) continuation--1/2048th
void 	(*systickLOpriority3X_ptr)(void) = 0;	// SYSTICK handler (low high priority) continuation--1/64th

/* APB2 bus frequency is needed to be able to set ADC prescalar so that it is less than 14 MHz, 
   but as high as possible. */
extern unsigned int	pclk2_freq;	// APB2 bus frequency in Hz (see 'lib/libmiscstm32/clockspecifysetup.c')

/* SYSCLK frequency is used to time delays. */
extern unsigned int	sysclk_freq;	// SYSCLK freq in Hz (see 'lib/libmiscstm32/clockspecifysetup.c')

static struct ADCDR_TENSION  strADC1dr;	// Double buffer array of ints for ADC readings, plus count and index
static struct ADCDR_TENSION *strADC1resultptr;	// Pointer to struct holding adc data stored by DMA

/* ----------------------------------------------------------------------------- 
 * void timedelay (u32 ticks);
 * ticks = processor ticks to wait, using DTW_CYCCNT (32b) tick counter
 ------------------------------------------------------------------------------- */
static void timedelay (unsigned int ticks)
{
	u32 t1 = ticks + DTWTIME; // DWT_CYCNT
	WAITDTW(t1);
	return;
}
/******************************************************************************
 * void adcsensor_tension_sequence(void);
 * @brief 	: Call this routine to do a timed sequencing of power up and calibration
*******************************************************************************/
void adcsensor_tension_sequence(void)
{
	u32 ticksperus = (sysclk_freq/1000000);	// Number of ticks in one microsecond

	/* Setup for lower level filtering of ADC readings buffered by DMA. */
	adc_cic_init_tension();

	/* Wait for previously switched-on voltages to stabilize. */
	u32 t1 = DTWTIME + (500 * (sysclk_freq/1000));	// 500 ms delay
	WAITDTW(t1);

	/* Initialize ADC1 */
	adcsensor_tension_init();	// Initialize ADC1 registers.
	timedelay(50 * ticksperus);	// Wait

	/* Start the reset process for the calibration registers */
	adcsensor_tension_start_cal_register_reset();
	timedelay(10 * ticksperus);	// Wait

	/* Start the register calibration process */
	adcsensor_tension_start_calibration();
	timedelay(80 * ticksperus);	// Wait

	/* ADC is now ready for use.  Start ADC conversions */
	adcsensor_tension_start_conversion();	// Start ADC conversions and wait for ADC watchdog interrupts

	return;
}
/******************************************************************************
 * static void adcsensor_tension_init(void);
 * @brief 	: Initialize adc for dma channel 1 transfer
*******************************************************************************/
const struct PINCONFIGALL pin_accel_x = {(volatile u32 *)GPIOC, 0, IN_ANALOG, 0};
const struct PINCONFIGALL pin_accel_y = {(volatile u32 *)GPIOC, 1, IN_ANALOG, 0};
const struct PINCONFIGALL pin_accel_z = {(volatile u32 *)GPIOC, 2, IN_ANALOG, 0};

static void adcsensor_tension_init(void)
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

	/* ==>Override the above<==  32 MHz pclk2, divide by 4 -> 8 MHz. */
	ucPrescalar = 3;

	/* Set APB2 bus divider for ADC clock */
	RCC_CFGR |= ( (ucPrescalar & 0x3) << 14); // Set code for bus division (p 98)

	/* Enable bus clocking for ADC */
	RCC_APB2ENR |= RCC_APB2ENR_ADC1EN;	// Enable ADC1 clocking (see p 104)
	
	//         (   scan      | watchdog enable | watchdog interrupt enable | watchdog channel number )
	ADC1_CR1 = (ADC_CR1_SCAN ); 	// Scan mode

	/*               use DMA   |  Continuous   | Power ON 	*/
	ADC1_CR2  = (  ADC_CR2_DMA | ADC_CR2_CONT  | ADC_CR2_ADON ); 
	/* 1 us Tstab time is required before writing a second 1 to ADON to start conversions */
	timedelay(2 * ticksperus);	// Wait

	/* Set sample times for channels used on POD board */
	ADC1_SMPR2 = ( (SMP3  << 9) );	
	ADC1_SMPR1 = ( (SMP12 << 6) | (SMP11 << 3) | (SMP10 << 0) );

	/* Setup the number of channels scanned */
	ADC1_SQR1 =  ((NUMBERADCCHANNELS_TEN-1) << 20);	// Channel count

	/* This maps the ADC channel number to the position in the conversion sequence */
	// Load channels IN0, IN10, IN11, IN12, for conversions sequences
	ADC1_SQR3 = (3 << 0) | (10 << 5) | (11 << 10) | (12 << 15); 

	/* Setup DMA for storing data in the ADC_DR as the channels in the sequence are converted */
	RCC_AHBENR |= RCC_AHBENR_DMA1EN;			// Enable DMA1 clock (p 102)
	DMA1_CNDTR1 = (2 * NUMBERSEQUENCES * NUMBERADCCHANNELS_TEN);// Number of data items before wrap-around
	DMA1_CPAR1 = (u32)&ADC1_DR;				// DMA channel 1 peripheral address (adc1 data register)
	DMA1_CMAR1 = (u32)&strADC1dr.in[0][0][0];		// Memory address of first buffer array for storing data 

	// Channel configuration register
	//          priority high  | 32b mem xfrs | 16b adc xfrs | mem increment | circular mode | half xfr     | xfr complete   | dma chan 1 enable
	DMA1_CCR1 =  ( 0x02 << 12) | (0x02 << 10) |  (0x01 << 8) | DMA_CCR1_MINC | DMA_CCR1_CIRC |DMA_CCR1_HTIE | DMA_CCR1_TCIE  | DMA_CCR1_EN;

	/* Set and enable interrupt controller for DMA transfer complete interrupt handling */
	NVICIPR (NVIC_DMA1_CHANNEL1_IRQ, DMA1_CH1_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_DMA1_CHANNEL1_IRQ);

	/* Low level interrupt for doing adc filtering following DMA1 CH1 interrupt. */
	NVICIPR (NVIC_FSMC_IRQ, ADC_FSMC_PRIORITY);	// Set low level interrupt priority for post DMA1 interrupt processing
	NVICISER(NVIC_FSMC_IRQ);			// Enable low level interrupt

	return;
}
/******************************************************************************
 * static void adcsensor_tension_start_cal_register_reset(void);
 * @brief 	: Start calibration register reset
 * @return	: Current 32 bit SYSTICK count
*******************************************************************************/
static void adcsensor_tension_start_cal_register_reset(void)
{
	/* Reset calibration register  */
	ADC1_CR2 |= ADC_CR2_RSTCAL;			// Turn on RSTCAL bit to start calibration register reset
	
	/* Wait for register to reset */
	while ((ADC1_CR2 & ADC_CR2_RSTCAL) != 0);

	return;
}
/******************************************************************************
 * static void adcsensor_tension_start_calibration(void);
 * @brief 	: Start calibration
*******************************************************************************/
static void adcsensor_tension_start_calibration(void)
{
	/* Start calibration  */
	ADC1_CR2 |= ADC_CR2_CAL;			// Turn on RSTCAL bit to start calibration register reset

	/* Wait for calibration to complete (about 7 us) */
	while ((ADC1_CR2 & ADC_CR2_CAL) != 0);

	return;
}
/******************************************************************************
 * static unsigned int adcsensor_tension_start_conversion(void);
 * @brief 	: Start a conversion of the regular sequence
*******************************************************************************/
static void adcsensor_tension_start_conversion(void)
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
void DMA1CH1_IRQHandler_tension(void)
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
	else
	{
		if ( (DMA1_ISR & DMA_ISR_HTIF1) != 0 )	// Is this a half transfer complete interrupt?
		{ // Here, yes.  The first sequence has been converted and stored in the first buffer
			strADC1resultptr->flg  = 0;	// Set index to the first buffer.  It is ready.
			strADC1resultptr->cnt += 1;	// Running count of sequences completed
			DMA1_IFCR = DMA_IFCR_CHTIF1;	// Clear transfer complete flag (p 208)
		}
	}

	/* Trigger a pending interrupt that will handle filter the ADC readings. */
	NVICISPR(NVIC_FSMC_IRQ);	// Set pending (low priority) interrupt

	return;
}
/*#######################################################################################
 * ISR routine for handling lower priority procesing
 *####################################################################################### */
/* Pointer to functions to be executed under a low priority interrupt, forced by DMA interrupt. */
void 	(*dma_ll_ptr)(void) = 0;		// DMA -> FSMC  (low priority)

void FSMC_IRQHandler_ten(void)
{
	/* Call other routines if an address is set up */
	if (dma_ll_ptr != NULL)	// Skip subroutine call if pointer not intialized
		(*dma_ll_ptr)();	// Go do something
	return;
}
/* $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
         Below is CIC Filtering of DMA stored ADC readings 
 $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ */
/* **** CIC filtering stuff **** */
#include "cic_filter_l_N2_M3.h"

/* This flag increments and the low order bit is the index of the double buffered array 'adc_readings_cic'.  
   Filtering is being done in index =  (0x1 & adc_temp_flag)
   Users use the opposite index  */
int32_t	adc_temp_flag[NUMBERADCCHANNELS_TEN];	// Signal main new filtered reading ready

/* Last computed & doubly filtered value for each channel */
uint32_t	adc_readings_cic[2][NUMBERADCCHANNELS_TEN];	// NOT SCALED

static struct CICLN2M3 adc_cic_1st[NUMBERADCCHANNELS_TEN];	// CIC intermediate storage adc readings
static struct CICLN2M3 adc_cic_2nd[NUMBERADCCHANNELS_TEN];// CIC intermediate storage for second filter/decimiation
static struct CICLN2M3 adc_cic_3rd[NUMBERADCCHANNELS_TEN];// CIC intermediate storage for second filter/decimiation

/******************************************************************************
 * static void adc_cic_init_tension(void);
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

static void adc_cic_init_tension(void)
{
	int i,j;

	/* Initialize the structs that hold the CIC filtering intermediate values. */
	strADC1dr.cnt = 0;
	strADC1dr.flg = 0;
	for (i = 0; i < NUMBERADCCHANNELS_TEN; i++)	
	{
		adc_cic_1st[i].usDecimateNum = DECIMATION_TEN; // Decimation number
		adc_cic_1st[i].usDecimateCt = 0;		// Decimation counter
		adc_cic_1st[i].usDiscard = DISCARD_TEN;	// Initial discard count
		adc_cic_1st[i].usFlag = 0;			// 1/2 buffer flag
		for (j = 0; j < 3; j++)
		{ // Very important that the integrators begin with zero.
			adc_cic_1st[i].lIntegral[j] = 0;
			adc_cic_1st[i].lDiff[j][0] = 0;
			adc_cic_1st[i].lDiff[j][1] = 0;
		}	
		/* adc_cic_temp filters/decimates the 64/sec thermistor adc reading to 2/sec */
		adc_cic_2nd[i].usDecimateNum = DECIMATION_TEN; // Decimation number
		adc_cic_2nd[i].usDecimateCt = 0;		// Decimation counter
		adc_cic_2nd[i].usDiscard = DISCARD_TEN;	// Initial discard count
		adc_cic_2nd[i].usFlag = 0;			// 1/2 buffer flag
		for (j = 0; j < 3; j++)
		{ // Very important that the integrators begin with zero.
			adc_cic_2nd[i].lIntegral[j] = 0;
			adc_cic_2nd[i].lDiff[j][0] = 0;
			adc_cic_2nd[i].lDiff[j][1] = 0;
		}
		/* adc_cic_temp filters/decimates the 64/sec thermistor adc reading to 2/sec */
		adc_cic_3rd[i].usDecimateNum = DECIMATION_TEN; // Decimation number
		adc_cic_3rd[i].usDecimateCt = 0;		// Decimation counter
		adc_cic_3rd[i].usDiscard = DISCARD_TEN;	// Initial discard count
		adc_cic_3rd[i].usFlag = 0;			// 1/2 buffer flag
		for (j = 0; j < 3; j++)
		{ // Very important that the integrators begin with zero.
			adc_cic_3rd[i].lIntegral[j] = 0;
			adc_cic_3rd[i].lDiff[j][0] = 0;
			adc_cic_3rd[i].lDiff[j][1] = 0;
		}
	}	

	/* Following DMA interrupt do filtering. */
	dma_ll_ptr = &adc_cic_filtering_tension;

	return;
}
/* ########################## UNDER LOW PRIORITY SYSTICK INTERRUPT ############################### 
 * Run the latest adc readings through the cic filter.
 * ############################################################################################### */
/* 
   This routine is entered from  'FSMC_IRQHandler' via 'dma_ll_ptr' triggered by 'DMA1CH1_IRQHandler_tension'
*/
unsigned int cicdebug0,cicdebug1;

static void adc_cic_filtering_tension(void)
{
	int i,j,k;	// FORTRAN variables, of course
	uint32_t* p;		// Ptr to raw double buffered DMA'd readings

cicdebug0 += 1;
	for (i = 0; i < NUMBERSEQUENCES; i++)
	{ // First level of CIC filtering

		p = &strADC1dr.in[strADC1resultptr->flg][i][0];
		for (j = 0; j < NUMBERADCCHANNELS_TEN; j++)	
		{
			adc_cic_1st[j].nIn = *p; 	// Load reading to filter struct
			if (cic_filter_l_N2_M3 (&adc_cic_1st[j]) != 0) // Filtering complete?
			{ // Here, yes.  Pass first level filter results through a second time.
				adc_cic_2nd[j].nIn = adc_cic_1st[j].lout >> CICSCALE; // Scale
				if (cic_filter_l_N2_M3 (&adc_cic_2nd[j]) != 0) // Filtering complete?
				{ // Here, two CIC filtering passes complete.
					adc_cic_3rd[j].nIn = adc_cic_2nd[j].lout >> CICSCALE; // Scale
					if (cic_filter_l_N2_M3 (&adc_cic_3rd[j]) != 0) // Filtering complete?
					{ // Here, three CIC filtering passes complete.
						k = (0x1 & adc_temp_flag[j]);	// Get current double buffer index
						adc_readings_cic[k][j] = adc_cic_2nd[j].lout; // Save for mainline NOT SCALED
						adc_temp_flag[j] +=1;	// Signal main readings are ready for computation
cicdebug1 += 1;
					}
				}
			}
			p++;	// Use pointer to avoid multiplies with triple index array
		}
	}
	return;
}


