/******************************************************************************
* File Name          : adcsensor_foto_h.c
* Date First Issued  : 01/31/2014
* Board              : RxT6
* Description        : ADC routines for f103 sensor--sensor board histogram collection
*******************************************************************************/
/*
02-07-2014 rev 370: poll-response scheme.  Error msg sending disabled.  Needs PC to retry if hangs.

02-05-2014: Hack of adcsensor_foto.c to include building a histogram of ADC readings
and providing a provision for a request|response readout of these readings via the
CAN bus.  See comments in 'int adc_histo_cansend(struct CANRCVBUF* p);' below for more
detail.

To store the readings DMA is required and with DMA enabled ADC1 data register has ADC2
readings stored in the high half of the ADC1 data register.  This renders the ADC1
watchdog inoperable.  The workaround is to use ADC1 DMA to store ADC2 readings and
ADC3 with its own DMA to store ADC3 readings.  ADC3 watchdog function replaces ADC1
of the early version.
*/
/*
07-13-2013: earlier rev 206 works with 64/sec compute rate.  Major changes started
    to cic filter rpm, with cic input at a higher than 64 rate.  Works except for
    some glitch when the rpm hovers around the no encoder counts to one count within
    one 1/64th sec frame.
*/
/* 
07/08/2013 This is a hack of svn_pod/sw_stm32/trunk/devices/adcsensor_pod.c
See p214 of Ref Manual for ADC section 

ADC3 and ADC2 are used for photocell A and B, respectively, emitter measurements.
The photocell transmit diode is driven by the 3.3v analog voltage via a 150 ohm resistor.
The photocell photodetector transistor emitter has a 470 ohm resistor to ground.  The 
voltage across the 470 ohm resistor is measured by the ADC.

Each ADC "scans" just one channel continuously.  The high and low threshold watchdog
registers are used to trigger an interrupt.  When the converted signal value is greater than
the high threshold register (HTR), or is less than the low threshold register (LTR), the
watddog flag is set, and when the watchdog interrupt is enabled an interrupt is triggered.

Each conversion that results in a value greater than the HTR, or less than the LTR, will
set the interrupt flag.

ADC1 and ADC2 have their interrupts mapped together.  Using DMA to store values causes the
ADC1 data register to be loaded with ADC2 in the upper half of the 32b register which means
the ADC1 watchdog register can never be higher, unless ADC2 reading happens to be zero.
Therefore, ADC3 is used with it's independent DMA channel and ADC1 DMA is used to store
ADC2 readings.

The interrupt routine checks the watchdog flag in the status register for each ADC and 
for the ADC that is set the data register is compared to the high threshold register 
to see if the interrupt was a measurement that was going high, or going low, thereby 
determining the 0|1 status of that photocell.

The interrupts are caused when the measurement completes and the reading is above the high
register threshold, or below the low register threshold.  To prevent interrupts after every
ADC conversion the high register threshold is raised to the the max so that no further interrupts 
can occur, and the low register is set with the low threshold.  When the signal drops and 
drops below the low register threshold the interrupt servicing sets the low register at zero 
so that it cannot cause further interrupts, and the high level is set at the high threshold.

The 0|1 levels of the two photocells is used to provide quadrature detection and counting
of the shaft.  The count is 4 counts for a segment.  The quadrature encoding is done using a
table lookup that is based on the new state and the previous ("old") state.  ADC1 and ADC2
provide 1 bit each, so the new state can be 00, 01, 11, 10.  When combined with the old state
there are 16 possibilities.  The table lookup gives +1, -1, and zero.  Zero is an illegal
change, e.g. 00 to 11 has both changing state at the same time.  The +/- 1 gives a signed
count to the shaft position.

NOTE:  There is an issue with regard to the ADC measurement rate being short enough that by the
time the interrupt service routine is serviced the next ADC measurement might have completed.  To
prevent multiple counts, the threshold register is first set to an impossible value, then the quadrature
logic executed, which allows time for the register value to be set in the peripheral.  If a 2nd completion 
occurred before the threshold register was set to an impossible value (and the measurement was
above|below the threshold register) the interrupt request would not change an already set interrupt
flag.  The last action in the interrupt service is to reset the interrupt flag, and at this point
in time the new, impossible threshold register value would prevent a 2nd interrupt request.  If the
foregoing sequence was not done, e.g. the interrupt flag reset at the beginning of the interrupt
service routine, it would be make possible a double count if the exact timing circumstances would
occur.

11/12/2012 -- Initial code for ADC1|2 approach.
11/16/2012 -- CAW & DEH Skype through the code

*/
/*
NOTE: Some page number refer to the Ref Manual RM0008, rev 11 and some more recent ones, rev 14.

 p 219
11.3.7 Analog watchdog
       The AWD analog watchdog status bit is set if the analog voltage converted by the ADC is
       below a low threshold or above a high threshold. These thresholds are programmed in the
       12 least significant bits of the ADC_HTR and ADC_LTR 16-bit registers. An interrupt can be
       enabled by using the AWDIE bit in the ADC_CR1 register.
       The threshold value is independent of the alignment selected by the ALIGN bit in the
       ADC_CR2 register. The comparison is done before the alignment (see Section 11.5).
       The analog watchdog can be enabled on one or more channels by configuring the
       ADC_CR1 register as shown in Table 69.
*/

#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/gpio.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/adc.h"
#include "libopenstm32/dma.h"
#include "libusartstm32/commonbitband.h"
#include "libmiscstm32/clockspecifysetup.h"

#include "common_can.h"
#include "common_misc.h"

#include "pinconfig_all.h"
#include "adcsensor_foto.h"
#include "../../../../svn_common/trunk/can_driver.h"
#include "SENSORpinconfig.h"

#include "libopenstm32/scb.h"
#include "libopenstm32/systick.h"

#include "../../../../svn_common/trunk/pay_type_cnvt.h"
#include "CANascii.h"
#include "../../../../svn_common/trunk/can_gps_phasing.h"
#include "adcsensor_foto_h.h"


/* Pointer to control block for CAN1. */
extern struct CAN_CTLBLOCK* pctl1;

/* Delay for CR2_ADON (adc power up) (about 1 us) */
#define DELAYCOUNTCR2_ADON	2	// Delay in microseconds

/* ADC usage (only Encoder A & B used)

PA0 ADC123-IN0	Encoder A (Throttle potentiometer)
PA1 ADC123-IN1  Encoder B (Thermistor)
PA3 ADC123-IN2	Pressure sensor  

*/
/*
Bits 29:0 SMPx[2:0]: Channel x Sample time selection (1)
           These bits are written by software to select the sample time individually for each channel.
           During sample cycles channel selection bits must remain unchanged.
           000: 1.5 cycles	@14MHz  0.4K max input R (p 123 datasheet)
           001: 7.5 cycles	@14MHz  5.9K max input R  ### (3.3K in prototype 11/15/2012)
           010: 13.5 cycles	@14MHz 11.4K max input R
           011: 28.5 cycles	@14MHz 25.2K max input R
           100: 41.5 cycles	@14MHz 37.2K max input R
           101: 55.5 cycles	@14MHz 50.0K max input R
           110: 71.5 cycles	50K max
           111: 239.5 cycles	50K max
(1) For less than 1/4 LSB of 12 bits conversion

With an ADC clock of 12MHz the total conversion time--
12.5 + SMPx time = 12.5 + 7.5 = 20 adc clock cycles
At 12 MHz, 20 cycles * 83 1/2 ns per adc clock cycle = 1 2/3 us per conversion
*/
// *CODE* for number of cycles in conversion on each adc channel
#define SMP1	1	// ### Photocell emitter A
#define SMP2	1	// ### Photocell emitter B
#define SMP3	1
#define SMP4	1
#define SMP5	1
#define SMP6	3
#define SMP7	3
#define SMP8	6
#define SMP9	6
#define SMP10	 1	
#define SMP11	 1		
#define SMP12	3
#define SMP13	3
#define SMP14	3
#define SMP15	3
#define SMP16	7
#define SMP17	7


/* ********** Static routines **************************************************/
static void adc_init_foto(void);
static void adc_start_conversion_foto(void);
static void adc_start_cal_register_reset_foto(void);
static void adc_start_calibration_foto(void);
static void compute_init(struct FLASHP_SE4* phfp);
static void savereadings2(void);
static void compute_doit2(void);
static s32 compute_speed(u32 ctrZ, u32 encode_timeZ, u32 ticktimeZ);
static void adc_histo_build_ADC12(void);
static void adc_histo_build_ADC3(void);

/* Error counter array */
u32 adcsensor_foto_err[ADCERRORCTRSIZE];
	// [0] =  Error count: shaft encoding illegal transitions
	// [1] =  Error count: for illogical tick durations (re: the systick interrupt priority caper.)
	// [2] =  Error count: for illogical tick durations (re: the systick interrupt priority caper.)
	// [3] =  Error count: cic out of sync with end of 1/64th sec


/* APB2 bus frequency is needed to set ADC prescalar so that it is less than 14 MHz,
   but as high as possible. */
extern unsigned int	pclk2_freq;	// APB2 bus frequency in Hz (see 'lib/libmiscstm32/clockspecifysetup.c')

/* SYSCLK frequency is used to convert millisecond delays into SYSTICK counts */
extern unsigned int	sysclk_freq;	// SYSCLK freq in Hz (see 'lib/libmiscstm32/clockspecifysetup.c')

/* High and low watchdog register threshold values */
static unsigned short usHtr1 = ADC3_HTR_INITIAL;	// ADC1 High register initial value
static unsigned short usLtr1 = ADC3_LTR_INITIAL;	// ADC1 Low  register initial valueit
static unsigned short usHtr2 = ADC2_HTR_INITIAL;	// ADC2 High register initial value
static unsigned short usLtr2 = ADC2_LTR_INITIAL;	// ADC2 Low  register initial value


/* DMA circular buffered with 1/2 way interrupt double buffers ADC1,2 pairs. */
//  [Double for DMA][pairs of 16b ADC readings with ADC1, ADC2]
 union ADC12VAL adc12valbuff[2][ADCVALBUFFSIZE];
static volatile int adc12valbuffflag = 0;		// 0 = 1st half; 1 = 2nd half
//  [Double for DMA][16b ADC readings with ADC3]
 unsigned short adc3valbuff[2][ADCVALBUFFSIZE];
static volatile int adc3valbuffflag = 0;	// 0 = 1st half; 1 = 2nd half

/* Pointer into histogram bins, used for readout. */
u32* can_msg_histo_ptr = 0;
static u32 histthrottle = 0;	// Throttle for readouts of bins

/* ADC3 ADC2 readings readout */
static u32 adcreadings_ctr = 0;
static struct CANRCVBUF can_msg_adc_5;	// CAN msg. Histogram tx: request count, switch buffers. rx: send count

/******************************************************************************
 * static int adv_index(int idx, int size);
 * @brief	: Format and print date time in readable form
 * @param	: idx = incoming index
 * @param	: size = number of items in FIFO
 * return	: index advanced by one
 ******************************************************************************/
static int adv_index(int idx, int size)
{
	int localidx = idx;
	localidx += 1; if (localidx >= size) localidx = 0;
	return localidx;
}

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
 * void adc_init_sequence_foto_h(struct FLASHP_SE4* phfp);
 * @brief 	: Call this routine to do a timed sequencing of power up and calibration
 * @param	: phfp = pointer to struct stored in highflashp area
*******************************************************************************/
void adc_init_sequence_foto_h(struct FLASHP_SE4* phfp)
{


	/* Set threshold registers from parameter area. */
	usHtr1 = phfp->adc3_htr_initial;	// High threshold registor setting, ADC3
	usLtr1 = phfp->adc3_ltr_initial;	// Low  threshold register setting, ADC3
	usHtr2 = phfp->adc2_htr_initial;	// High threshold registor setting, ADC2
	usLtr2 = phfp->adc2_ltr_initial;	// Low  threshold register setting, ADC2

//	usHtr1 = *(pthres + 0);	// ADC1 High register initial value
//	usLtr1 = *(pthres + 1);	// ADC1 Low  register initial value
//	usHtr2 = *(pthres + 2);	// ADC2 High register initial value
//	usLtr2 = *(pthres + 3);	// ADC2 Low  register initial value

	u32 ticksperus = (sysclk_freq/1000000);	// Number of ticks in one microsecond

	/* Set up for computations and CAN messaging shaft count and speed */
	compute_init(phfp);

	/* Initialize ADC1 & ADC2 */
	adc_init_foto();			// Initialize ADC1 and ADC2 registers.
	timedelay(50 * ticksperus);	// Wait

	/* Start the reset process for the calibration registers */
	adc_start_cal_register_reset_foto();
	timedelay(10 * ticksperus);	// Wait

	/* Start the register calibration process */
	adc_start_calibration_foto();
	timedelay(80 * ticksperus);	// Wait
	
	/* DMA has been setup, but not started. */
	DMA1_CCR1 |= DMA_CCR1_EN;	// Start stream
	DMA2_CCR5 |= DMA_CCR1_EN;	// Start stream

	/* ADC is now ready for use.  Start ADC conversions */
	adc_start_conversion_foto();	// Start ADC conversions and wait for ADC watchdog interrupts

	return;
}
/******************************************************************************
 * static void adc_init_foto(void);
 * @brief 	: Initialize adc for dma channel 1 transfer
*******************************************************************************/
static const struct PINCONFIGALL pin_pa0 = {(volatile u32 *)GPIOA, 0, IN_ANALOG, 0};
static const struct PINCONFIGALL pin_pa1 = {(volatile u32 *)GPIOA, 1, IN_ANALOG, 0};
static const struct PINCONFIGALL pin_pa2 = {(volatile u32 *)GPIOA, 2, IN_ANALOG, 0};

static void adc_init_foto(void)
{
	u32 ticksperus = (sysclk_freq/1000000);	// Number of ticks in one microsecond

	/*  Setup ADC pins for ANALOG INPUT (p 148) */
	pinconfig_all( (struct PINCONFIGALL *)&pin_pa0);
	pinconfig_all( (struct PINCONFIGALL *)&pin_pa1);
	pinconfig_all( (struct PINCONFIGALL *)&pin_pa2);

	/* Find prescalar divider code for the highest permitted ADC freq (which is 14 MHz) */
	unsigned char ucPrescalar = 3;		// Division by 8	
	if (pclk2_freq/8 < 14000000) ucPrescalar = 2;	// Division by 6
	if (pclk2_freq/6 < 14000000) ucPrescalar = 1;	// Division by 4
	if (pclk2_freq/4 < 14000000) ucPrescalar = 0;	// Division by 2

//ucPrescalar = 3;	// Slow it down for debugging and test

	/* Set APB2 bus divider for ADC clock */
	RCC_CFGR |= ( (ucPrescalar & 0x3) << 14)	; // Set code for bus division (p 92)

	/* Enable bus clocking for ADC */
	RCC_APB2ENR |= ( (RCC_APB2ENR_ADC1EN) | (RCC_APB2ENR_ADC2EN) | (RCC_APB2ENR_ADC3EN) );	// Enable ADC1, ADC2, ADC3 clocking (see p 104)
	
	/* Scan mode (p 236) 
"This bit is set and cleared by software to enable/disable Scan mode. In Scan mode, the
inputs selected through the ADC_SQRx or ADC_JSQRx registers are converted." 
	AWDEN - enable watchdog register p 229,30. */
	//         (   dual mode    |  scan        | watchdog enable | watchdog interrupt enable | watchdog channel number )
	ADC1_CR1 = (     (6 << 16)  | ADC_CR1_SCAN                                   		     );	// ADC1
	ADC2_CR1 = (                  ADC_CR1_SCAN | ADC_CR1_AWDEN   | ADC_CR1_AWDIE 		 | 1 );	// ADC2 chan 1
	ADC3_CR1 = (                  ADC_CR1_SCAN | ADC_CR1_AWDEN   | ADC_CR1_AWDIE 		 | 0 );	// ADC3 chan 0

	/*          (  Continuous   | Power ON     | DMA	) */
	ADC1_CR2  = ( ADC_CR2_CONT  | ADC_CR2_ADON | ADC_CR2_DMA); 
	ADC2_CR2  = ( ADC_CR2_CONT  | ADC_CR2_ADON              ); 
	ADC3_CR2  = ( ADC_CR2_CONT  | ADC_CR2_ADON | ADC_CR2_DMA); 

	/* 1 us Tstab time is required before writing a second 1 to ADON to start conversions 
	(see p 98 of datasheet) */
	timedelay(2 * ticksperus);	// Wait

	// ---Modified for sensor---
	/* Set sample times  */	
	ADC1_SMPR2 = ( (SMP1 << 0) );
	ADC2_SMPR2 = ( (SMP1 << 0) );
	ADC3_SMPR2 = ( (SMP1 << 0) );

	/* Set high and low threshold registers for both ADCs with initial values */
	ADC3_HTR = ADC3_HTR_INITIAL;
	ADC3_LTR = ADC3_LTR_INITIAL;
	ADC2_HTR = ADC2_HTR_INITIAL;
	ADC2_LTR = ADC2_LTR_INITIAL;

	/* Setup the number of channels scanned (zero = just one) (p 238) */
	ADC3_SQR1 = ( ( (NUMBERADCCHANNELS_FOTO-1) << 20) );	// Chan count, sequences 16 - 13 (p 217, 238)
	ADC2_SQR1 = ( ( (NUMBERADCCHANNELS_FOTO-1) << 20) );	// Chan count, sequences 16 - 13 (p 217, 238)
	ADC1_SQR1 = ( ( (NUMBERADCCHANNELS_FOTO-1) << 20) );	// Chan count, sequences 16 - 13 (p 217, 238)

	/* This maps the ADC channel number to the position in the conversion sequence */
	// Load channels IN11,IN10, for conversions sequences (p 240)
	ADC3_SQR3 = ( (0 << 0) ); // Conversion sequence number one sensor photocell A emitter measurement
	ADC2_SQR3 = ( (1 << 0) ); // Conversion sequence number one sensor photocell B emitter measurement
	ADC1_SQR3 = ( (2 << 0) ); // Conversion sequence number dummy

	/* Set and enable interrupt controller for ADCs. */
	NVICIPR (NVIC_ADC1_2_IRQ,ADC1_2_PRIORITY_FOTO );// Set interrupt priority (p 227)
	NVICISER(NVIC_ADC1_2_IRQ);			// Enable interrupt controller for ADC1|ADC2

	NVICIPR (NVIC_ADC3_IRQ,ADC1_2_PRIORITY_FOTO );	// Set interrupt priority (p 227) *SAME AS* ADC1|ADC2
	NVICISER(NVIC_ADC3_IRQ);			// Enable interrupt controller for ADC3

	/* Low level interrupt for doing adc filtering following DMA1 CH1 interrupt. */
	NVICIPR (NVIC_FSMC_IRQ, ADC_FSMC_PRIORITY);	// Set low level interrupt priority for post DMA1 interrupt processing
	NVICISER(NVIC_FSMC_IRQ);			// Enable low level interrupt

	/* Low level interrupt for doing adc filtering following DMA2 CH5 interrupt. */
	NVICIPR (NVIC_SDIO_IRQ, ADC_FSMC_PRIORITY);// Set low level interrupt priority for post DMA2 interrupt processing
	NVICISER(NVIC_SDIO_IRQ);			// Enable low level interrupt

	/* Setup DMA1 for storing data in the ADC_DR (with 32v ADC1|ADC2 pairs) */
	RCC_AHBENR |= RCC_AHBENR_DMA1EN;		// Enable DMA1 clock (p 102)
	DMA1_CNDTR1 = (2 * ADCVALBUFFSIZE);		// Number of data items before wrap-around
	DMA1_CPAR1 = (u32)&ADC1_DR;			// DMA1 channel 1 peripheral address (adc1|2 data register) (p 211, 247)
	DMA1_CMAR1 = (u32)&adc12valbuff[0][0];		// Memory address of first buffer array for storing data (p 211)

	// Channel configurion reg (p 209)
	//          priority high  | 32b mem xfrs | 32b adc xfrs | mem increment | circular mode | half xfr     | xfr complete   | dma chan 1 enable
	DMA1_CCR1 =  ( 0x02 << 12) | (0x02 << 10) |  (0x02 << 8) | DMA_CCR1_MINC | DMA_CCR1_CIRC |DMA_CCR1_HTIE | DMA_CCR1_TCIE  | 0 ;

	/* Set and enable interrupt controller for DMA transfer complete interrupt handling */
	NVICIPR (NVIC_DMA1_CHANNEL1_IRQ, DMA1_CH1_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_DMA1_CHANNEL1_IRQ);

	/* Setup DMA2 for storing data in the ADC_DR (with 16b ADC3) */
	RCC_AHBENR |= RCC_AHBENR_DMA2EN;		// Enable DMA2 clock
	DMA2_CNDTR5 = (2 * ADCVALBUFFSIZE);		// Number of data items before wrap-around
	DMA2_CPAR5 = (u32)&ADC3_DR;			// DMA2 channel 5 peripheral address (adc3 data register) (p 211, 247)
	DMA2_CMAR5 = (u32)&adc3valbuff[0][0];		// Memory address of first buffer array for storing data (p 211)

	// Channel configurion reg (p 209)
	//          priority high  | 16b mem xfrs | 16b adc xfrs | mem increment | circular mode | half xfr     | xfr complete   | dma chan 1 enable
	DMA2_CCR5 =  ( 0x02 << 12) | (0x01 << 10) |  (0x01 << 8) | DMA_CCR5_MINC | DMA_CCR5_CIRC |DMA_CCR5_HTIE | DMA_CCR5_TCIE  | 0 ;

	/* Set and enable interrupt controller for DMA transfer complete interrupt handling */
	NVICIPR (NVIC_DMA2_CHANNEL4_5_IRQ, DMA1_CH1_PRIORITY );	// Set interrupt priority *SAME AS* DMA1 for ADC1|ADC2
	NVICISER(NVIC_DMA2_CHANNEL4_5_IRQ);

	return;
}
/******************************************************************************
 * static void adc_start_cal_register_reset_foto(void);
 * @brief 	: Start calibration register reset
 * @return	: Current 32 bit SYSTICK count
*******************************************************************************/
static void adc_start_cal_register_reset_foto(void)
{
	/* Reset calibration register  */
	ADC3_CR2 |= ADC_CR2_RSTCAL;			// Turn on RSTCAL bit to start calibration register reset
	ADC2_CR2 |= ADC_CR2_RSTCAL;			// Turn on RSTCAL bit to start calibration register reset
	
	/* Wait for register to reset */
	while ((ADC3_CR2 & ADC_CR2_RSTCAL) != 0);
	while ((ADC2_CR2 & ADC_CR2_RSTCAL) != 0);

	return;
}
/******************************************************************************
 * static void adc_start_calibration_foto(void);
 * @brief 	: Start calibration
*******************************************************************************/
static void adc_start_calibration_foto(void)
{
	/* Start calibration  */
	ADC2_CR2 |= ADC_CR2_CAL;				// Turn on RSTCAL bit to start calibration register reset
	ADC3_CR2 |= ADC_CR2_CAL;				// Turn on RSTCAL bit to start calibration register reset

	/* Wait for calibration to complete (about 7 us */
	while ((ADC2_CR2 & ADC_CR2_CAL) != 0);
	while ((ADC3_CR2 & ADC_CR2_CAL) != 0);

	return;
}
/******************************************************************************
 * static unsigned int adc_start_conversion_foto(void);
 * @brief 	: Start a conversion of the regular sequence
*******************************************************************************/
static void adc_start_conversion_foto(void)
{	
/* After the initial write of this bit, subsequent writes start the ADC conversion--(p 241)
"Conversion starts when this bit holds a value of 1 and a 1 is written to it. The application."
should allow a delay of tSTAB between power up and start of conversion. Refer to Figure 27 */
	ADC3_CR2 |= ADC_CR2_ADON;		// Writing a 1 starts the conversion (see p 238)
	ADC2_CR2 |= ADC_CR2_ADON;		// Writing a 1 starts the conversion (see p 238)
	ADC1_CR2 |= ADC_CR2_ADON;		// Writing a 1 starts the conversion (see p 238)

	return;
}
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
*/
	if ( (DMA1_ISR & DMA_ISR_TCIF1) != 0 )	// Is this a transfer complete interrupt?
	{ // Here, yes.  The second sequence has been converted and stored in the second buffer
		adc12valbuffflag  = 1;		// Set the index to the second buffer.  It is ready.
		DMA1_IFCR = DMA_IFCR_CTCIF1;	// Clear transfer complete flag (p 208)
	}
	if ( (DMA1_ISR & DMA_ISR_HTIF1) != 0 )	// Is this a half transfer complete interrupt?
	{ // Here, yes.  The first sequence has been converted and stored in the first buffer
		adc12valbuffflag  = 0;		// Set index to the first buffer.  It is ready.
		DMA1_IFCR = DMA_IFCR_CHTIF1;	// Clear transfer complete flag (p 208)
	}

	/* Trigger a pending interrupt that will handle filter the ADC readings. */
	NVICISPR(NVIC_FSMC_IRQ);		// Set pending (low priority) interrupt for  ('../lib/libusartstm32/nvicdirect.h')

	return;
}
/*#######################################################################################
 * ISR routine for DMA2 Channel5 reading ADC regular sequence of adc channels on ADC3
 *####################################################################################### */
void DMA2CH4_5_IRQHandler(void)
{
/* Double buffer the sequence of channels converted.  When the DMA goes from the first
to the second buffer a half complete interrupt is generated.  When it completes the
storing of two buffers full a transfer complete interrut is issued, and the DMA address
pointer is automatically reloaded with the beginning of the buffer space (i.e. circular). 

'flg' is the high order index into the two dimensional array. 
*/
	if ( (DMA2_ISR & DMA_ISR_TCIF5) != 0 )	// Is this a transfer complete interrupt?
	{ // Here, yes.  The second sequence has been converted and stored in the second buffer
		adc3valbuffflag  = 1;		// Set the index to the second buffer.  It is ready.
		DMA2_IFCR = DMA_IFCR_CTCIF5;	// Clear transfer complete flag (p 208)
	}
	if ( (DMA2_ISR & DMA_ISR_HTIF5) != 0 )	// Is this a half transfer complete interrupt?
	{ // Here, yes.  The first sequence has been converted and stored in the first buffer
		adc3valbuffflag  = 0;		// Set index to the first buffer.  It is ready.
		DMA2_IFCR = DMA_IFCR_CHTIF5;	// Clear transfer complete flag (p 208)
	}

	/* Trigger a pending interrupt that will handle filter the ADC readings. */
	NVICISPR(NVIC_SDIO_IRQ);		// Set pending (low priority) interrupt for  ('../lib/libusartstm32/nvicdirect.h')

	return;
}
/*#######################################################################################
 * ISR routine for ADC 1 & ADC2 (they are mapped together by ST to make life difficult)
 * ADC1 is not used because the DMA stores ADC2 in the high half of the ADC1 DR making the
 * watchdog inoperable.  Therefore, Foto A uses ADC3 with DMA5CH5, and Foto B uses ADC2, and ADC1 
 * with DMA1CH1 to store ADC2.
 *####################################################################################### */
static unsigned short new;	// Latest 0|1 pair--bit 0 = ADC1 (photocell A); bit 1 = ADC2 (photocell B);
static unsigned short old;	// Saved "new" from last interrupt.

/* Values generated in interrupt routine */
s32 encoder_ctr2;	// Shaft encoder running count (+/-)
u32 encoder_Actr2;	// Cumulative count--photocell A
u32 encoder_Bctr2;	// Cumulative count--photocell B
u32 adc_encode_time2;	// DTW_CYCCNT time of last transition


u32 adc_encode_time_prev2;	// Previous time at SYSTICK
s32 encoder_ctr_prev2;		// Previous encoder running count

static unsigned int speedzeroflg = 0;	// not-zero = wait for a encoder tick


/* Table lookup for shaft encoding count: -1 = negative direction count, +1 = positive direction count, 0 = error */
/* One sequence of 00 01 11 10 results in four counts. */
static const signed char en_state[16] = {0,+1,-1, 0,-1, 0, 0,+1,+1, 0, 0,-1, 0,-1,+1, 0};

void ADC1_2_IRQHandler2(void)
{ // Here, interrupt of ADC1 (Photocell A) or ADC1 (Photocell B)
     volatile int temp;	// Register readback to assure a prior write has been taken by the register
	int both;	// Table lookup index = ((old << 2) | new)

	/* ADC2, IN1, Photocell B  */	
	if ((ADC2_SR & ADC_SR_AWD) != 0)	// ADC2 watchdog interrupt?
	{ // Here, yes.  Watchdog flag is on
		/* Is the data *above* the high threshold register? (p 219) */
		if (ADC2_DR > ADC2_HTR)
		{ // Here, watchdog interrupt was the signal going upwards
			ADC2_HTR = 0x0fff;	// Change high register to max and impossible to exceed
			ADC2_LTR = usLtr2;	// Bring low register up to threshold
			new |= 0x02;		// Turn 0|1 status bit ON
			LED19RED_off;
		}
		else
		{ // Here, the watchdog interrupt was the signal going downwards
			ADC2_LTR = 0;		// Set low register to impossible
			ADC2_HTR = usHtr2;	// Set high register to threshold
			new &= ~0x02;		// Turn 0|1 status bit OFF
			LED19RED_on;
		}
		encoder_Bctr2 += 1;		// Cumulative counter--either direction
		both = (old << 2) | new;	// Make lookup index
		encoder_ctr2 += en_state[both];	// Add +1, -1, or 0
		if (en_state[both] == 0)	// Was this an error
			adcsensor_foto_err[0] += 1;	// Cumulative count
		/* Save time of shaft encoding transitions (for speed computation) */
		adc_encode_time2 = *(volatile unsigned int *)0xE0001004; // DWT_CYCNT
		old = new;			// Update for next 
		temp = ADC2_HTR | ADC2_LTR;	// Be sure new threshold values are set before turning off interrupt flag
		ADC2_SR = ~0x1;			// Reset adc watchdog flag
		temp = ADC2_SR;			// Readback assures interrupt flag is off (prevent tail-chaining)
	}
	return;
}
/*#######################################################################################
 * ISR routine for ADC3
 *####################################################################################### */
void ADC3_IRQHandler(void)
{ // Here, interrupt of ADC1 (Photocell A) or ADC1 (Photocell B)
     volatile int temp;	// Register readback to assure a prior write has been taken by the register
	int both;	// Table lookup index = ((old << 2) | new)

	/* ADC3, IN0, Photocell A  */
	if ((ADC3_SR & ADC_SR_AWD) != 0)	// ADC1 watchdog interrupt?
/* Note: bit banding the above 'if' makes not a difference in the number of instructions compiled. */
	{ // Here, yes.  Watchdog flag is on
		/* Is the data *above* the high threshold register? (p 219) */
		if (ADC3_DR > ADC3_HTR)
		{ // Here, watchdog interrupt was the signal going upwards
			ADC3_HTR = 0x0fff;	// Change high register to max and impossible to exceed
			ADC3_LTR = usLtr1;	// Bring low register up to threshold
			new |= 0x01;		// Turn 0|1 status bit ON
			LED20RED_off;
		}
		else
		{ // Here, the watchdog interrupt was the signal going downwards
			ADC3_LTR = 0;		// Set low register to impossible
			ADC3_HTR = usHtr1;	// Set high register to threshold
			new &= ~0x01;		// Turn 0|1 status bit OFF
			LED20RED_on;
		}
		encoder_Actr2 += 1;		// Cumulative counter--either direction
		both = (old << 2) | new;	// Make lookup index
		encoder_ctr2 += en_state[both];	// Add +1, -1, or 0
		if (en_state[both] == 0)	// Was this an error
			adcsensor_foto_err[0] += 1;	// Cumulative count
		/* Save time of shaft encoding transitions (for speed computation) */
		adc_encode_time2 = *(volatile unsigned int *)0xE0001004; // DWT_CYCNT
		old = new;			// Update for next
		temp = ADC3_HTR | ADC3_LTR;	// Be sure new threshold values are set before turning off interrupt flag
		ADC3_SR = ~0x1;			// Reset adc watchdog interrupt flag
		temp = ADC3_SR;			// Readback assures interrupt flag is off (prevent tail-chaining)
	}
	return;
}
/* $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
         Below deals with the handling of the encoding done in the foregoing routines
   $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ */
/******************************************************************************
 * static void compute_init(struct FLASHP_SE4* phfp);
 * @brief 	: Initialize for computations & sending CAN msg
 * @param	: pointer to highflashp with parameters.
*******************************************************************************/
/* **** CIC filtering stuff **** */
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

#include "cic_filter_l_N2_M3.h"

/* Recent readings for mainline monitoring */
long	speed_filteredA2;	// Most recent computed & filtered rpm
u32 	encoder_ctrA2;		// Most recent encoder count

static struct CICLN2M3 rpm_cic;		// Double buffer CIC intermediate storage
static struct CANRCVBUF can_msg_DS;	// CAN msg. Drive Shaft--odometer|speed
static struct CANRCVBUF can_msg_ER1;	// CAN msg. Errors
static struct CANRCVBUF can_msg_ER2;	// CAN msg. Errors
static struct CANRCVBUF can_msg_histo31;	// CAN msg. Histogram tx: request count, switch buffers. rx: send count
static struct CANRCVBUF can_msg_histo32;	// CAN msg. Histogram tx: bin number, rx: send bin count
static struct CANRCVBUF can_msg_histo21;	// CAN msg. Histogram tx: request count, switch buffers; rx send count
static struct CANRCVBUF can_msg_histo22;	// CAN msg. Histogram tx: bin number, rx: send bin count
static struct CANRCVBUF* can_msg_ptr1 = 0;	// 1st can msg pointer
static struct CANRCVBUF* can_msg_ptr2 = 0;	// 2nd can msg pointer

/* Pointers to functions to be executed under a low priority interrupt */
// These hold the address of the function that will be called
void 	(*systickHIpriority3_ptr)(void) = 0;	// SYSTICK handler (very high priority) continuation
void 	(*systickLOpriority3X_ptr)(void) = 0;	// SYSTICK handler (low high priority) continuation--1/64th

static void compute_init(struct FLASHP_SE4* phfp)
{
	int j;

	/* Initialize the structs that hold the CIC filtering intermediate values. */
	rpm_cic.usDecimateNum = DECIMATION_FOTO; // Decimation number
	rpm_cic.usDecimateCt = 0;		// Decimation counter
	rpm_cic.usDiscard = DISCARD_FOTO;	// Initial discard count
	rpm_cic.usFlag = 0;			// 1/2 buffer flag
	for (j = 0; j < 3; j++)
	{ // Very important that the integrators begin with zero.
		rpm_cic.lIntegral[j] = 0;
		rpm_cic.lDiff[j][0]  = 0;
		rpm_cic.lDiff[j][1]  = 0;
	}

	/* CAN messages for msgs sent periodically. */
	can_msg_DS.id  = phfp->canid_se4h_counternspeed;	// CAN msg: count & rpm
	can_msg_ER1.id = phfp->canid_error2;			// [0]encode_state er ct [1]adctick_diff<2000 ct
	can_msg_ER2.id = phfp->canid_error1;			// [2]elapsed_ticks_no_adcticks<2000 ct  [3]cic not in sync

	/* CAN messages for histogram readout. */
	can_msg_histo31.id = phfp->canid_adc3_histogramA;	// ADC3 Histogram tx: request count, switch buffers. rx: send count
	can_msg_histo32.id = phfp->canid_adc3_histogramB;	// ADC3 Histogram tx: bin number, rx: send bin count
	can_msg_histo21.id = phfp->canid_adc2_histogramA;	// ADC2 Histogram tx: request count, switch buffers; rx send count
	can_msg_histo22.id = phfp->canid_adc2_histogramB;	// ADC2 Histogram tx: bin number, rx: send bin count
	can_msg_adc_5.id   = phfp->canid_adc3_adc2_readout;	// ADC3 ADC2 readings readout

	can_msg_DS.dlc  = 8;
	can_msg_ER1.dlc = 8;
	can_msg_ER2.dlc = 8;

	can_msg_histo31.dlc = 4;
	can_msg_histo32.dlc = 8;
	can_msg_histo21.dlc = 4;
	can_msg_histo22.dlc = 8;
	can_msg_adc_5.dlc   = 8;

	can_msg_histo32.cd.ui[1] = ADCHISTOSIZE; // Set 'bin' number to max to indicated completed readout
	can_msg_histo22.cd.ui[1] = ADCHISTOSIZE;
	adcreadings_ctr = ADC3ADC2READCT;	// Shutdown count for limiting readings

	/* This is a low priority post SYSTICK interrupt call. */
	systickLOpriority_ptr = &compute_doit2;

	/* This is a very high priority post SYSTICK interrupt call. */
	systickHIpriority_ptr = &savereadings2;

	return;
}
/* *********************************************************************************
 * Place CAN msg in output queue
 ************************************************************************************/
void can_msg_put(struct CANRCVBUF *pcan)
{ // note: pctl1 via extern
	can_driver_put(pctl1, pcan, 8, 0);
	return;
}
/* *********************************************************************************
 * Set up payload with two signed ints and send
 ************************************************************************************/
void canmsg_send(struct CANRCVBUF* pcan, int pay1, int pay2)
{
	pay_type_cnvt_S32_S32toPay_S32_S32(&pcan->cd.uc[0], pay1, pay2);
	can_msg_put(pcan);
	return;
}
/* **********************************************************************************
// Send the msgs once and use cangate to see if they are correct.
 ************************************************************************************/
void testfoto(void)
{
can_msg_put(&can_msg_DS);	// 0 highflash table index
can_msg_put(&can_msg_ER1);	// 1
can_msg_put(&can_msg_ER2);	// 2
can_msg_put(&can_msg_histo31);	// 3
can_msg_put(&can_msg_histo32);	// 4
can_msg_put(&can_msg_histo21);	// 5
can_msg_put(&can_msg_histo22);	// 6
can_msg_put(&can_msg_adc_5);	// 7
};

/* ######################### UNDER HIGH PRIORITY SYSTICK INTERRUPT ############################### 
 * Enter from SYSTICK interrupt each 64 per sec, and buffer latest readings
 * ############################################################################################### */
/* Circular buffer saved data */
#define RPMDATABUFFSIZE	8
static int idxsaveh = 0;	// Index for storing new data
static int idxsavel = 0;	// Index for retrieving data for 'compute_doit2'
static u32 encoder_ctrZ[RPMDATABUFFSIZE];
static u32 adc_encode_timeZ[RPMDATABUFFSIZE];
static u32 systicktimeZ[RPMDATABUFFSIZE];
static u32 stk_32ctrZ[RPMDATABUFFSIZE];

static void savereadings2(void)
{
	volatile u32 temp;

	if ( ((stk_32ctr & 0x1f) & ( (32/DECIMATION_FOTO)-1) ) != ((32/DECIMATION_FOTO)-1) ) return;

	/* Disable interrupts that might change readings while we copy them */
	ADC3_CR1 &= ~ADC_CR1_AWDIE;	// ADC1 chan 0
	ADC2_CR1 &= ~ADC_CR1_AWDIE;	// ADC2 chan 1
	temp = ADC3_CR1;	// Readback to assure disabled
	temp = ADC2_CR2;	// Readback to assure disabled

	/* Save most recent values */
	encoder_ctrZ[idxsaveh]     = encoder_ctr2;		// Shaft encoder running count (+/-)
	adc_encode_timeZ[idxsaveh] = adc_encode_time2;	// DTW_CYCCNT time of last transition

	/* This "read time" must follow the above so that it is not possible to have a ADC interrupt
           store the time after the SYSTICK interrupt, which would then cause a small negative
           difference in the times */
	systicktimeZ[idxsaveh] = *(volatile unsigned int *)0xE0001004; // DWT_CYCNT;
	stk_32ctrZ[idxsaveh] = (stk_32ctr & 0x1f); // Save low order bits: 0 - 31 for sub-tick within 1/64th sec
 
	/* Re-enable interrupts */
	ADC3_CR1 |= ADC_CR1_AWDIE;	// ADC1 chan 0
	ADC2_CR1 |= ADC_CR1_AWDIE;	// ADC2 chan 1

	/* Advance index for high priority storing. */
	idxsaveh = adv_index(idxsaveh, RPMDATABUFFSIZE);

	/* Call other routines if an address is set up--64 per sec. */
	if (systickHIpriority3_ptr != 0)	// Having no address for the following is bad.
		(*systickHIpriority3_ptr)();	// Go do something

	return;
}
/* ########################## UNDER LOW PRIORITY SYSTICK INTERRUPT ############################### 
 * Do the computations and send the CAN msg
 * ############################################################################################### */
/* 
   This routine is entered from the SYSTICK interrupt handler triggering I2C2_EV low priority interrupt. 
*/


static void compute_doit2(void)
{
	s32 	speed_filtered = 0;
//	struct CANRCVTIMBUF* pcanrcvtim;

	/* Handle all buffered sets of data.  Normally one, but could be more. */
	while (idxsaveh != idxsavel)
	{
		/* Compute speed and load into filter struct. */
		rpm_cic.nIn = compute_speed(encoder_ctrZ[idxsavel],adc_encode_timeZ[idxsavel],systicktimeZ[idxsavel]);

		/* Run filter on the value just loaded into the struct. */
		if ( cic_filter_l_N2_M3 (&rpm_cic) != 0) // Filtering complete?
		{ // Here, yes.
			speed_filtered = rpm_cic.lout >> CICSCALE_FOTO; // Scale and save
			speed_filteredA2 = speed_filtered;	// Save for mainline monitoring
		}
		/* stk_32ctr == 31 on the last interrupt of the 32 comprising 1/64th sec.  It marks the 
        	   end and beginning point between two 1/64th sec intervals. */
		if (stk_32ctrZ[idxsavel] == 31)
		{ // Here, yes.  Time to setup for sending

			/* ========= Here is where adc readings get setup for sending ============ */
			/* Setup counter and speed readings for sending */
			canmsg_send(&can_msg_DS, encoder_ctrZ[idxsavel], speed_filtered);
			encoder_ctrA2 = encoder_ctrZ[idxsavel];	// Save for mainline monitoring
			
			/* We should have come out even, but check jic. */
			if (rpm_cic.usDecimateCt != 0)
			{
				adcsensor_foto_err[3] += 1;	// Count errors for cic not in sync with 1/64th sec end.
				rpm_cic.usDecimateCt = 0; 	// Re-sync
			}
			
			/* Send error msgs.  When the low order ticks of the time match our UNITID we can send one message. 
 			   There are 64 "slots" in a second, so when the low order bits of the time tick are 32-63, then we
                           can send another error message.  There are 32 possible UNITIDs, so if we only allow one error msg
			   per 1/64th sec time slot, then two per sec is permissible. */
//			pcanrcvtim = canrcvtim_get_sys();
//			if (pcanrcvtim != 0) // If we have a time msg, display the 1/64th ticks
//			{
//				if ( (pcanrcvtim->R.cd.ui[0] & 0x3f) == iamunitnumbershifted)
//					canmsg_send(&can_msg_ER1, adcsensor_foto_err[0],adcsensor_foto_err[1]);
//				if ( (pcanrcvtim->R.cd.ui[0] & 0x3f) == (iamunitnumbershifted << 1))
//					canmsg_send(&can_msg_ER2, adcsensor_foto_err[2],adcsensor_foto_err[3]);
//			}

			/* Call other routines if an address is set up. */
			if (systickLOpriority3X_ptr != 0)	// Having no address for the following is bad.
				(*systickLOpriority3X_ptr)();	// Go do something

			/* This is for cangate command 'a' that displays printf data. */		
			while ( CANascii_send() != 0); // Output any can msgs buffered with printf ascii
		}
		/* Advance index for low priority processing. */
		idxsavel = adv_index(idxsavel, RPMDATABUFFSIZE);
	}

	/* Setup histogram CAN msgs.  Msg #1 once, then next time Msg #2 'HISTO' times each pass until complete. */
	if (can_msg_ptr1 != 0) // JIC
	{ // Here, sent 1st msg (with total counts)
		can_msg_put(can_msg_ptr1);
		histthrottle = 0;
	}
	/* Send some 2nd msgs with bin counts, each pass, if ptr is setup and 1st msg has been sent. */
	if ( (can_msg_histo_ptr != 0) && (can_msg_ptr1 == 0) && (can_msg_ptr2 != 0) )
	{ // Here: valid ptrs and 1st msg has been sent.
	  // can_msg_ptr2->cd.ui[1] has 'bin' number (0 - (ADCHISTOSIZE-1))
		if ( (can_msg_ptr2->cd.ui[1] < ADCHISTOSIZE) && (histthrottle > THROTTLE) )
		{
			can_msg_ptr2->cd.ui[0] = *can_msg_histo_ptr++;	// Set count in payload
			can_msg_put(can_msg_ptr2);			// Send msg2
			can_msg_ptr2->cd.ui[1] += 1;			// Advance bin number
			histthrottle = 0;
		}
	}
	can_msg_ptr1 = 0; // Show that 1st msg has been sent.

	if ((adcreadings_ctr < ADC3ADC2READCT) && (histthrottle > THROTTLE2))
	{
		histthrottle = 0;
		can_msg_adc_5.cd.ui[0] = adc3valbuff[0][0];
		can_msg_adc_5.cd.ui[1] = adc12valbuff[0][0].us[1];
		can_msg_put(&can_msg_adc_5);			// Send msg5: ADC3, ADC2 readings
		adcreadings_ctr += 1;
	}
	histthrottle += 1;

	return;
}

/******************************************************************************
 * static s32 speed_arith(u32 numerator, s32 denominator);
 * @brief	: Do the arithemtic to compute pulley speed (with tick rate adjustment)
 * @return	: speed in RPS, signed, scaled by 10x and by number of cts/rev
 *              : For RPM multiply result by: 60 / (number of cts per revolution)
 *              : Number of cts per rev = Number of segments (black + white) * 2.
*******************************************************************************/
/*
If there are 5 black and 5 white bars per revolution, then there are 20 counts per revolution, then
divide the output by by 20 to get rps, or to get rpm multiply the value output by the following 
computation by 60 and divide by 20, i.e. just multiply by 3 to get rpm.
*/
static s32 speed_arith(u32 numerator, s32 denominator)
{
	s32 x = deviation_one64th/(1<<TIMESCALE);	// Average ticks deviation per 1/64th sec (scaled)
	x *= numerator;
	x /= 1000000;
	x = numerator - x;
	x /= 10;		// Scale upwards by 10x
	return ( (64000000) * denominator) / x;
}

//#include <float.h>
//static s32 speed_arith_fp(u32 numerator, s32 denominator)
//{
//	float clk = ticks64thideal - ((float)(deviation_one64th)/(1<<TIMESCALE));
//	s32 y = (denominator * 64E13) /(numerator * clk);
//	return y;
//}

/**************************************************************************************************
 * Compute speed from all the wonderful readings we have at our disposal.
***************************************************************************************************/
static s32 ctrZ_prev;
static u32 encode_timeZ_prev;
static s32 last_direction;	

static s32 compute_speed(u32 ctrZ, u32 encode_timeZ, u32 ticktimeZ)
{
	s32 speed;
	u32 adctick_diff;
	u32 elapsed_ticks_no_adcticks;
	s32 ztick;

	/* Compute the number of encoder ticks in this 1/64th sec interval. (NOTE: speed can be negative!) */
	s32 encoder_diff = ctrZ - ctrZ_prev;
	ctrZ_prev = ctrZ;


	if (encoder_diff != 0) // (Encoder diff can be + or -)
	{ // Here, one or more encoder pulses during the last 1/64th sec interval

		if (speedzeroflg == 0)
		{ // Here, we are not in a zero speed situation 
			/* System DWT ticks between latest ADC interrupt and the previous latest. */
			adctick_diff = encode_timeZ - encode_timeZ_prev;
			encode_timeZ_prev = encode_timeZ;
			last_direction = encoder_diff;	// Save direction of rotation (for handling zero tick case)

//volatile u32  dbg_t0 = *(volatile unsigned int *)0xE0001004; // DWT_CYCNT--measure execution duration
			speed = speed_arith(adctick_diff,encoder_diff);
//ADC_dbg0 = *(volatile unsigned int *)0xE0001004 - dbg_t0; // Measure time

			if (adctick_diff < 2000) adcsensor_foto_err[1] += 1;
		}
		else
		{ // Here, we were in "zero mode".
			speedzeroflg = 0;		// Reset the flag that forces speed to be zero.
			speed = 0;			// Next cycle will compute a non-zero rpm.
		}
	}
	else
	{ // Here, there were no encoder pulses during the last 1/256th sec interval.

		/* System DWT ticks between SYSTICK interrupt and last ADC interrupt */
		elapsed_ticks_no_adcticks = ticktimeZ - encode_timeZ;

		if (speedzeroflg == 0)
		{ // Here, we have't timed out for zero rpm. (About 1/4 sec)
			if ( elapsed_ticks_no_adcticks > 8000000 )
			{ // Here, we timed out.
				speedzeroflg = 1;	// Set timed out flag.
				speed = 0;		// Force rpm to be zero.
			}
			else
			{ // Here, we haven't timed out, so compute a (declining) rpm.
				ztick = -1;
				if (last_direction > 0 ) ztick = 1;
				speed = speed_arith(elapsed_ticks_no_adcticks, ztick);
				if (elapsed_ticks_no_adcticks < 2000) adcsensor_foto_err[2] += 1;
			}
		}
		else
		{ // Here, we timed out and now are waiting for a rpm pulse to get us off of a forced zero.
				speed = 0;		// Force rpm to be zero.
		}
	}
	return speed;
}
/*#######################################################################################
 * ISR routine for handling lower priority procesing
 *####################################################################################### */
/* Pointer to functions to be executed under a low priority interrupt, forced by DMA interrupt. */
void 	(*dma_ll_ptr)(void) = 0;		// DMA1CH1 -> FSMC  (low priority)

void FSMC_IRQHandler(void)
{
	adc_histo_build_ADC12();	// Add buffer readings to histogram

	/* Call other routines if an address is set up */
	if (dma_ll_ptr != 0)	// Having no address for the following is bad.
		(*dma_ll_ptr)();	// Go do something
	return;
}
/******************************************************************************
 * static void adc_histo_build_ADC12(void);
 * @brief 	: Add ADC readings from DMA buffer to histogram counts
 * WARNING: This is called from low priority level interrupt: FSMC_IRQHandler
*******************************************************************************/
/*  DMA1CH1 stores ADC1 & ADC2 upon each completion of ADC1 as two 16b readings.
When the DMA reaches the 1/2 way point it gives an interrupt and 'adc12valbuffflag' is
set to zero.  When it reaches the end that flag is set to one.  That flag is always
the index into the buffer not being filled.

DMA2CH5 stores ADC3 readings as one 16b reading.

After a DMA interrupt the low priority level interrupt is triggered.  This interrupt
calls the following routine to add to the 'bin' counts to make a histogram.  The
histogram bins are double buffered, and 'adc2histo_build' points to the histogram buffer
that is being built.
*/
/* Double buffer histogram */
//  [double buffer output][two ADCs][number of bins]
//  Buffer indices: ADC3 = 0; ADC2 = 1;
 u32 adc2histo[2][ADCHISTOSIZE];
 u32 adc3histo[2][ADCHISTOSIZE];
 int adc2histo_build = 0;	// 0 = 1st half; 1 = 2nd half
 int adc3histo_build = 0;	// 0 = 1st half; 1 = 2nd half
 int adc2histo_send = 1;	// 0 = 1st half; 1 = 2nd half
 int adc3histo_send = 1;	// 0 = 1st half; 1 = 2nd half
 int adc2histoctr[2];		// Number of readings in histogram
 int adc3histoctr[2];		// Number of readings in histogram
 int adc2histo_sw = 0;		// Switch histogram buffer request switch
 int adc3histo_sw = 0;		// Switch histogram buffer request switch

// Note: ADC range = 0-4095; 4096/64 -> 64; ADC = 0-63 goes into bin 0

static void adc_histo_build_ADC12(void)
{
	u32* ph2a;
	u32* pend1;
	int x;

	/* Switch histogram buffer request? */
	if (adc2histo_sw != 0)
	{ // Here yes.  Reset and zero next buffer
		adc2histo_sw = 0;	// Reset switch
		can_msg_histo21.cd.ui[0] = adc2histoctr[adc2histo_build]; // CAN msg payload
		can_msg_ptr1 = &can_msg_histo21;

		x = adc2histo_build;	// Swap buffer indices
		adc2histo_build = adc2histo_send;
		adc2histo_send = x;

		/* Zero out buffer */
		ph2a  = &adc2histo[adc2histo_build][0];
		pend1 = &adc2histo[adc2histo_build][ADCVALBUFFSIZE];
		while (ph2a < pend1)
			*ph2a++ = 0;
		adc2histoctr[adc2histo_build] = 0;

		/* Set up for 2nd msg sending (after 1st msg is sent) */
		can_msg_ptr2 = &can_msg_histo22; // Show systick entry, above, that CAN msg is ready to send
		can_msg_histo_ptr = &adc2histo[adc3histo_send][0]; // Pointer into histo counts
		// The following is the one that triggers off the sending
		can_msg_histo22.cd.ui[1] = 0; // Reset bin number for responses with histogram counts
	}

	/* Use pointers to make a fast loop. */
	union ADC12VAL* p    = &adc12valbuff[adc12valbuffflag][0];
	union ADC12VAL* pend = &adc12valbuff[adc12valbuffflag][ADCVALBUFFSIZE];
	u32* ph2 = &adc2histo[adc2histo_build][0];	// ADC2 histogram
	u32 tmp;

	/* Build bins */
	while (p < pend)
	{
		tmp = ((p->us[1]) >> 6) & 63;	// ADC2 into 64 ranges
		*(ph2 + tmp) += 1;		// Increment bin(n) count
		p++;
	}
	/* Keep a count of total readings in the histogram. */
	adc2histoctr[adc2histo_build] += 1;
	
	return;
}

/*#######################################################################################
 * ISR routine for handling lower priority procesing
 *####################################################################################### */
/* Pointer to functions to be executed under a low priority interrupt, forced by DMA interrupt. */
void 	(*dma2_ll_ptr)(void) = 0;		// DMA2CH5 -> SDIO_IRQHandler (low priority)

void SDIO_IRQHandler(void)
{
	adc_histo_build_ADC3();	// Add buffer readings to histogram


	/* Call other routines if an address is set up */
	if (dma2_ll_ptr != 0)	// Having no address for the following is bad.
		(*dma2_ll_ptr)();	// Go do something
	return;
}
/******************************************************************************
 * static void adc_histo_build_ADC3(void);
 * @brief 	: Add ADC readings from DMA buffer to histogram counts
 * WARNING: This is called from low priority level interrupt: FSMC_IRQHandler
*******************************************************************************/
static void adc_histo_build_ADC3(void)
{
	u32* ph3;
	u32* pend3;
	int x;

	/* Switch histogram buffer request? */
	if (adc3histo_sw != 0)
	{
		adc3histo_sw = 0;	// Reset request
		can_msg_histo31.cd.ui[0] = adc3histoctr[adc3histo_build]; // Respond with total ct.
		can_msg_ptr1 = &can_msg_histo31;

		x = adc3histo_build;	// Swap buffer indices
		adc3histo_build = adc3histo_send;
		adc3histo_send = x;

		/* Zero out buffer */
		ph3  = &adc3histo[adc3histo_build][0];
		pend3 = &adc3histo[adc3histo_build][ADCVALBUFFSIZE];
		while (ph3 < pend3)
			*ph3++ = 0;
		adc3histoctr[adc3histo_build] = 0; // Zero counter

		/* Set up for 2nd msg sending (after 1st msg is sent) */
		can_msg_ptr2 = &can_msg_histo32; // Show systick entry, above, that CAN msg1 is ready to send
		can_msg_histo_ptr = &adc3histo[adc3histo_send][0]; // Pointer into 
		// The following is the one that triggers off the sending
		can_msg_histo32.cd.ui[1] = 0; // Reset bin number for responses with histogram counts
	}

	/* Use pointers to make a fast loop. */
	unsigned short* p    = &adc3valbuff[adc3valbuffflag][0];
	unsigned short* pend = &adc3valbuff[adc3valbuffflag][ADCVALBUFFSIZE];
	ph3  = &adc3histo[adc3histo_build][0];	// ADC3 histogram
	u32 tmp;

	/* Increment histogram bins from ADC readings. */
	while (p < pend)
	{
		tmp = (*p++ >> 6) & 63;	// ADC3 into 64 ranges
		*(ph3 + tmp) += 1;	// Increment bin(n) count
	}
	/* Keep a count of total readings in the histogram. */
	adc3histoctr[adc3histo_build] += 1;
	
	return;
}
/******************************************************************************
 * void adc_histo_cansend(struct CANRCVBUF* p);
 * @brief 	: Send data in response to a CAN msg
 * @param	: p = pointer to 'struct' with CAN msg
 * @return	: ? 
*******************************************************************************/
/* Sequence:
1) The PC sends a '1 msg id that requests the total count.
   'adc?histo_sw' is set to 1.
     the next DMA interrupt sets low priority interrupt
       low priority interrupt sees 'adc?histo_sw' set, switches histo buffers, and zeroes "other" buffer,
       and sets 'adc?histo_msg_flag' to 1.
   next systick interrupt (64/sec) sees 'can_msg_ptr1' not zero and sends CAN msg1 with total count.
   next systick interrupt sees 'can_msg_ptr1' zero and if 'can_msg_histo_ptr2' and 'can_msg_ptr2' bin count not still at max
     sends a number of bin count msgs (the number defined by 'HISTO')
     when the bin count (can_msg_ptr2->ui[1]) reaches max, then it stops.

*/
void adc_histo_cansend(struct CANRCVBUF* p)
{
	/* ADC3 */
	if ((p->id & ~0x1) == can_msg_histo31.id)	// Request: switch buffers and send total bin count
	{ // Set request to switch buffers (see 'adc_histo_build_ADC3')
		adc3histo_sw = 1; // Set switch to cause 'build to switch buffers 
		return;
	}

	/* ADC2 */
	if ((p->id & ~0x1) ==  can_msg_histo21.id)	// Request: switch buffers and send total bin count
	{ // Set request to switch buffers (see 'adc_histo_build_ADC12')
		adc2histo_sw = 1; 
		return;
	}

	/* ADC3 ADC2 filtered readings command. */
	if ((p->id & ~0x1) ==  can_msg_adc_5.id)	// Request: switch buffers and send total bin count
	{ // Set request to switch buffers (see 'adc_histo_build_ADC12')
		adcreadings_ctr = 0;
		return;
	}
	return;


}

