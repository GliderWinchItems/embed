/******************************************************************************
* File Name          : adc_mc.c
* Date First Issued  : 12/22/2013
* Board              : Discovery F4
* Description        : ADC routines for Master Controller
*******************************************************************************/
/* 
This routine handles the ADC for--

11/11/2012 This is a hack of svn_pod/sw_stm32/trunk/devices/adcpod.c
See p214 of Ref Manual for ADC section 

12/21/2013 This is a hack of svn_sensor/sw_103/trunk/lib/libdevicestm32/adcsensor_eng.[ch]

01/18/2014 Changed ADC to PC1,2,4

08/12/2014 Added Vrefint and internal temp

*/
/*
NOTE: Some page number refer to the Ref Manual RM0008, rev 11 and some more recent ones, rev 14.

Strategy--
Three ADC pins are read at a rapid rate with the DMA storing the sequence.  The DMA wraps around
at the end of the buffer.  At the end and half-way points in the buffer the DMA interrupt triggers
a low priority interrupt and the readings are entered into a CIC filter.  When the CIC filter 
complete a decimation the result if placed a buffer that holds the latest, filtered, decimated,
ADC reading for each pin where 'main' can use it.

Note: the output readings are not re-scaled.


To change the number of ADC channels--
  In adc_mc.h, change:
     NUMBERADCCHANNELS_MC
  In 'static int adc_mc_init(void)' add pins and channels as shown in the following--

To change ADC pins--
  In 'static int adc_mc_init(void)' the following are affected--
     Pin configuration:
	f4gpiopins_Config ((volatile u32*)GPIOA, 3, (struct PINCONFIG*)&inputadc);
     Sample times for the channel:
	ADC1_SMPR2 = ( (SMP3 << 9) | (SMP2 << 6) | (SMP1 << 0) );
     Channel versus conversion sequence:
	ADC1_SQR3 = (0 << 0) | (2 << 5) | (3 << 10);


 
*/

#include "libopencm3/stm32/f4/gpio.h"
#include "libopencm3/stm32/f4/rcc.h"
#include "libopencm3/stm32/f4/adc.h"
#include "libopencm3/stm32/f4/dma_common_f24.h"
#include "libopencm3/cm3/common.h"
#include "common_can.h"
#include "adc_mc.h"
#include "nvicdirect.h" 
#include "nvic_dma_mgr.h"
#include "DISCpinconfig.h"
#include "libopencm3/stm32/nvic.h"
#include "adc_mc.h"
#include "cic_filter_l_N2_M3.h"

/* For timer triggered initiation of ADC conversion sequence
   uncomment the following line. */
//#define TIMERTIGGERED
#ifdef TIMERTIGGERED
#include "countdowntimer.h"
#endif

/* ADC usage

PA0 ADC123-IN0	xxxxx
PA1 ADC123-IN2  xxxxx
PA3 ADC123-IN3	xxxxx  

Revision for MC
PC1 ADC123_IN11 Control Lever
PC2 ADC123_IN12
PC4 ADC12 -IN14

Vrefint IN16
Temp	IN17

*/
/*
Bits 29:0 SMPx[2:0]: Channel x Sample time selection for less than 1/4 LSB of 12 bit conversion.
           These bits are written by software to select the sample time individually for each channel.
           During sample cycles channel selection bits must remain unchanged.
           000:   3 cycles	@30MHz  0.4K max input R (p 123 datasheet)
           001:  15 cycles	@30MHz  5.9K max input R 
           010:  28 cycles	@30MHz 11.4K max input R
           011:  56 cycles	@30MHz 25.2K max input R
           100:  84 cycles	@30MHz 37.2K max input R
           101: 112 cycles	@30MHz 50.0K max input R
           110: 144 cycles	50K max
           111: 480 cycles	50K max

With system clock of 168 MHz and p2 clock of 84 MHz, the ADC clock
will be 84/4 = 21 MHz (max freq must be less than 30 MHz).
The total conversion time--
12 + SMPx time
At 0.047 us per adc clock cycle 
(1) xxxxx: 		12 + 480 = 492; * (1/21)us =   23.42857 us per conversion
(2) xxxxx:	 	12 + 480 = 492; * (1/21)us =   23.42857 us per conversion
(3) xxxxx:		12 + 480 = 492; * (1/21)us =   23.42857 us per conversion
(4) xxxxx:		12 + 480 = 492; * (1/21)us =   23.42857 us per conversion
(5) xxxxx:		12 + 480 = 492; * (1/21)us =   23.42857 us per conversion
Total for sequence of three ...................... =  117.143 us per sequence of three
For 16 sequences per DMA interrupt................ = 1874.286 us per buffer (or 533.536 buffers per second)

*/
// *CODE* for number of cycles in conversion on each adc channel
#define SMP11	7	// PC1
#define SMP12	7	// PC2
#define SMP14	7	// PC4
#define SMP16	7	// Vrefint
#define SMP17	7	// Temp internal

/* ********** External for timer ***********************************************/
// This holds the address of the function that will be called from countdowntimers.c.
// This is set in the 'init' below to point to where the ADC conversion is triggered.
extern void 	(*timer_sw_ptr)(void);	// Function pointer for CH2 timer interrupt

/* ********** Chain of timer call **********************************************/
// This is the pointer to call the next routine after the ADC trigger is called by
// the timer routine.
void 	(*adc_mc_ptr)(void);

/* ********** Static routines **************************************************/
static int adc_mc_init(void);


/* CIC routines for adc filtering are near the end */
static void adc_cic_filtering(void);
static void adc_cic_init(void);

/* ADC storing is done via DMA which interrupts at end and 1/2 buffer points. */
void DMA2_MC_IRQHandler(volatile u32* p);

/* APB2 bus frequency is needed to be able to set ADC prescalar so that it is less than 30 MHz, 
   but as high as possible. */
extern unsigned int	pclk2_freq;	// APB2 bus frequency in Hz (see 'lib/libmiscstm32/clockspecifysetup.c')

/* SYSCLK frequency is used to convert millisecond delays into SYSTICK counts */
extern unsigned int	sysclk_freq;	// SYSCLK freq in Hz (see 'lib/libmiscstm32/clockspecifysetup.c')

/* Double buffering of a series of ADC channel conversion sequences */
static struct ADCDR_ENG  strADC1dr;		// Double buffer array of ints for ADC readings, plus count and index

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
 * int adc_dma_init_sequence(void);
 * @brief 	: Call this routine to do a timed sequencing of power up and calibration
*******************************************************************************/
#define ADC_DMA_STREAM	0	// ADC1 is on dma2 stream 0
#define ADC_DMA_CHANNEL	0	// ADC1 is on dma2 channel 0
#define ADC_DMA_IRQ_NUM	DMASTRM20	// NVIC IRQ number
#define ADC_DMA_PRIORITY 0x80	// DMA interrupt priority
#define ADC_FSMC_PRIORITY 0xc0	// Low level interrupt for adc filtering

int adc_dma_init_sequence(void)
{
	int tmp;

	/* Set dma stream interrupt to re-vector to this routine, and check if dma sreams is already in use. */
	//                                              local dma IRQ handler, pointer passed to us,   irq number,  dma stream number
	tmp = nvic_dma_stream_vector_add( (void(*)(u32*))&DMA2_MC_IRQHandler, 0, ADC_DMA_IRQ_NUM, ADC_DMA_STREAM);
	if (tmp != 0) return (-30 + tmp); // Return a negative error code.

	/* --------- Set up the DMA channel ------------------------------------------------------------------------ */

	/* Set DMA peripheral address for ADC1 */
	DMA_SPAR(DMA2_BASE,ADC_DMA_STREAM) = &ADC1_DR;

	/* Set DMA memory address with buffer base address (stays forever with circular buffer option) */
	DMA_SM0AR(DMA2_BASE,ADC_DMA_STREAM) = (u32*)&strADC1dr.in[0][0][0];		// Memory address of first buffer array for storing data

	/* DMA stream configuration register--p 325 */
	//                                            Channel number  | Mem 32b   |  Per 16b  |  MINC   |  CIRC  | Per->Mem |  TCIE  |  HTIE  | priority
	DMA_SCR(DMA2_BASE, ADC_DMA_STREAM) = ( (ADC_DMA_CHANNEL<< 25) | (0x1<<13) | (0x1<<11) | (1<<10) | (1<<8) | (0x0<<6) | (1<<4) | (1<<3) | (0x1<<16) );

	/* Set the number of bytes in the buff */
	DMA_SNDTR(DMA2_BASE,ADC_DMA_STREAM) = (2 * NUMBERSEQUENCES * NUMBERADCCHANNELS_MC);// Number of data items before wrap-around

	/* Set low level interrupt handler pointer for filtering adc readings */
	dma_ll_ptr = &adc_cic_filtering;

	/* Low level interrupt for doing adc filtering */
	NVICIPR (NVIC_FSMC_IRQ, ADC_FSMC_PRIORITY);	// Set dma interrupt priority
	NVICISER(NVIC_FSMC_IRQ);			// Enable dma interrupt

	/* DMA for TX interrupts */
	NVICIPR (ADC_DMA_IRQ_NUM, ADC_DMA_PRIORITY);	// Set dma interrupt priority
	NVICISER(ADC_DMA_IRQ_NUM);			// Enable dma interrupt

	/* Final enabling of DMA */
	DMA_SCR(DMA2_BASE, ADC_DMA_STREAM) |= 0x1;	// Enable dma stream

	return 0;
}
/******************************************************************************
 * int adc_mc_init_sequence(void);
 * @brief 	: Call this routine to do a timed sequencing of power up and calibration
 * @param	: iamunitnumber = CAN unit id for this unit, see 'common_can.h'
*******************************************************************************/
int adc_mc_init_sequence(void)
{
	int tmp;

	/* Setup for lower level filtering of ADC readings buffered by DMA. */
	adc_cic_init();

	u32 ticksperus = (sysclk_freq/1000000);	// Number of ticks in one microsecond

	/* Initialize ADC1 */
	tmp = adc_mc_init();		// Initialize ADC1 registers.
	if (tmp < 0) return tmp;

	timedelay(50 * ticksperus);	// Wait

	/* ADC is now ready for use.  Start ADC conversions */
	strADC1dr.flg = 1;		// We start filling buffer 0, so show previous buffer
	ADC1_CR2 |= ADC_CR2_SWSTART;	// Starts the conversion
	return 0;
}
/* ######################################################################################
 * Trigger another ADC sequence
 * Enter from countdowntimers.c, CH2 interrupt
 * ###################################################################################### */
unsigned int triggerct =0;
void adc_trigger(void)
{
	ADC1_CR2 |= ADC_CR2_SWSTART;	// Starts the conversion
triggerct += 1;
	if (adc_mc_ptr != 0)	// Do something?
		(*adc_mc_ptr)();	// Call a function.
	return;
}
/******************************************************************************
 struct's for configuring CAN pins
*******************************************************************************/
//  ADC pin configuration  */
const struct PINCONFIG	inputadc = { \
	GPIO_MODE_ANALOG,	// mode: Analog 
	0,	 		// output type: not applicable 		
	0, 			// speed: not applicable
	0, 			// pull up/down: pullup 
	0 };			// AFRLy & AFRHy selection
/******************************************************************************
 * static int adc_mc_init(void);
 * @brief 	: Initialize adc for dma channel 1 transfer
*******************************************************************************/
unsigned int adcdebug1;

static int adc_mc_init(void)
{
	int tmp;
	u32 ticksperus = (sysclk_freq/1000000);	// Number of ticks in one microsecond

	/*  Setup ADC pins for ANALOG INPUT */
	f4gpiopins_Config ((volatile u32*)GPIOC, 1, (struct PINCONFIG*)&inputadc);
	f4gpiopins_Config ((volatile u32*)GPIOC, 2, (struct PINCONFIG*)&inputadc);
	f4gpiopins_Config ((volatile u32*)GPIOC, 4, (struct PINCONFIG*)&inputadc);

	/* Find prescalar divider code for the highest permitted ADC freq (which is 30 MHz) (typical) */
//	unsigned char ucPrescalar = 0;			// Division by 2	
//	if ((pclk2_freq/8) < 30000000) ucPrescalar = 3;	// Division by 8
//	if ((pclk2_freq/6) < 30000000) ucPrescalar = 2;	// Division by 6
//	if ((pclk2_freq/4) < 30000000) ucPrescalar = 1;	// Division by 4

	/* Enable bus clocking for ADC */
	RCC_APB2ENR |= (1<<8);	// Enable ADC1 clocking (see p 104)

	/* Set APB2 bus divider for ADC clock */
	/* ADC Common Control Register */
//	ADC_CCR |= (ucPrescalar << 16);	// Prescalar setting. All other settings default.
	// Use for prescalar lower than highest permitted 
	ADC_CCR |= (0x3 << 16);		
	
	/* Scan mode */
	//         (   scan      | watchdog enable | watchdog interrupt enable | watchdog channel number )
	ADC1_CR1 = (ADC_CR1_SCAN );

	/*           DMA selection|      use DMA   |  Continuous   | Power ON 	*/
#ifndef TIMERTIGGERED
	ADC1_CR2  = (ADC_CR2_DDS) | (  ADC_CR2_DMA | ADC_CR2_CONT  | ADC_CR2_ADON ); // Continuous ADC sequence scan
#else
	ADC1_CR2  = (ADC_CR2_DDS) | (  ADC_CR2_DMA |                 ADC_CR2_ADON ); // One-time (trigger by timer)
#endif
	ADC_CCR |= (1 << 23) ; // Bit 23 TSVREFE: Temperature sensor and VREFINT enable

	/* 1 us Tstab time is required before writing a second 1 to ADON to start conversions 
	(see p 98 of datasheet) */
	timedelay(2 * ticksperus);	// Wait

	/* Set sample times for channels */
	//                PC1/IN11,      PC2/IN12,       PC4/IN14,       Vrefint         Temp
	ADC1_SMPR1 = (  (SMP11 << 3) | (SMP12 << 6) | (SMP14 << 12) | (SMP16 << 18) | (SMP17 << 21) ); 

	/* Setup the number of channels scanned (p 238) */
	ADC1_SQR1 =  ( (NUMBERADCCHANNELS_MC-1) << 20) ;

	/* This maps the ADC channel number to the position in the conversion sequence */
	// Load channels for conversions sequences
//	ADC1_SQR3 = ( 0 << 0) | ( 2 << 5) | ( 3 << 10); // For PA0/IN0,  PA2/IN2,  PA3/IN3  for sequence #1, #2, #3
	// For      PC1/IN11,    PC2/IN12,    PC4/IN14,    IN16        IN17
	//              1           2           3            4            5
	ADC1_SQR3 = (11 << 0) | (12 << 5) | (14 << 10) | (16 << 15) | (17 << 20);	

	/* Setup the DMA */		
	if ((tmp = adc_dma_init_sequence()) != 0) return tmp;

	/* Setup for timer to trigger ADC normal sequence conversions. (countdowntimers.c) */
#ifdef TIMERTIGGERED
	timer_sw_ptr = &adc_trigger;
	timer_debounce_init();
#endif

	return 0;
}

/*#######################################################################################
 * ISR routine for DMA1 Channel1 reading ADC regular sequence of adc channels
 *####################################################################################### */
unsigned int adc_mc_debug0;
unsigned int adc_mc_debug0_prev;
unsigned int adc_mc_debug0_diff;

void DMA2_MC_IRQHandler(volatile u32* p)
{
	__attribute__ ((unused))  u32 tmp;
	p = p; // Pointer is not used; this is to avoid compiler warning

	/* Include code appropriate for the high or low register */
	#if ADC_DMA_STREAM > 3

	// High register  is used with this stream

	if ( (DMA_HISR(DMA2_BASE) & 0x20) != 0 )	// Is this a transfer complete interrupt?
	{ // Here, yes.  The second sequence has been converted and stored in the second buffer
		strADC1dr.flg  = 1;	// Set the index to the second buffer.  It is ready.
		DMA_HIFCR(DMA2_BASE) = 0x20;	// Clear transfer complete flag (p 208)
	}
	else
	{
		if ( (DMA_HISR(DMA2_BASE) & 0x10) != 0 )	// Is this a half transfer complete interrupt?
		{ // Here, yes.  The first sequence has been converted and stored in the first buffer
			strADC1dr.flg  = 0;	// Set index to the first buffer.  It is ready.
			DMA_HIFCR(DMA2_BASE) = 0x10;	// Clear transfer complete flag (p 208)
		}
	}
	tmp = DMA_HIFCR(DMA2_BASE);	// Avoid tail chaining

	#else

	// Low register is used with this stream

	if ( (DMA_LISR(DMA2_BASE) & 0x20) != 0 )	// Is this a transfer complete interrupt?
	{ // Here, yes.  The second sequence has been converted and stored in the second buffer
		strADC1dr.flg  = 1;	// Set the index to the second buffer.  It is ready.
		DMA_LIFCR(DMA2_BASE) = 0x20;	// Clear transfer complete flag (p 208)
adc_mc_debug0_diff += 1;
	}
	else
	{
		if ( (DMA_LISR(DMA2_BASE) & 0x10) != 0 )	// Is this a half transfer complete interrupt?
		{ // Here, yes.  The first sequence has been converted and stored in the first buffer
			strADC1dr.flg  = 0;	// Set index to the first buffer.  It is ready.
			DMA_LIFCR(DMA2_BASE) = 0x10;	// Clear transfer complete flag (p 208)
		}
adc_mc_debug0_diff += 1;

	}
	tmp = DMA_LIFCR(DMA2_BASE);	// Avoid tail chaining

	#endif


/* Double buffer the sequence of channels converted.  When the DMA goes from the first
to the second buffer a half complete interrupt is generated.  When it completes the
storing of two half buffers a transfer complete interrupt is issued, and the DMA address
pointer is automatically reloaded with the beginning of the buffer space (i.e. circular). 

'flg' is the high order index into the two dimensional array. 

'cnt' is a running counter of sequences converted.  Maybe not too useful except for debugging */



	/* Trigger a pending interrupt that will handle filtering the ADC readings. */
	NVICISPR(NVIC_FSMC_IRQ);	// Set pending (low priority) interrupt for  ('../lib/libusartstm32/nvicdirect.h')

	return;
}
/*#######################################################################################
 * ISR routine for handling lower priority procesing
 *####################################################################################### */
/* Pointer to functions to be executed under a low priority interrupt, forced by DMA interrupt. */
void 	(*dma_ll_ptr)(void) = 0;		// DMA -> FSMC  (low priority)

void FSMC_IRQHandler(void)
{ // Here, DMA interrupt when 1/2 of a the buffer has filled.
	/* Run the readings through the cic filter */
	adc_cic_filtering();

	/* Call other routines if an address is set up */
	if (dma_ll_ptr != 0)	// Having no address for the following is bad.
		(*dma_ll_ptr)();	// Go do something
	return;
}
/* $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
         Below is CIC Filtering of DMA stored ADC readings 
 $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ */
/* **** CIC filtering stuff **** */


long	adc_last_filtered[NUMBERADCCHANNELS_MC];	// Last computed & filtered value for each channel

static struct CICLN2M3 adc_cic[NUMBERADCCHANNELS_MC];	// CIC intermediate storage

/******************************************************************************
 * static void adc_cic_init(void);
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

static void adc_cic_init(void)
{
	int i,j;

	/* Initialize the structs that hold the CIC filtering intermediate values. */
	strADC1dr.cnt = 0;
	strADC1dr.flg = 0;
	for (i = 0; i < NUMBERADCCHANNELS_MC; i++)	
	{
		adc_cic[i].usDecimateNum = DECIMATION_MC; // Decimation number
		adc_cic[i].usDecimateCt = 0;		// Decimation counter
		adc_cic[i].usDiscard = DISCARD_MC;	// Initial discard count
		adc_cic[i].usFlag = 0;			// 1/2 buffer flag
		for (j = 0; j < 3; j++)
		{ // Very important that the integrators begin with zero.
			adc_cic[i].lIntegral[j] = 0;
			adc_cic[i].lDiff[j][0] = 0;
			adc_cic[i].lDiff[j][1] = 0;
		}	
	}
	return;
}

/* ########################## UNDER LOW PRIORITY SYSTICK INTERRUPT ############################### 
 * Run the latest adc readings through the cic filter.
 * ############################################################################################### */
u32 cic_debug0;

static void adc_cic_filtering(void)
{
	int i,j;
	short *p; 

	p = &strADC1dr.in[strADC1dr.flg][0][0];

	/* Run the 1/2 buffer load through the cic filter */
	for (j = 0; j < NUMBERSEQUENCES; j++)
	{
		for (i = 0; i < NUMBERADCCHANNELS_MC; i++)
		{
			adc_cic[i].nIn = *p++; 		// Load reading to filter struct
			
			if (cic_filter_l_N2_M3 (&adc_cic[i]) != 0) // Filtering complete?
			{ // Here, yes.  Save filter output in buffer for 'main'
				adc_last_filtered[i] = adc_cic[i].lout >> CICSCALE; // Scale and save
cic_debug0 += 1;

			}
		}
	}
	return;
}

