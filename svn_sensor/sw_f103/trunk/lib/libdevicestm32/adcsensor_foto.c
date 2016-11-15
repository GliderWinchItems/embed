/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : adcsensor_foto.h
* Author             : deh
* Date First Issued  : 07/08/2013
* Board              : RxT6
* Description        : ADC routines for f103 sensor--sensor board
*******************************************************************************/
/*
07-13-2013: earlier rev 206 works with 64/sec compute rate.  Major changes started
    to cic filter rpm, with cic input at a higher than 64 rate.  Works except for
    some glitch when the rpm hovers around the no encoder counts to one count within
    one 1/64th sec frame.
*/
/* 
07/08/2013 This is a hack of svn_pod/sw_stm32/trunk/devices/adcsensor_pod.c
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

#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/gpio.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/adc.h"
#include "libopenstm32/dma.h"
#include "libusartstm32/commonbitband.h"
#include "libmiscstm32/clockspecifysetup.h"

#include "common.h"
#include "common_can.h"
#include "common_misc.h"

#include "pinconfig_all.h"
#include "adcsensor_foto.h"
#include "canwinch_pod_common_systick2048.h"
#include "SENSORpinconfig.h"

#include "libopenstm32/scb.h"
#include "libopenstm32/systick.h"

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
#define SMP3	3
#define SMP4	3
#define SMP5	3
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
static void compute_init(u32 iamunitnumber);
static void savereadings(void);
static void compute_doit(void);
static s32 compute_speed(u32 ctrZ, u32 encode_timeZ, u32 ticktimeZ);

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
unsigned short usHtr1 = ADC1_HTR_INITIAL;	// ADC1 High register initial value
unsigned short usLtr1 = ADC1_LTR_INITIAL;	// ADC1 Low  register initial value
unsigned short usHtr2 = ADC2_HTR_INITIAL;	// ADC2 High register initial value
unsigned short usLtr2 = ADC2_LTR_INITIAL;	// ADC2 Low  register initial value

/******************************************************************************
 * static int adv_index(int idx, int size)
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
 * void adc_init_sequence_foto(u32 iamunitnumber);
 * @brief 	: Call this routine to do a timed sequencing of power up and calibration
 * @param	: iamunitnumber = CAN unit id for this unit, see 'common_can.h'
*******************************************************************************/
void adc_init_sequence_foto(u32 iamunitnumber)
{
	u32 ticksperus = (sysclk_freq/1000000);	// Number of ticks in one microsecond

	/* Set up for computations and CAN messaging shaft count and speed */
	compute_init(iamunitnumber);

	/* Initialize ADC1 & ADC2 */
	adc_init_foto();			// Initialize ADC1 and ADC2 registers.
	timedelay(50 * ticksperus);	// Wait

	/* Start the reset process for the calibration registers */
	adc_start_cal_register_reset_foto();
	timedelay(10 * ticksperus);	// Wait

	/* Start the register calibration process */
	adc_start_calibration_foto();
	timedelay(80 * ticksperus);	// Wait

	/* ADC is now ready for use.  Start ADC conversions */
	adc_start_conversion_foto();	// Start ADC conversions and wait for ADC watchdog interrupts

	return;
}
/******************************************************************************
 * static void adc_init_foto(void);
 * @brief 	: Initialize adc for dma channel 1 transfer
*******************************************************************************/
const struct PINCONFIGALL pin_pa0 = {(volatile u32 *)GPIOA, 0, IN_ANALOG, 0};
const struct PINCONFIGALL pin_pa1 = {(volatile u32 *)GPIOA, 1, IN_ANALOG, 0};

static void adc_init_foto(void)
{
	/*  Setup ADC pins for ANALOG INPUT (p 148) */
	pinconfig_all( (struct PINCONFIGALL *)&pin_pa0);
	pinconfig_all( (struct PINCONFIGALL *)&pin_pa1);

	/* Find prescalar divider code for the highest permitted ADC freq (which is 14 MHz) */
	unsigned char ucPrescalar = 3;		// Division by 8	
	if (pclk2_freq/8 < 14000000) ucPrescalar = 2;	// Division by 6
	if (pclk2_freq/6 < 14000000) ucPrescalar = 1;	// Division by 4
	if (pclk2_freq/4 < 14000000) ucPrescalar = 0;	// Division by 2

//ucPrescalar = 3;	// Slow it down for debugging and test

	/* Set APB2 bus divider for ADC clock */
	RCC_CFGR |= ( (ucPrescalar & 0x3) << 14)	; // Set code for bus division (p 92)

	/* Enable bus clocking for ADC */
	RCC_APB2ENR |= ( (RCC_APB2ENR_ADC1EN) | (RCC_APB2ENR_ADC2EN) );	// Enable ADC1 and ADC2 clocking (see p 104)
	
	/* Scan mode (p 236) 
"This bit is set and cleared by software to enable/disable Scan mode. In Scan mode, the
inputs selected through the ADC_SQRx or ADC_JSQRx registers are converted." 
	AWDEN - enable watchdog register p 229,30. */
	//         (   scan      | watchdog enable | watchdog interrupt enable | watchdog channel number )
	ADC1_CR1 = (ADC_CR1_SCAN | ADC_CR1_AWDEN   | ADC_CR1_AWDIE | 0 );	// ADC1 chan 0
	ADC2_CR1 = (ADC_CR1_SCAN | ADC_CR1_AWDEN   | ADC_CR1_AWDIE | 1 );	// ADC2 chan 1

	/*          (   Continuous   | Power ON 	) */
	ADC1_CR2  = (  ADC_CR2_CONT  | ADC_CR2_ADON	); 	// (p 240)
	ADC2_CR2  = (  ADC_CR2_CONT  | ADC_CR2_ADON	); 	// (p 240)

	/* 1 us Tstab time is required before writing a second 1 to ADON to start conversions 
	(see p 98 of datasheet) */

	// ---Modified for sensor---
	/* Set sample times for channels used on POD board (p 236) */	
	ADC1_SMPR2 = ( (SMP1 << 0) );
	ADC2_SMPR2 = ( (SMP2 << 3) );

	/* Set high and low threshold registers for both ADCs with initial values */
	ADC1_HTR = ADC1_HTR_INITIAL;
	ADC1_LTR = ADC1_LTR_INITIAL;
	ADC2_HTR = ADC2_HTR_INITIAL;
	ADC2_LTR = ADC2_LTR_INITIAL;

	/* Setup the number of channels scanned (zero = just one) (p 238) */
	ADC1_SQR1 = ( ( (NUMBERADCCHANNELS_FOTO-1) << 20) );	// Chan count, sequences 16 - 13 (p 217, 238)
	ADC2_SQR1 = ( ( (NUMBERADCCHANNELS_FOTO-1) << 20) );	// Chan count, sequences 16 - 13 (p 217, 238)

	/* This maps the ADC channel number to the position in the conversion sequence */
	// Load channels IN11,IN10, for conversions sequences (p 240)
	ADC1_SQR3 = ( (0 << 0) ); // Conversion sequence number one sensor photocell A emitter measurement
	ADC2_SQR3 = ( (1 << 0) ); // Conversion sequence number one sensor photocell B emitter measurement

	/* Set and enable interrupt controller for ADC */
	NVICIPR (NVIC_ADC1_2_IRQ,ADC1_2_PRIORITY_FOTO );// Set interrupt priority (p 227)
	NVICISER(NVIC_ADC1_2_IRQ);			// Enable interrupt controller for ADC1|ADC2

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
	ADC1_CR2 |= ADC_CR2_RSTCAL;			// Turn on RSTCAL bit to start calibration register reset
	ADC2_CR2 |= ADC_CR2_RSTCAL;			// Turn on RSTCAL bit to start calibration register reset
	
	/* Wait for register to reset */
	while ((ADC1_CR2 & ADC_CR2_RSTCAL) != 0);
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
	ADC1_CR2 |= ADC_CR2_CAL;				// Turn on RSTCAL bit to start calibration register reset

	/* Wait for calibration to complete (about 7 us */
	while ((ADC2_CR2 & ADC_CR2_CAL) != 0);
	while ((ADC1_CR2 & ADC_CR2_CAL) != 0);

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
	ADC1_CR2 |= ADC_CR2_ADON;		// Writing a 1 starts the conversion (see p 238)
	ADC2_CR2 |= ADC_CR2_ADON;		// Writing a 1 starts the conversion (see p 238)

	return;
}
/*#######################################################################################
 * ISR routine for ADC 1 & ADC2 (they are mapped together by ST to make life difficult)
 *####################################################################################### */
static unsigned short new;	// Latest 0|1 pair--bit 0 = ADC1 (photocell A); bit 1 = ADC2 (photocell B);
static unsigned short old;	// Saved "new" from last interrupt.

/* Values generated in interrupt routine */
s32 encoder_ctr;	// Shaft encoder running count (+/-)
u32 encoder_Actr;	// Cumulative count--photocell A
u32 encoder_Bctr;	// Cumulative count--photocell B
u32 adc_encode_time;	// DTW_CYCCNT time of last transition


u32 adc_encode_time_prev;	// Previous time at SYSTICK
s32 encoder_ctr_prev;		// Previous encoder running count

static unsigned int speedzeroflg = 0;	// not-zero = wait for a encoder tick


/* Table lookup for shaft encoding count: -1 = negative direction count, +1 = positive direction count, 0 = error */
/* One sequence of 00 01 11 10 results in four counts. */
static const signed char en_state[16] = {0,+1,-1, 0,-1, 0, 0,+1,+1, 0, 0,-1, 0,-1,+1, 0};

void ADC1_2_IRQHandler(void)
{ // Here, interrupt of ADC1 (Photocell A) or ADC1 (Photocell B)
     volatile int temp;	// Register readback to assure a prior write has been taken by the register
	int both;	// Table lookup index = ((old << 2) | new)

	/* ADC1, IN0, Photocell A  */
	if ((ADC1_SR & ADC_SR_AWD) != 0)	// ADC1 watchdog interrupt?
/* Note: bit banding the above 'if' makes no difference in the number of instructions compiled. */
	{ // Here, yes.  Watchdog flag is on
		/* Is the data *above* the high threshold register? (p 219) */
		if (ADC1_DR > ADC1_HTR)
		{ // Here, watchdog interrupt was the signal going upwards
			ADC1_HTR = 0x0fff;	// Change high register to max and impossible to exceed
			ADC1_LTR = usLtr1;	// Bring low register up to threshold
			new |= 0x01;		// Turn 0|1 status bit ON
			LED20RED_off;
		}
		else
		{ // Here, the watchdog interrupt was the signal going downwards
			ADC1_LTR = 0;		// Set low register to impossible
			ADC1_HTR = usHtr1;	// Set high register to threshold
			new &= ~0x01;		// Turn 0|1 status bit OFF
			LED20RED_on;
		}
		encoder_Actr += 1;		// Cumulative counter--either direction
		both = (old << 2) | new;	// Make lookup index
		encoder_ctr += en_state[both];	// Add +1, -1, or 0
		if (en_state[both] == 0)	// Was this an error
			adcsensor_foto_err[0] += 1;	// Cumulative count
		/* Save time of shaft encoding transitions (for speed computation) */
		adc_encode_time = *(volatile unsigned int *)0xE0001004; // DWT_CYCNT
		old = new;			// Update for next
		temp = ADC1_HTR | ADC1_LTR;	// Be sure new threshold values set before turning off interrupt flag
		ADC1_SR = ~0x1;			// Reset adc watchdog interrupt flag
		temp = ADC1_SR;			// Readback assures interrupt flag is off (prevent tail-chaining)
	}

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
		encoder_Bctr += 1;		// Cumulative counter--either direction
		both = (old << 2) | new;	// Make lookup index
		encoder_ctr += en_state[both];	// Add +1, -1, or 0
		if (en_state[both] == 0)	// Was this an error
			adcsensor_foto_err[0] += 1;	// Cumulative count
		/* Save time of shaft encoding transitions (for speed computation) */
		adc_encode_time = *(volatile unsigned int *)0xE0001004; // DWT_CYCNT
		old = new;			// Update for next 
		temp = ADC2_HTR | ADC2_LTR;	// Be sure new threshold values set before turning off interrupt flag
		ADC2_SR = ~0x1;			// Reset adc watchdog flag
		temp = ADC2_SR;			// Readback assures interrupt flag is off (prevent tail-chaining)
	}
	return;
}
/* $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
         Below deals with the handling of the encoding done in the foregoing routines
   $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ */
/******************************************************************************
 * static void compute_init(u32 iamunitnumber);
 * @brief 	: Initialize for computations & sending CAN msg
 * @param	: iamunitnumber = CAN unit id for this unit, see 'common_can.h'
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
long	speed_filteredA;	// Most recent computed & filtered rpm
u32 	encoder_ctrA;		// Most recent encoder count

static struct CICLN2M3 rpm_cic;		// Double buffer CIC intermediate storage
static struct CANRCVBUF can_msg_DS;	// CAN msg. Drive Shaft--odometer|speed
static struct CANRCVBUF can_msg_ER1;	// CAN msg. Errors
static struct CANRCVBUF can_msg_ER2;	// CAN msg. Errors

/* Pointers to functions to be executed under a low priority interrupt */
// These hold the address of the function that will be called
void 	(*systickHIpriority2_ptr)(void) = 0;	// SYSTICK handler (very high priority) continuation
void 	(*systickLOpriority2X_ptr)(void) = 0;	// SYSTICK handler (low high priority) continuation--1/64th

/* When the 1/64th sec tick time in the CAN time sync message matches our unit ID we can send an error message. */
static u32 iamunitnumbershifted;	// Save for comparison to 1/64th tick for sending error messages

static void compute_init(u32 iamunitnumber)
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

	/* CAN message id's.  Two readings in each message. */
	can_msg_DS.id  = iamunitnumber | (CAN_DATAID_SEN_COUNTERandSPEED << CAN_DATAID_SHIFT);  // CAN msg: count & rpm
	can_msg_ER1.id = iamunitnumber | (CAN_DATAID_ERROR1              << CAN_DATAID_SHIFT);	// CAN msg. Errors
	can_msg_ER2.id = iamunitnumber | (CAN_DATAID_ERROR2              << CAN_DATAID_SHIFT);	// CAN msg. Errors

//can_msg_DS.id  = 0x30400000;  	// CAN msg: count & rpm
//can_msg_ER1.id = 0xD0400008;	// CAN msg. Errors
//can_msg_ER2.id = 0xD0400006;	// CAN msg. Errors


	/* Right justify the unit ID number (from left justification) */
	iamunitnumbershifted = iamunitnumber >> CAN_UNITID_SHIFT;

	/* This is a low priority post SYSTICK interrupt call. */
	systickLOpriority_ptr = &compute_doit;

	/* This is a very high priority post SYSTICK interrupt call. */
	systickHIpriority_ptr = &savereadings;

	return;
}
/* ######################### UNDER HIGH PRIORITY SYSTICK INTERRUPT ############################### 
 * Enter from SYSTICK interrupt each 64 per sec, and buffer latest readings
 * ############################################################################################### */
/* Circular buffer saved data */
#define RPMDATABUFFSIZE	8
static int idxsaveh = 0;	// Index for storing new data
static int idxsavel = 0;	// Index for retrieving data for 'compute_doit'
static u32 encoder_ctrZ[RPMDATABUFFSIZE];
static u32 adc_encode_timeZ[RPMDATABUFFSIZE];
static u32 systicktimeZ[RPMDATABUFFSIZE];
static u32 stk_32ctrZ[RPMDATABUFFSIZE];

static void savereadings(void)
{
	volatile u32 temp;

	if ( ((stk_32ctr & 0x1f) & ( (32/DECIMATION_FOTO)-1) ) != ((32/DECIMATION_FOTO)-1) ) return;

	/* Disable interrupts that might change readings while we copy them */
	ADC1_CR1 &= ~ADC_CR1_AWDIE;	// ADC1 chan 0
	ADC2_CR1 &= ~ADC_CR1_AWDIE;	// ADC2 chan 1
	temp = ADC1_CR1;	// Readback to assure disabled
	temp = ADC2_CR2;	// Readback to assure disabled

	/* Save most recent values */
	encoder_ctrZ[idxsaveh]     = encoder_ctr;		// Shaft encoder running count (+/-)
	adc_encode_timeZ[idxsaveh] = adc_encode_time;	// DTW_CYCCNT time of last transition

	/* This "read time" must follow the above so that it is not possible to have a ADC interrupt
           store the time after the SYSTICK interrupt, which would then cause a small negative
           difference in the times */
	systicktimeZ[idxsaveh] = *(volatile unsigned int *)0xE0001004; // DWT_CYCNT;
	stk_32ctrZ[idxsaveh] = (stk_32ctr & 0x1f); // Save low order bits: 0 - 31 for sub-tick within 1/64th sec
 
	/* Re-enable interrupts */
	ADC1_CR1 |= ADC_CR1_AWDIE;	// ADC1 chan 0
	ADC2_CR1 |= ADC_CR1_AWDIE;	// ADC2 chan 1

	/* Advance index for high priority storing. */
	idxsaveh = adv_index(idxsaveh, RPMDATABUFFSIZE);

	/* Call other routines if an address is set up--64 per sec. */
	if (systickHIpriority2_ptr != 0)	// Having no address for the following is bad.
		(*systickHIpriority2_ptr)();	// Go do something

	return;
}
/* ########################## UNDER LOW PRIORITY SYSTICK INTERRUPT ############################### 
 * Do the computations and send the CAN msg
 * ############################################################################################### */
/* 
   This routine is entered from the SYSTICK interrupt handler triggering I2C2_EV low priority interrupt. 
*/
static void compute_doit(void)
{
	s32 	speed_filtered = 0;
	struct CANRCVTIMBUF* pcanrcvtim;

	/* Handle all buffered sets of data.  Normally one, but could be more. */
	while (idxsaveh != idxsavel)
	{
		/* Compute speed and load into filter struct. */
		rpm_cic.nIn = compute_speed(encoder_ctrZ[idxsavel],adc_encode_timeZ[idxsavel],systicktimeZ[idxsavel]);

		/* Run filter on the value just loaded into the struct. */
		if ( cic_filter_l_N2_M3 (&rpm_cic) != 0) // Filtering complete?
		{ // Here, yes.
			speed_filtered = rpm_cic.lout >> CICSCALE_FOTO; // Scale and save
			speed_filteredA = speed_filtered;	// Save for mainline monitoring
		}
	
		/* stk_32ctr == 31 on the last interrupt of the 32 comprising 1/64th sec.  It marks the 
        	   end and beginning point between two 1/64th sec intervals. */
		if (stk_32ctrZ[idxsavel] == 31)	
		{ // Here, yes.  Time to setup for sending

			/* ========= Here is where adc readings get setup for sending ============ */
			/* Setup counter and speed readings for sending */
			canmsg_send(&can_msg_DS, encoder_ctrZ[idxsavel], speed_filtered);

			encoder_ctrA = encoder_ctrZ[idxsavel];	// Save for mainline monitoring
			
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
			pcanrcvtim = canrcvtim_get_sys();
			if (pcanrcvtim != 0) // If we have a time msg, display the 1/64th ticks
			{
				if ( (pcanrcvtim->R.cd.ui[0] & 0x3f) == iamunitnumbershifted)
					canmsg_send(&can_msg_ER1, adcsensor_foto_err[0],adcsensor_foto_err[1]);
				if ( (pcanrcvtim->R.cd.ui[0] & 0x3f) == (iamunitnumbershifted << 1))
					canmsg_send(&can_msg_ER2, adcsensor_foto_err[2],adcsensor_foto_err[3]);
			}

			/* Call other routines if an address is set up. */
			if (systickLOpriority2X_ptr != 0)	// Having no address for the following is bad.
				(*systickLOpriority2X_ptr)();	// Go do something
		}
		/* Advance index for low priority processing. */
		idxsavel = adv_index(idxsavel, RPMDATABUFFSIZE);
	}
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

