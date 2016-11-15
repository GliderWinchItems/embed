/******************************************************************************
* File Name          : sdadc_discovery_nodma.c
* Date First Issued  : 03/27/2015
* Board              : Discovery F3 w F373 processor
* Description        : SDADC not using DMA 
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
	f3gpiopins_Config ((volatile u32*)GPIOA, 3, (struct PINCONFIG*)&inputadcnodma);
     Sample times for the channel:
	ADC1_SMPR2 = ( (SMP3 << 9) | (SMP2 << 6) | (SMP1 << 0) );
     Channel versus conversion sequence:
	ADC1_SQR3 = (0 << 0) | (2 << 5) | (3 << 10);
*/
#include "libopencm3/stm32/f3/sdadc.h"
#include "libopencm3/cm3/common.h"
#include "nvicdirect.h" 
#include "libopencm3/stm32/f3/nvic.h"
#include "f3DISCpinconfig.h"
#include "nvic_dma_mgrf3.h"
#include "libopencm3/stm32/gpio.h"
#include "libopencm3/stm32/rcc.h"
#include "libopencm3/stm32/dma.h"
#include "libopencm3/stm32/pwr.h"
#include "common_can.h"
#include "sdadc_discovery_nodma.h"

#include "panic_ledsDf3.h"
#include "f3Discovery_led_pinconfig.h"

#include "DTW_counter_F3.h"
#include <stdint.h>
#include "cic_filter_l_N8_M3_f3.h"

/* Subroutines */
static void sdadc_config(void);

/* APB2 bus frequency is needed to be able to set ADC prescalar so that it is less than 30 MHz, 
   but as high as possible. */
extern unsigned int	pclk2_freq;	// APB2 bus frequency in Hz (see 'lib/libmiscstm32/clockspecifysetup.c')

/* SYSCLK frequency is used to convert millisecond delays into SYSTICK counts */
extern unsigned int	sysclk_freq;	// SYSCLK freq in Hz (see 'lib/libmiscstm32/clockspecifysetup.c')

/* CIC integrators for each possible SDADC module and port (3 * 9) */
int long long cic1[NUMBERSDADCS][MAXPORTSPERSDADC];

/* cic filtering buffer for each port  */
static struct CICLN8M3 sdbuf1[NUMBERSDADC1];
static struct CICLN8M3 sdbuf2[NUMBERSDADC2];
static struct CICLN8M3 sdbuf3[NUMBERSDADC3];

/*  */
static struct SDADCPORTPARM portparm1;
static struct SDADCPORTPARM portparm2;
static struct SDADCPORTPARM portparm3;


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
 * static int sdadc_discovery_nodma_chan(struct CICLN8M3 *pcic, uint32_t bits);
 * @brief 	: Set channel number in cic buffer from selection bits in .h file
 * @param	: pcic = pointer to cic struct/buffer
 * @param	: bits = bit pattern for selecting channels (.h file)
*******************************************************************************/
static int sdadc_discovery_nodma_chan(struct CICLN8M3 *pcic, uint32_t bits)
{
	int i;
	uint32_t ct = 0;	// Count number of bits ON as a check
	for (i = 0; i < MAXPORTSPERSDADC; i++)
	{
		if ((bits & 0x1) != 0)	// Check low ord bit
		{ 
			pcic->ucChan = i; // Set channel number (0 - 8)
			pcic += 1;	// Next buffer/struct
			ct += 1;
		}
		bits = bits >> 1;	// Next channel bit
	}
	return ct;
}
/******************************************************************************
 * int sdadc_discovery_nodma_init(void);
 * @brief 	: Initialize SDADC, calibration, etc., then start it running
 * @return	: 0 = OK; negative = failed
*******************************************************************************/
int sdadc_discovery_nodma_init(void)
{
	/* Configure pins & registers for SDADC. */
	sdadc_config();

	// Set channel numbers.
	sdadc_discovery_nodma_chan(&sdbuf1[0], SDADC1PORTS);
	sdadc_discovery_nodma_chan(&sdbuf2[0], SDADC2PORTS);
	sdadc_discovery_nodma_chan(&sdbuf3[0], SDADC3PORTS);

	// Set pointers
	portparm1.p = &sdbuf1[0];
	portparm2.p = &sdbuf2[0];
	portparm3.p = &sdbuf3[0];

	portparm1.p_begin = portparm1.p;
	portparm2.p_begin = portparm2.p;
	portparm3.p_begin = portparm3.p;

	portparm1.p_next = portparm1.p + 1;
	portparm2.p_next = portparm2.p + 1;
	portparm3.p_next = portparm3.p + 1;

	portparm1.p_end = &sdbuf1[NUMBERSDADC1];
	portparm2.p_end = &sdbuf2[NUMBERSDADC2];
	portparm3.p_end = &sdbuf3[NUMBERSDADC3];
	
	// Set channel number for first conversion
	SDADC1_CR2 &= (0xf << 16); // Clear old chan number
	SDADC2_CR2 &= (0xf << 16);
	SDADC3_CR2 &= (0xf << 16);

	SDADC1_CR2 |= (portparm1.p->ucChan << 16); // Set new chan number
	SDADC2_CR2 |= (portparm2.p->ucChan << 16);
	SDADC3_CR2 |= (portparm3.p->ucChan << 16);

	/* Start continuous regular conversions. 
	   After initial conversion started, set the next port/channel
 	   number.  When the first interrupt occurs this next port/channel
	   number will have taken effect for the next conversion. */
	#if SDADC3PORTS
		SDADC3_CR2 |= (1<<23); 	// Start regular conversions
		while ((SDADC3_ISR & (1<<14)) == 0); // Wait till recognized
		SDADC3_CR2 &= (0xf << 16);	// Clear old chan number
		SDADC3_CR2 |= (portparm3.p->ucChan << 16); // Set new chan number
	#endif
	#if SDADC2PORTS
		SDADC2_CR2 |= (1<<23);
		while ((SDADC2_ISR & (1<<14)) == 0);
		SDADC2_CR2 &= (0xf << 16);
		SDADC2_CR2 |= (portparm2.p->ucChan << 16);
	#endif
	#if SDADC1PORTS
		SDADC1_CR2 |= (1<<23);
		while ((SDADC1_ISR & (1<<14)) == 0);
		SDADC1_CR2 &= (0xf << 16);
		SDADC1_CR2 |= (portparm1.p->ucChan << 16);
	#endif

	return 0;
}

/******************************************************************************
 * Pin configuration
*******************************************************************************/
//  ADC pin configuration  */
const struct PINCONFIG	inputadcnodma = { \
	GPIO_MODE_ANALOG,	// mode: Analog 
	0,	 		// output type: not applicable 		
	0, 			// speed: not applicable
	0, 			// pull up/down: none
	0 };			// AFRLy & AFRHy selection (filled in on the fly)
/******************************************************************************
 * static void sdadc_config(void);
 * @brief 	: Configure pins and registers for SDADC
*******************************************************************************/
uint32_t sdbug0;
uint32_t sdbug1;

static void sdadc_config(void)
{
	uint32_t ticksperusec = sysclk_freq/1000000;

	/*  Setup SDADC pins for ANALOG INPUT */

// NOTE: '//L' = Discovery board LED.  '//T' Not used in test
// Single ended
// '*' = pin can be used either with SDADC1 or SDADC2
// Column 'A' each SDADC can have 9 single-ended inputs, but a few are shared (see *)
// Column 'B' 8P8C connector pin number (connector 1 & 3) 6P6C (connector 2)

	// SDADC1                                                                          A  B chan
	f3gpiopins_Config ((volatile u32*)GPIOB, 0,  (struct PINCONFIG*)&inputadcnodma);	// 1  8  6
	f3gpiopins_Config ((volatile u32*)GPIOB, 1,  (struct PINCONFIG*)&inputadcnodma);	// 2  7  5
	f3gpiopins_Config ((volatile u32*)GPIOB, 2,  (struct PINCONFIG*)&inputadcnodma);	//*3  6  4
	f3gpiopins_Config ((volatile u32*)GPIOE, 7,  (struct PINCONFIG*)&inputadcnodma);	//*4  5  3
	f3gpiopins_Config ((volatile u32*)GPIOE, 8,  (struct PINCONFIG*)&inputadcnodma);	// 5  4  8
	f3gpiopins_Config ((volatile u32*)GPIOE, 9,  (struct PINCONFIG*)&inputadcnodma);	//*6  3  7
	f3gpiopins_Config ((volatile u32*)GPIOE, 10, (struct PINCONFIG*)&inputadcnodma);	// 7  2  2
	f3gpiopins_Config ((volatile u32*)GPIOE, 11, (struct PINCONFIG*)&inputadcnodma);	//*8  1  1
	f3gpiopins_Config ((volatile u32*)GPIOE, 12, (struct PINCONFIG*)&inputadcnodma);	//*9  -  0

	// SDADC2
//T	f3gpiopins_Config ((volatile u32*)GPIOB, 2,  (struct PINCONFIG*)&inputadcnodma);	//*3  -  6
//T	f3gpiopins_Config ((volatile u32*)GPIOE, 7,  (struct PINCONFIG*)&inputadcnodma);	//*4  -  5
//L	f3gpiopins_Config ((volatile u32*)GPIOE, 8,  (struct PINCONFIG*)&inputadcnodma);	// 5  -  8
//L	f3gpiopins_Config ((volatile u32*)GPIOE, 9,  (struct PINCONFIG*)&inputadcnodma);	//*6  -  7
//L	f3gpiopins_Config ((volatile u32*)GPIOE, 11, (struct PINCONFIG*)&inputadcnodma);	//*8  -  4
//L	f3gpiopins_Config ((volatile u32*)GPIOE, 12, (struct PINCONFIG*)&inputadcnodma);	//*9  2  3
	f3gpiopins_Config ((volatile u32*)GPIOE, 13, (struct PINCONFIG*)&inputadcnodma);	// 1  3  2
	f3gpiopins_Config ((volatile u32*)GPIOE, 14, (struct PINCONFIG*)&inputadcnodma);	// 2  5  1
	f3gpiopins_Config ((volatile u32*)GPIOE, 15, (struct PINCONFIG*)&inputadcnodma);	// 3  4  0

	// SDADC3
	f3gpiopins_Config ((volatile u32*)GPIOB, 14, (struct PINCONFIG*)&inputadcnodma);	// 1  8  8
	f3gpiopins_Config ((volatile u32*)GPIOB, 15, (struct PINCONFIG*)&inputadcnodma);	// 2  7  7
	f3gpiopins_Config ((volatile u32*)GPIOD, 8,  (struct PINCONFIG*)&inputadcnodma);	// 3  6  6
	f3gpiopins_Config ((volatile u32*)GPIOD, 9,  (struct PINCONFIG*)&inputadcnodma);	// 4  5  5
	f3gpiopins_Config ((volatile u32*)GPIOD, 10, (struct PINCONFIG*)&inputadcnodma);	// 5  4  4
	f3gpiopins_Config ((volatile u32*)GPIOD, 11, (struct PINCONFIG*)&inputadcnodma);	// 6  3  3
	f3gpiopins_Config ((volatile u32*)GPIOD, 12, (struct PINCONFIG*)&inputadcnodma);	// 7  2  2
	f3gpiopins_Config ((volatile u32*)GPIOD, 13, (struct PINCONFIG*)&inputadcnodma);	// 8  1  1
	f3gpiopins_Config ((volatile u32*)GPIOD, 14, (struct PINCONFIG*)&inputadcnodma);	// 9  -  0

	/* Power Control Register: Supply power to the SDADCs. */
	RCC_APB1ENR |= (1 << 28);	// Enable Power module
	PWR_CR |= (0x7 << 9);		// Enable SDs 1,2,3

	/* Reset bus clocking  fr SDADC1,2,3 */
	RCC_APB2RSTR = (1<<26) | (1<<25) | (1<<24);
	RCC_APB2RSTR = 0;

timedelay(1000 * ticksperusec);  // 1 ms

	/* Enable bus clocking for SDADCs: 3, 2, 1 */
	RCC_APB2ENR |= (1<<26) | (1<<25) | (1<<24);

	/* Set prescalar for SD clocking from AHB1 clock. < 6 MHz */
	/* NOTE: At 72 MHz SD clock is the max 6.0 MHz. */
	/*       At 64 MHz SD clock is at 5.333 MHz. */
	RCC_CFGR &= ~(0x1f << 27);	// SDPRE[4:0]
	RCC_CFGR |=  (0x15 << 27);	// Divide by 12 code

	/* SDADC disable (reset) */
	SDADC1_CR2 = 0;
	SDADC2_CR2 = 0;
	SDADC3_CR2 = 0;

	/* Select Vrefsd reference.  ADON must be 0. */

/* "If VDDSDx is selected through the
reference voltage selection bits (REFV=”11” in SDADC_CR1 register), the application must
first configure REFV and then wait for at least 2 ms before enabling the SDADC (ADON=1
in SDADC_CR2 register). The 1 μF decoupling capacitor must be fully charged before
enabling the SDADC." */

	timedelay(10000 * ticksperusec);  // 10 ms

	/* Select internal Vref: 1 = 1.2v, 2 = 1.8v, 3 = Vddsd. */
	SDADC1_CR1 = (0x2 << 8);

	/* This may not be needed... */
 	timedelay(5000 * ticksperusec);  // 5 ms

	/* SDADC enable. Set ADON */
	SDADC1_CR2 = 0x1;
	SDADC2_CR2 = 0x1;
	SDADC3_CR2 = 0x1;

	/* Check that stabilization has completed. */
	while ( (SDADC1_ISR & (1<<15)) != 0);
	while ( (SDADC2_ISR & (1<<15)) != 0);
	while ( (SDADC3_ISR & (1<<15)) != 0);

	/* Generous additonal delay for stabilization-- jic */
	timedelay(3000 * ticksperusec);  // 3 ms

	/* Control Register 1.  Set INIT ON. */
	SDADC1_CR1 |= (1 << 31);
	SDADC2_CR1 |= (1 << 31);
	SDADC3_CR1 |= (1 << 31);

	/* Wait for INITRDY to show INIT is active.  */
	while( (SDADC1_ISR & (1<<31)) == 0);
	while( (SDADC2_ISR & (1<<31)) == 0);
	while( (SDADC3_ISR & (1<<31)) == 0);

	/* Control Register 1 */
	// Use default

	/* Control Register 2 */
	//  Reg continuous conv 
	SDADC1_CR2 |= (1 << 22);
	SDADC2_CR2 |= (1 << 22);
	SDADC3_CR2 |= (1 << 22);

	/* SDADC channel configuration register SDADCX_CONFCHR1,2 */
	// Use '00' default - all channels use THE SDADC_CONF0R configuration

	/* SDADC configuration 0 register (SDADC_CONF0R) */
	// Gain = 1x; Ground ref.
	SDADC1_CONF0R = (0x3 << 26)|(0x1<<30); // single-ended zero-volt reference mode
	SDADC2_CONF0R = (0x3 << 26)|(0x1<<30); // single-ended zero-volt reference mode
	SDADC3_CONF0R = (0x3 << 26)|(0x1<<30); // single-ended zero-volt reference mode

	/* JIC--clear all isr flags */
	SDADC1_CLRISR = 0x15;
	SDADC2_CLRISR = 0x15;
	SDADC3_CLRISR = 0x15;

	/* Initialization of registers ends. */
	SDADC1_CR1 &= ~(1 << 31);	// Turn INIT off
	SDADC2_CR1 &= ~(1 << 31);
	SDADC3_CR1 &= ~(1 << 31);

	/* Wait for INITRDY to show INIT is off.  */
	while( (SDADC1_ISR & (1<<31)) != 0);
	while( (SDADC2_ISR & (1<<31)) != 0);
	while( (SDADC3_ISR & (1<<31)) != 0);

	/* JIC. */
	timedelay(1000 * ticksperusec);  // 1 ms

sdbug0 = DTWTIME;	
	/* Set "start calibration" bit. */
	SDADC1_CR2 |= (1 << 4);	// Start calibration
	SDADC2_CR2 |= (1 << 4);	// Start calibration
	SDADC3_CR2 |= (1 << 4);	// Start calibration

	/* Wait for calibration to complete */
	while( (SDADC1_ISR & 0x1) == 0);
	while( (SDADC2_ISR & 0x1) == 0);
	while( (SDADC3_ISR & 0x1) == 0);
sdbug1 = DTWTIME;

	/* Generous delay for stabilization-- jic */
	timedelay(100 * ticksperusec);  // 100us

	/* JIC--clear all isr flags */
	SDADC1_CLRISR = 0x15;
	SDADC2_CLRISR = 0x15;
	SDADC3_CLRISR = 0x15;

	return;
}
/******************************************************************************
 * void sdadc_discovery_nodma_request_calib(void);
 * @brief 	: Calibration with be inserted after the current conversion completes
*******************************************************************************/
void sdadc_discovery_nodma_request_calib(void)
{
	//  Stop Regular continuous conv
//	SDADC1_CR2 &= ~(1 << 22);
//	SDADC2_CR2 &= ~(1 << 22);
//	SDADC3_CR2 &= ~(1 << 22);

	/* Set "start calibration" bit. */
	SDADC1_CR2 |= (1 << 4);	// Start calibration
	SDADC2_CR2 |= (1 << 4);	// Start calibration
	SDADC3_CR2 |= (1 << 4);	// Start calibration

	//  Start Regular continuous conv
	SDADC1_CR2 |= (1 << 22);
	SDADC2_CR2 |= (1 << 22);
	SDADC3_CR2 |= (1 << 22);

	return;	
}

/*#######################################################################################
 * ISR routines for handling lower priority procesing
 * Triggered when DMA channel passes 1/2 or end of buffer points.
 *####################################################################################### */

/*############## SDADC1 low level interrupt ############################################ */
void SDADC1_IRQHandler_NODMA(void)
{
	long ltmp;
	unsigned short ustmp;

	if ((SDADC1_ISR & (1<<3)) != 0)	// End of regular conversion flag	
	{
	
		ltmp = SDADC1_RDATAR;	// Read result (clears isr flag, clears register)
		ustmp = cic_filter_l_N8_M3_f3 (portparm1.p, ltmp);
		portparm1.p = portparm1.p_next;
		portparm1.p_next += 1;
		if (portparm1.p_next >= portparm1.p_end) // End of sequence of ports?
		{ // Yes, start another sequence
			portparm1.p_next = portparm1.p_begin;
			if (ustmp != 0)	// End of filtering?
			{ // Yes
				/* Trigger low level interrupt */
			}		
		}
		/* In continuous mode the next conversion has already started, RCIP = 1,
                   so the new channel number takes effect upon completion of current conversion. */
		SDADC1_CR2 &= (0xf << 16);	// Clear channel 
		SDADC1_CR2 |= (portparm1.p_next->ucChan << 16);	// Set channel number for next conversion
	}
	return;
}
/*############## SDADC2 low level interrupt ############################################ */
void SDADC2_IRQHandler_NODMA(void)
{
	return;
}
/*############## SDADC3 low level interrupt ############################################ */
void SDADC3_IRQHandler_NODMA(void)
{
	return;
}

