/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : adcsensor_pod.c
* Hackeroos          : deh
* Date First Issued  : 11/11/2012
* Board              : STM32F103VxT6_pod_mm
* Description        : ADC routines for f103 sensor--pod board
*******************************************************************************/
/* 
11/11/2012 This is a hack of svn_pod/sw_stm32/trunk/devices/adcpod.c
See p214 of Ref Manual for ADC section 

ADC1 and ADC2 are used for photocell A and B, respectively, emitter measurements.  The 
protoype unit uses the POD board with an external handwired board for the photocells.
The photocell transmit diode is driven by the 3.2v analog voltage via a 330 ohm resistor.
The photocell photodetector transistor emitter has a 3.3K resistor to ground.  The 
voltage across the 3.3K resistor is measured by the ADC.

Each ADC "scans" just one channel continuously.  The high and low threshold watchdog
registers are used to trigger an interrupt.  When the converted signal value is greater than
the high threshold register (HTR), or is less than the low threshold register (LTR), the
watddog flag is set, and when the watchdog interrupt is enabled an interrupt is triggered.

Each conversion that results in a value greater than the HTR, or less than the LTR, will
set the interrupt flag.

Note that ADC1 and ADC2 have their interrupts mapped together.  For independent
interrupts ADC1 & ADC3 (or ADC2 & ADC3) would have to be used.  ADC1 and ADC2 is used, 
therefore, when there is an interrupt the status registers for both ADC1 and ADC2 have
to be checked.

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

#include "PODpinconfig.h"
//#include "libmiscstm32/systick1.h"
#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/gpio.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/adc.h"
#include "libusartstm32/commonbitband.h"
#include "common.h"

#include "adcsensor_pod.h"


/* Delay for CR2_ADON (adc power up) (about 1 us) */
#define DELAYCOUNTCR2_ADON	2	// Delay in microseconds

/* ADC usage on POD board-- ==> Those marked with '###' are the only ADC inputs used with the sensor app. <==
PA 3 ADC123-IN3	Thermistor on 32 KHz xtal
PB 0 ADC12 -IN8	Bottom cell of battery
PB 1 ADC12 -IN9	Top cell of battery 
PC 0 ADC12 -IN10	Accelerometer X	 ### Photocell emitter A (ADC1)
PC 1 ADC12 -IN11	Accelerometer Y  ### Photocell emitter B (ADC2) 
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
#define SMP1	3	// Code for number of cycles in conversion on each adc channel
#define SMP2	3
#define SMP3	3
#define SMP4	3
#define SMP5	3
#define SMP6	3
#define SMP7	3
#define SMP8	6
#define SMP9	6
#define SMP10	 1	// ### Photocell emitter A
#define SMP11	 1	// ### Photocell emitter B	
#define SMP12	3
#define SMP13	3
#define SMP14	3
#define SMP15	3
#define SMP16	7
#define SMP17	7

/* ------- LED identification ----------- 
|-number on pc board below LEDs (also port E bit number)
| color   ADCx codewheel  bit
3 green   ADC2   black    0
4 red	  ADC2   white    1
5 green   ADC1   black    0
6 yellow  ADC1   white    1
  --------------------------------------*/

/* ********** Static routines **************************************************/
static void adc_init_se(void);
static void adc_start_conversion_se(void);
static void adc_start_cal_register_reset_se(void);
static void adc_start_calibration_se(void);

/* APB2 bus frequency is needed to set ADC prescalar so that it is less than 14 MHz,
   but as high as possible. */
extern unsigned int	pclk2_freq;	// APB2 bus frequency in Hz (see 'lib/libmiscstm32/clockspecifysetup.c')

/* SYSCLK frequency is used to convert millisecond delays into SYSTICK counts */
extern unsigned int	sysclk_freq;	// SYSCLK freq in Hz (see 'lib/libmiscstm32/clockspecifysetup.c')

/* High and low watchdog register threshold values */
unsigned short usHtr1 = ADC1_HTR_INITIAL;	// ADC1 High register initial value
unsigned short usLtr1 = ADC1_LTR_INITIAL;	// ADC1 Low  register initial value
unsigned short usHtr2 = ADC2_HTR_INITIAL;	// ADC2 High register initial value
unsigned short usLtr2 = ADC2_LTR_INITIAL;	// ADC2 Low  register initial value

/* ----------------------------------------------------------------------------- 
 * void timedelay (u32 ticks);
 * ticks = processor ticks to wait, using DTW_CYCCNT (32b) tick counter
 ------------------------------------------------------------------------------- */
void timedelay (unsigned int ticks)
{
	/* Start time count */
	u32 t0 = *(volatile unsigned int *)0xE0001004; // DWT_CYCNT
	t0 += ticks;
	while (( (int)t0 ) - (int)(*(volatile unsigned int *)0xE0001004) > 0 );
	return;
}
/******************************************************************************
 * void adc_init_sequence(void);
 * @brief 	: Call this routine to do a timed sequencing of power up and calibration
*******************************************************************************/
void adc_init_sequence_se(void)
{
	u32 ticksperus = (sysclk_freq/1000000);	// Number of ticks in one microsecond

	/* Turn on 3.2v regulator */
	adc_regulator_turn_on_se();	// Turn on 3.2 v regulator.
	timedelay(20000 * ticksperus);	// Wait 20,000 us (20 ms) for voltage to stabilize

	/* Initialize ADC1 & ADC2 */
	adc_init_se();			// Initialize ADC1 and ADC2 registers.
	timedelay(50 * ticksperus);	// Wait

	/* Start the reset process for the calibration registers */
	adc_start_cal_register_reset_se();
	timedelay(10 * ticksperus);	// Wait

	/* Start the register calibration process */
	adc_start_calibration_se();
	timedelay(80 * ticksperus);	// Wait

	/* ADC is now ready for use.  Start ADC conversions */
	adc_start_conversion_se();	// Start ADC conversions and wait for ADC watchdog interrupts

	return;
}
/******************************************************************************
 * static void adc_init_se(void);
 * @brief 	: Initialize adc for dma channel 1 transfer
*******************************************************************************/
static void adc_init_se(void)
{
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

	/* PE7	3.2v Regulator EN, Analog: gpio (JIC--it should have been already been turned on along with a wait delay) */
	ANALOGREG_on				// gpio macro (see PODpinconfig.h)

	/* Enable bus clocking for ADC */
	RCC_APB2ENR |= ( (RCC_APB2ENR_ADC1EN) | (RCC_APB2ENR_ADC2EN) );	// Enable ADC1 and ADC2 clocking (see p 104)
	
	/* Scan mode (p 236) 
"This bit is set and cleared by software to enable/disable Scan mode. In Scan mode, the
inputs selected through the ADC_SQRx or ADC_JSQRx registers are converted." 
	AWDEN - enable watchdog register p 229,30. */
	//         (   scan      | watchdog enable | watchdog interrupt enable | watchdog channel number )
	ADC1_CR1 = (ADC_CR1_SCAN | ADC_CR1_AWDEN   | ADC_CR1_AWDIE | 10 );
	ADC2_CR1 = (ADC_CR1_SCAN | ADC_CR1_AWDEN   | ADC_CR1_AWDIE | 11 );	

	/*          (   Continuous   | Power ON 	) */
	ADC1_CR2  = (  ADC_CR2_CONT  | ADC_CR2_ADON	); 	// (p 240)
	ADC2_CR2  = (  ADC_CR2_CONT  | ADC_CR2_ADON	); 	// (p 240)

	/* 1 us Tstab time is required before writing a second 1 to ADON to start conversions 
	(see p 98 of datasheet) */

	// ---Original for POD---
	/* Set sample times for channels used on POD board (p 241,242) */	
//	ADC1_SMPR1 = ( (SMP17<<21) | (SMP16<<18) | (SMP13<<9) | (SMP12<<6) | (SMP11<<3) | (SMP10<<0) );
//	ADC1_SMPR2 = ( (SMP9 <<27) | (SMP8 <<24) | (SMP3 <<9) );
	
	// ---Modified for sensor---
	/* Set sample times for channels used on POD board (p 241,242) */	
	ADC1_SMPR1 = ( (SMP10 << 0) );
	ADC2_SMPR1 = ( (SMP11 << 3) );

	/* Set high and low threshold regsiters for both ADCs with initial values */
	ADC1_HTR = ADC1_HTR_INITIAL;
	ADC1_LTR = ADC1_LTR_INITIAL;
	ADC2_HTR = ADC2_HTR_INITIAL;
	ADC2_LTR = ADC2_LTR_INITIAL;

	/* Setup the number of channels scanned (p 238) */
	ADC1_SQR1 = ( ( (NUMBERADCCHANNELS_SE-1) << 20) );	// Chan count, sequences 16 - 13 (p 217, 238)
	ADC2_SQR1 = ( ( (NUMBERADCCHANNELS_SE-1) << 20) );	// Chan count, sequences 16 - 13 (p 217, 238)

	/* This maps the ADC channel number to the position in the conversion sequence */
	// Load channels IN11,IN10, for conversions sequences (p 240)
	ADC1_SQR3 = ( (10 << 0) ); // Conversion sequence ### one sensor photocell A emitter measurement
	ADC2_SQR3 = ( (11 << 0) ); // Conversion sequence ### one sensor photocell B emitter measurement

	/* Set and enable interrupt controller for ADC */
	NVICIPR (NVIC_ADC1_2_IRQ,ADC1_2_PRIORITY );	// Set interrupt priority (p 227)
	NVICISER(NVIC_ADC1_2_IRQ);			// Enable interrupt controller for ADC1|ADC2

	return;
}
/******************************************************************************
 * static void adc_start_cal_register_reset_se(void);
 * @brief 	: Start calibration register reset
 * @return	: Current 32 bit SYSTICK count
*******************************************************************************/
static void adc_start_cal_register_reset_se(void)
{
	/* Reset calibration register  */
	ADC1_CR2 |= ADC_CR2_RSTCAL;			// Turn on RSTCAL bit to start calibration register reset
	ADC2_CR2 |= ADC_CR2_RSTCAL;			// Turn on RSTCAL bit to start calibration register reset
	
	/* Wait for register to reset */
	while ((ADC1_CR2 & ADC_CR2_RSTCAL) != 0);
	while ((ADC2_CR2 & ADC_CR2_RSTCAL) != 0);

	return;
}
/******************************************************************************
 * static void adc_start_calibration_se(void);
 * @brief 	: Start calibration
*******************************************************************************/
static void adc_start_calibration_se(void)
{
	/* Start calibration  */
	ADC2_CR2 |= ADC_CR2_CAL;				// Turn on RSTCAL bit to start calibration register reset
	ADC1_CR2 |= ADC_CR2_CAL;				// Turn on RSTCAL bit to start calibration register reset

	/* Wait for calibration to complete (about 7 us */
	while ((ADC2_CR2 & ADC_CR2_CAL) != 0);
	while ((ADC1_CR2 & ADC_CR2_CAL) != 0);

	return;
}
/******************************************************************************
 * static unsigned int adc_start_conversion_se(void);
 * @brief 	: Start a conversion of the regular sequence
*******************************************************************************/
static void adc_start_conversion_se(void)
{	
/* After the initial write of this bit, subsequent writes start the ADC conversion--(p 241)
"Conversion starts when this bit holds a value of 1 and a 1 is written to it. The application."
should allow a delay of tSTAB between power up and start of conversion. Refer to Figure 27 */
	ADC1_CR2 |= ADC_CR2_ADON;		// Writing a 1 starts the conversion (see p 238)
	ADC2_CR2 |= ADC_CR2_ADON;		// Writing a 1 starts the conversion (see p 238)

	return;
}
/******************************************************************************
 * void adc_regulator_turn_on_se(void);
 * @brief 	: Turn on 3.2v regulator for ADC and get time
*******************************************************************************/
void adc_regulator_turn_on_se(void)
{
	/* PE7	3.2v Regulator EN, Analog: gpio */
	ANALOGREG_on				// gpio macro (see PODpinconfig.h)

	return;
}
/*#######################################################################################
 * ISR routine for ADC 1 & ADC2 (they are mapped together by ST to make life difficult)
 *####################################################################################### */
static unsigned short new;	// Latest 0|1 pair--bit 0 = ADC1 (photocell A); bit 1 = ADC2 (photocell B);
static unsigned short old;	// Saved "new" from last interrupt.

volatile s32 encoder_ctr;	// Shaft encoder running count (+/-)
volatile u32 encoder_error;	// Cumulative count--shaft encoding errors
volatile u32 encoder_Actr;	// Cumulative count--photocell A
volatile u32 encoder_Bctr;	// Cumulative count--photocell B
volatile u32 adc_encode_time;	// DTW_CYCCNT time of last transition
u32 adc_encode_time_prev;	// Previous time at SYSTICK
s32 encoder_ctr_prev;		// Previous encoder running count

/* Table lookup for shaft encoding count: -1 = negative direction count, +1 = positive direction count, 0 = error */
/* One sequence of 00 01 11 10 results in four counts. */
static const signed char en_state[16] = {0,+1,-1, 0,-1, 0, 0,+1,+1, 0, 0,-1, 0,-1,+1, 0};

void ADC1_2_IRQHandler(void)
{ // Here, interrupt of ADC1 (Photocell A) or ADC1 (Photocell B)
	int temp;	// Register readback to assure a prior write has been taken by the register
	int both;	// Table lookup index = ((old << 2) | new)

	/* Save time of shaft encoding transitions (for speed computation) */
	adc_encode_time = *(volatile unsigned int *)0xE0001004; // DWT_CYCNT

	/* ADC1, IN10, Photocell A  */
	if ((ADC1_SR & ADC_SR_AWD) != 0)	// ADC1 watchdog interrupt?
/* Note: bit banding the above 'if' makes not a difference in the number of instructions compiled. */
	{ // Here, yes.  Watchdog flag is on
		/* Is the data *above* the high threshold register? (p 219) */
		if (ADC1_DR > ADC1_HTR)
		{ // Here, watchdog interrupt was the signal going upwards
			ADC1_HTR = 0x0fff;	// Change high register to max and impossible to exceed
			ADC1_LTR = usLtr1;	// Bring low register up to threshold
			new |= 0x01;		// Turn 0|1 status bit ON
			GPIO_BRR(GPIOE)  = (1<<6);	// Set Yellow LED 
			GPIO_BSRR(GPIOE) = (1<<5);	// Reset Green LED

		}
		else
		{ // Here, the watchdog interrupt was the signal going downwards
			ADC1_LTR = 0;		// Set low register to impossible
			ADC1_HTR = usHtr1;	// Set high register to threshold
			new &= ~0x01;		// Turn 0|1 status bit OFF
			GPIO_BRR(GPIOE)  = (1<<5);	// Set Green LED 
			GPIO_BSRR(GPIOE) = (1<<6);	// Reset Yellow LED
		}
		encoder_Actr += 1;		// Cumulative counter--either direction
		both = (old << 2) | new;	// Make lookup index
		encoder_ctr += en_state[both];	// Add +1, -1, or 0
		if (en_state[both] == 0)	// Was this an error
			encoder_error += 1;	// Cumulative count
		old = new;			// Update for next
		temp = ADC1_HTR | ADC1_LTR;	// Be sure new threshold values set before turning off interrupt flag
		ADC1_SR = ~0x1;			// Reset adc watchdog interrupt flag
		temp = ADC1_SR;			// Readback assures interrupt flag is off (prevent tail-chaining)
	}

	/* ADC2, IN11, Photocell B  */	
	if ((ADC2_SR & ADC_SR_AWD) != 0)	// ADC2 watchdog interrupt?
	{ // Here, yes.  Watchdog flag is on
		/* Is the data *above* the high threshold register? (p 219) */
		if (ADC2_DR > ADC2_HTR)
		{ // Here, watchdog interrupt was the signal going upwards
			ADC2_HTR = 0x0fff;	// Change high register to max and impossible to exceed
			ADC2_LTR = usLtr2;	// Bring low register up to threshold
			new |= 0x02;		// Turn 0|1 status bit ON
			GPIO_BRR(GPIOE)  = (1<<4);	// Set Green LED 
			GPIO_BSRR(GPIOE) = (1<<3);	// Reset Red LED
		}
		else
		{ // Here, the watchdog interrupt was the signal going downwards
			ADC2_LTR = 0;		// Set low register to impossible
			ADC2_HTR = usHtr2;	// Set high register to threshold
			new &= ~0x02;		// Turn 0|1 status bit OFF
			GPIO_BRR(GPIOE)  = (1<<3);	// Reset Red LED
			GPIO_BSRR(GPIOE) = (1<<4);	// Set Green LED
		}
		encoder_Bctr += 1;		// Cumulative counter--either direction
		both = (old << 2) | new;	// Make lookup index
		encoder_ctr += en_state[both];	// Add +1, -1, or 0
		if (en_state[both] == 0)	// Was this an error
			encoder_error += 1;	// Cumulative count
		old = new;			// Update for next 
		temp = ADC2_HTR | ADC2_LTR;	// Be sure new threshold values set before turning off interrupt flag
		ADC2_SR = ~0x1;			// Reset adc watchdog flag
		temp = ADC2_SR;			// Readback assures interrupt flag is off (prevent tail-chaining)
	}
	return;
}

