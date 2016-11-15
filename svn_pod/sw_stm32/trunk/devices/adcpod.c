/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : adcpod.c
* Hackerees          : deh
* Date First Issued  : 07/06/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : ADC routines for pod
*******************************************************************************/
/* See p214 of Ref Manual for ADC section 
This routines uses ADC1.
This routine uses DMA for storing a "regular" sequence of adc channels.
ADC1 connects to DMA1 Channel 1.
*/

#include "adcpod.h"
#include "spi1ad7799.h"
#include "PODpinconfig.h"
#include "rtctimers.h"
#include "libmiscstm32/systick1.h"
#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/gpio.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/rtc.h"
#include "libopenstm32/dma.h"
#include "libopenstm32/adc.h"

/* Delay of ADC 3.2v supply to settle */
#define DELAYCOUNTADCREGULATOR	25	// Delay in tenth milliseconds

/* Delay for battery switches (allow divider resistors to charge capacitors) */
# define DELAYCOUNTBATTSWS	50	// Delay in tenth ms; 10 time constants is about 1/4 LSB

/* Delay for CR2_ADON (adc power up) (about 1 us) */
#define DELAYCOUNTCR2_ADON	2	// Delay in microseconds

/* ADC usage on POD board--
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
Bits 29:0 SMPx[2:0]: Channel x Sample time selection (1)
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
(1) For less than 1/4 LSB of 12 bits conversion
*/
#define SMP1	3	// Code for number of cycles in conversion on each adc channel
#define SMP2	3
#define SMP3	6
#define SMP4	3
#define SMP5	3
#define SMP6	3
#define SMP7	3
#define SMP8	6
#define SMP9	6
#define SMP10	3
#define SMP11	3
#define SMP12	3
#define SMP13	3
#define SMP14	3
#define SMP15	3
#define SMP16	7
#define SMP17	7

/* APB2 bus frequency is needed to set ADC prescalar so that it is less than 14 MHz,
   but as high as possible. */
extern unsigned int	pclk2_freq;	// APB2 bus frequency in Hz (see 'lib/libmiscstm32/clockspecifysetup.c')

/* SYSCLK frequency is used to convert millisecond delays into SYSTICK counts */
extern unsigned int	sysclk_freq;	// SYSCLK freq in Hz (see 'lib/libmiscstm32/clockspecifysetup.c')

volatile struct ADCDR	strADC1dr;	// Double buffer array of ints for ADC readings, plus count and index
volatile struct ADCDR	strADCsave[ADCSAVESIZE];

volatile struct ADCDR *strADC1resultptr;		// Pointer to struct holding adc data stored by DMA

unsigned int uiDebug0;
unsigned int uiDebug1;
unsigned int uiDebug2;

/******************************************************************************
 * unsigned int adc_init(volatile struct ADCDR *pstrADC1dr);
 * @brief 	: Initialize adc for dma channel 1 transfer
 * @param	: struct with int array to receive data, plus busy flag
 * @return	: SYSTICK count for end of CR2_ADON delay
*******************************************************************************/
unsigned int adc_init(volatile struct ADCDR *pstrADC1dr)
{
	unsigned int temp;

	/*  Setup ADC pins for ANALOG INPUT (p 148) */
	//  PA3 (IN3) - Thermistor 
	GPIO_CRL(GPIOA) &= ~(  (0x000f ) << (4*3));	// Input mode = 0x00; Analog = 0x00

	//  PB0, PB1 (IN8, IN9) - Bottom Battery Cell, Top Battery Cell
	GPIO_CRL(GPIOB) &= ~( ((0x000f ) << (4*0)) | 
                              ((0x000f ) << (4*1))   );	// Input mode = 0x00; Analog = 0x00

	//  PC0, PC1. PC2, PC3  (IN10, IN11, IN12, IN12) - Accelerometer X, Y, Z, Op amp output
	GPIO_CRL(GPIOC) &= ~( ((0x000f ) << (4*0)) | 
	                      ((0x000f ) << (4*1)) |
                              ((0x000f ) << (4*2)) | 
	                      ((0x000f ) << (4*3))   );	// Input mode = 0x00; Analog = 0x00

	/* Find prescalar divider code for the highest permitted ADC freq (which is 14 MHz) */
	unsigned char ucPrescalar = 3;		// Division by 8	
	if (pclk2_freq/8 < 14000000) ucPrescalar = 2;	// Division by 6
	if (pclk2_freq/6 < 14000000) ucPrescalar = 1;	// Division by 4
	if (pclk2_freq/4 < 14000000) ucPrescalar = 0;	// Division by 2

//ucPrescalar = 3;	// Slow it down for debugging and test

	/* Set APB2 bus divider for ADC clock */
	RCC_CFGR |= ( (ucPrescalar & 0x3) << 14)	; // Set code for bus division (p 92)

	/* Save pointer to struct where data will be stored.  ISR routine might use it */
	strADC1resultptr = pstrADC1dr;		// Pointer to data array and busy flag

	/* PE7	3.2v Regulator EN, Analog: gpio (JIC--it should have been already been turned on along with a wait delay) */
	ANALOGREG_on				// gpio macro (see PODpinconfig.h)

	/* Enable bus clocking for ADC */
	RCC_APB2ENR |= (RCC_APB2ENR_ADC1EN);	// Enable ADC1 clock (see p 104)
	
	/* Scan mode (p 236) 
"This bit is set and cleared by software to enable/disable Scan mode. In Scan mode, the
inputs selected through the ADC_SQRx or ADC_JSQRx registers are converted." */
	ADC1_CR1 = ADC_CR1_SCAN;	

	/* Internal temp sensor enabled |   use DMA   |  Continuous   | Power ON 	*/
	ADC1_CR2  = ( ADC_CR2_TSVREFE   | ADC_CR2_DMA | ADC_CR2_CONT  | ADC_CR2_ADON	); 	// (p 240)
	/* 1 us Tstab time is required before writing a second 1 to ADON to start conversions 
	(see p 98 of datasheet) */

	// Note: SYSTICK counter is a count-down, not count up counter.
	temp = SYSTICK_getcount32() - DELAYCOUNTCR2_ADON*(sysclk_freq/1000000);// Save tick count for end of delay

	/* Set sample times for channels used on POD board (p 241,242) */	
	ADC1_SMPR1 = ( (SMP17<<21) | (SMP16<<18) | (SMP13<<9) | (SMP12<<6) | (SMP11<<3) | (SMP10) );
	ADC1_SMPR2 = ( (SMP9 <<27) | (SMP8 <<24) | (SMP3 <<9) );

	/* Setup regular sequence (in the order shown in the notes above ) (p 244) */
	/* This maps the ADC channel number to the position in the conversion sequence */
	ADC1_SQR1 = ( ( (NUMBERADCCHANNELS-1) << 20) );	// Chan count, sequences 16 - 13 (p 244)
	// IN17,IN16,IN15, sequence 9,8,7 (p 244, 245)
	ADC1_SQR2 = ( (17<<10) | (16<< 5) | (13) );	// Sequences 12 - 7
	// IN12,IN11,IN10,IN9,IN8,IN3, sequence 6,5,4,3,2,1 (p 245, 246)
	ADC1_SQR3 = ( (12<<25) | (11<<20) | (10<<15) | (9<<10) | (8<<5) | (3) ); // Sequences 6 - 1

#define CHAN	3	// Debugging: set same adc channel for each of the nine sequences
//	ADC1_SQR2 = ( (CHAN<<10) | (CHAN<< 5) | (CHAN) );	// Sequences 12 - 7
//	ADC1_SQR3 = ( (CHAN<<25) | (CHAN<<20) | (CHAN<<15) | (CHAN<<10) | (CHAN<<5) | (CHAN) ); // Sequences 6 - 1

	/* Setup DMA for storing data in the ADC_DR as the channels in the sequence are converted (p 199) */
	RCC_AHBENR |= RCC_AHBENR_DMA1EN;			// Enable DMA1 clock (p 102)
	DMA1_CNDTR1 = (NUMBERADCCHANNELS * 2 );			// Number of data items before wrap-around
	DMA1_CPAR1 = (u32)&ADC1_DR;				// DMA channel 1 peripheral address (adc1 data register) (p 211, 247)
//DMA1_CPAR1 = 0x4001244c;				// DMA channel 1 peripheral address (adc1 data register) (p 211, 247)
	DMA1_CMAR1 = (u32)&pstrADC1dr->in[0][0];			// Memory address of first buffer array for storing data (p 211)

	// Channel configurion reg (p 209)
	//          priority high  | 32b mem xfrs | 16b adc xfrs | mem increment | circular mode | half xfr     | xfr complete   | dma chan 1 enable
	DMA1_CCR1 =  ( 0x02 << 12) | (0x02 << 10) |  (0x01 << 8) | DMA_CCR1_MINC | DMA_CCR1_CIRC |DMA_CCR1_HTIE | DMA_CCR1_TCIE  | DMA_CCR1_EN;

	/* Set and enable interrupt controller for DMA transfer complete interrupt handling */
	NVICIPR (NVIC_DMA1_CHANNEL1_IRQ, DMA1_CH1_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_DMA1_CHANNEL1_IRQ);			// Enable interrupt controller for RTC

	/* Return the SYSTICK count that ends the ADON delay */
	return	temp;						// Return SYSTICK count at point where ADC was turned on
}
/******************************************************************************
 * unsigned int adc_start_cal_register_reset(void);
 * @brief 	: Start calibration register reset
 * @return	: Current 32 bit SYSTICK count
*******************************************************************************/
unsigned int adc_start_cal_register_reset(void)
{
	/* Reset calibration register  */
	ADC1_CR2 |= ADC_CR2_RSTCAL;			// Turn on RSTCAL bit to start calibration register reset
	
	/* Wait for register to reset */
	while ((ADC1_CR2 & ADC_CR2_RSTCAL) != 0);

	/* Get the beginning time that we started the ADC setup */
	return	SYSTICK_getcount32();			// Return current SYSTICK count
}
/******************************************************************************
 * unsigned int adc_start_calibration(void);
 * @brief 	: Start calibration
 * @return	: Current 32 bit SYSTICK count
*******************************************************************************/
unsigned int adc_start_calibration(void)
{
	/* Start calibration  */
	ADC1_CR2 |= ADC_CR2_CAL;				// Turn on RSTCAL bit to start calibration register reset

	/* Wait for calibration to complete (about 7 us */
	while ((ADC1_CR2 & ADC_CR2_CAL) != 0);

	/* Get the beginning time that we started the ADC setup */
	return	SYSTICK_getcount32();			// Return current SYSTICK count
}
/******************************************************************************
 * unsigned int adc_start_conversion(volatile struct ADCDR *strADC1dr);
 * @brief 	: Start a conversion of the regular sequence
 * @param	: struct with int array to receive data, plus busy flag
 * @return	: Current 32 bit SYSTICK count
*******************************************************************************/
unsigned int adc_start_conversion(volatile struct ADCDR *strADC1dr)
{
	/* Save pointer to struct where data will be stored.  ISR routine will use it */
	strADC1resultptr = strADC1dr;		// Pointer to data array and busy flag
	strADC1resultptr->flg = 1;		// We start filling buffer 0, so show previous buffer
	
	/* After the initial write of this bit, subsequent writes start the ADC conversion--(p 241)
"Conversion starts when this bit holds a value of 1 and a 1 is written to it. The application."
should allow a delay of tSTAB between power up and start of conversion. Refer to Figure 27 */
	ADC1_CR2 |= ADC_CR2_ADON;		// Writing a 1 starts the conversion (see p 238)

	/* Return system tick in case timing is desired */
	return 	SYSTICK_getcount32();// Return current 23 bit SYSTICK reading (see lib/libmiscstm32/SYSTICK_getcount32.c)
}
/******************************************************************************
 * unsigned char adc_busy(void);
 * @brief 	: Check for busy
 * @param	: struct with int array to receive data, plus busy flag
 * @return	: Flag: 0 = not busy, 1 = busy.
*******************************************************************************/
unsigned char adc_busy(void)
{
	return strADC1resultptr->flg;
}
/******************************************************************************
 * unsigned int adc_regulator_turn_on(void);
 * @brief 	: Turn on 3.2v regulator for ADC and get time
 * @return	: 32 bit SYSTICK count of end of delay duration
*******************************************************************************/
unsigned int adc_regulator_turn_on(void)
{
	/* PE7	3.2v Regulator EN, Analog: gpio */
	ANALOGREG_on				// gpio macro (see PODpinconfig.h)

	/* Compute 32 bit SYSTICK count when delay has completed */
	return SYSTICK_getcount32() - (DELAYCOUNTADCREGULATOR*(sysclk_freq/10000));// 32 bit SYSTICK reading (see lib/libmiscstm32/SYSTICK_getcount32.c)	
}

/******************************************************************************
 * unsigned int adc_battery_sws_turn_on(void);
 * @brief 	: Turn on switches to battery cells and get time
 * @return	: 32 bit SYSTICK count of end of delay duration
*******************************************************************************/
unsigned int adc_battery_sws_turn_on(void)
{
	/* PC4 Battery top cell switch */
	TOPCELLADC_on				// gpio macro (see PODpinconfig.h)

	/* PC5 Battery bottom cell switch */
	BOTTOMCELLADC_on			// gpio macro (see PODpinconfig.h)

	/* Compute 32 bit SYSTICK count when delay has completed */
	return SYSTICK_getcount32() - (DELAYCOUNTBATTSWS*(sysclk_freq/10000));// 32 bit SYSTICK reading (see lib/libmiscstm32/SYSTICK_getcount32.c)	
}
/******************************************************************************
 * void adc_init_sequence(volatile struct ADCDR *strADC1dr);
 * @brief 	: Call this routine to do a timed sequencing of power up and calibration
*******************************************************************************/
void adc_init_sequence(volatile struct ADCDR *strADC1dr)
{
	unsigned int uiADCt1,uiADCt1b;

	uiADCt1 = adc_init((volatile struct ADCDR*)strADC1dr);	// Initialize the ADC for DMA reading of all POD adc pins
	while ( (uiADCt1b = SYSTICK_getcount32()) > uiADCt1 );	// Wait for ADC to power up (1st write of CR2_ADON bit)
	adc_start_cal_register_reset();		// Start reset of calibration registers (return 
	adc_start_calibration();		// Start calibration 

	return;
}



/* p 234
      Reading the temperature
      To use the sensor:
      1.   Select the ADCx_IN16 input channel.
      2.   Select a sample time of 17.1 μs
      3.   Set the TSVREFE bit in the ADC control register 2 (ADC_CR2) to wake up the
           temperature sensor from power down mode.
      4.   Start the ADC conversion by setting the ADON bit (or by external trigger).
      5.   Read the resulting VSENSE data in the ADC data register
      6.   Obtain the temperature using the following formula:
           Temperature (in °C) = {(V25 - VSENSE) / Avg_Slope} + 25.
           Where,
           V25 = VSENSE value for 25° C and
           Avg_Slope = Average Slope for curve between Temperature vs. VSENSE (given in
           mV/° C or μV/ °C).
           Refer to the Electrical characteristics section for the actual values of V25 and
           Avg_Slope.
Note: The sensor has a startup time after waking from power down mode before it can output
      VSENSE at the correct level. The ADC also has a startup time after power-on, so to minimize
      the delay, the ADON and TSVREFE bits should be set at the same time.

 p 105
Table 63.       TS characteristics
    Symbol                      Parameter            Min  Typ  Max   Unit
 TL(1)            VSENSE linearity with temperature       +/-1  +/-2   °C
 Avg_Slope        Average slope                       4.0  4.3  4.6 mV/°C
 V25              Voltage at 25 °C                   1.34 1.43 1.52    V
 tSTART(2)        Startup time                         4        10    μs
                  ADC sampling time when reading TS_temp      17.1   μs
*/

/*#######################################################################################
 * ISR routine for DMA1 Channel1 reading ADC regular sequence of adc channels
 *####################################################################################### */
void DMA1CH1_IRQHandler(void)
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
	return;
}

