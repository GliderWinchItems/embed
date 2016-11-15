/******************************************************************************
* File Name          : sdadc_discovery.c
* Date First Issued  : 01/20/2015
* Board              : Discovery F3 w F373 processor
* Description        : Initial shakeout of SDADC usage
*******************************************************************************/
/*
Example of timing/rates--

At sysclk = 64MHz, divided by 12 to get SDADC clock = 5 1/3 MHz--
one SDADC clock                     = 0.1875 us
360 SDADC clocks for one conversion = 67.5 us
nine conversions for one sequence   = 607.5 us
eight, nine conversion, sequences   = 4.860 ms per DMA interrupt
filter decimation of 64 * 607.5     = 33.880 ms per filtered output

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
#include "sdadc_discovery.h"

#include "panic_ledsDf3.h"
#include "f3Discovery_led_pinconfig.h"

#include "DTW_counter_F3.h"
#include <stdint.h>

/* Subroutines */
static void sdadc_config(void);
void DMA2_SDADC_IRQHandler(volatile u32* pall);
static int dma_sdadcx_init(const struct SDADCFIX* p);

uint32_t recalibctr = 0;	// Running count of re-calibrations 

/* SYSCLK frequency is used to convert millisecond delays into SYSTICK counts */
extern unsigned int	sysclk_freq;	// SYSCLK freq in Hz (see 'lib/libmiscstm32/clockspecifysetup.c')

/* Double buffers for DMA for each SDADC. */
/* ### Make these static after debugging. #### */
// Note: the order of dma storage matches the order of array subscripts
 int16_t buff1[2][NUMBERSEQ1][NUMBERSDADC1];	/* Double DMA buffer for #1 */
 int16_t buff2[2][NUMBERSEQ2][NUMBERSDADC2];	/* Double DMA buffer for #2 */
 int16_t buff3[2][NUMBERSEQ3][NUMBERSDADC3];	/* Double DMA buffer for #3 */

/* Variables for each SDADC */
struct SDADCVAR sdadcvar[NUMBERSDADCS];

/* Fixed pointers and things for configuring each SDADC & DMA */
// NOTE: 'nvic_dma_mgrf3.c' sets up to pass a pointer to the
// the ISR when the DMA half or total buffer interrupts occur.
const struct SDADCFIX sdadcfix[NUMBERSDADCS] = { \
   { 
	&buff1[0][0][0],/* Start of double buffer 		*/
	(u32)SDADC1,	/* Address for this SDADC module 	*/
	&sdadcvar[0],	/* Variables				*/
	61,		/* SDADC1 IRQ number			*/
	3,		/* DMA channel for this SDADC 		*/
	(1<<10),	/* Channel mask for half buff isr flag 	*/
	(0xf<<8),	/* Channel mask to reset all flags 	*/
	0,		/* SDADC number - 1 			*/
	(2 * NUMBERSDADC1 * NUMBERSEQ1), /* Number half-words 	*/
	0x90,		/* Low level interrupt priority		*/
	0x1,		/* DMA priority: medium			*/
	58,		/* DMA2 Chan 3 IRQ number		*/
	NUMBERSEQ1,	/* Number of sequences in 1/2 buffer	*/
	NUMBERSDADC1,	/* Number of ADCs in one sequence	*/
   },
   {
	&buff2[0][0][0],/* Start of double buffer 		*/
	(u32)SDADC2,	/* Address for this SDADC module 	*/
	&sdadcvar[1],	/* Variables				*/
	62,		/* SDADC2 IRQ number			*/
	4,		/* DMA channel for this SDADC 		*/
	(1<<14),	/* Channel mask for half buff isr flag 	*/
	(0xf<<12),	/* Channel mask to reset all flags 	*/
	1,		/* SDADC number - 1 			*/
	(2 * NUMBERSDADC2 * NUMBERSEQ2), /* Number half-words 	*/
	0x90,		/* Low level interrupt priority		*/
	0x0,		/* DMA priority: low			*/
	59,		/* DMA2 Chan 4 IRQ number		*/
	NUMBERSEQ2,	/* Number of sequences in 1/2 buffer	*/
	NUMBERSDADC2,	/* Number of ADCs in one sequence	*/
   },
   {
	&buff3[0][0][0],/* Start of double buffer 		*/
	(u32)SDADC3,	/* Address for this SDADC module 	*/
	&sdadcvar[2],	/* Variables				*/
	63,		/* SDADC3 IRQ number			*/
	5,		/* DMA channel for this SDADC 		*/
	(1<<18),	/* Channel mask for half buff isr flag 	*/
	(0xf<<16),	/* Channel mask to reset all flags 	*/
	2,		/* SDADC number - 1 			*/
	(2 * NUMBERSDADC3 * NUMBERSEQ3), /* Number half-words 	*/
	0x90,		/* Low level interrupt priority		*/
	0x2,		/* DMA priority: high			*/
	60,		/* DMA2 Chan 5 IRQ number		*/
	NUMBERSEQ3,	/* Number of sequences in 1/2 buffer	*/
	NUMBERSDADC3,	/* Number of ADCs in one sequence	*/
   }
};
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
 * int sdadc_discovery_init(void);
 * @brief 	: Initialize SDADC, DMA, calibration, etc., then start it running
 * @return	: 0 = OK; negative = failed
*******************************************************************************/
int sdadc_discovery_init(void)
{
	int32_t tmp;

	/* Configure pins & registers for SDADC. */
	sdadc_config();

	/* Setup buffers and configure DMA for each SDADC. */
	tmp  = dma_sdadcx_init(&sdadcfix[0]);
	tmp |= dma_sdadcx_init(&sdadcfix[1]);
	tmp |= dma_sdadcx_init(&sdadcfix[2]);

	/* Start injected conversions. */
	// If no ports selected, do not start
	#if SDADC3PORTS
		SDADC3_CR2 |= (1<<15); 
	#endif
	#if SDADC2PORTS
		SDADC2_CR2 |= (1<<15);
	#endif
	#if SDADC1PORTS
		SDADC1_CR2 |= (1<<15);
	#endif

	return 0;
}
/******************************************************************************
static void setup_one_dma_chan(const struct SDADCFIX* p);
 * @brief 	: Initialize the DMA for the SDADCx
 * @param	: p = pointer to control block for this SDADC
*******************************************************************************/
static void setup_one_dma_chan(const struct SDADCFIX* p)
{
	/* Disable DMA so that the registers can be changed. */
	DMA2_CCR(p->dmachan) &= ~0x1;

	/* Init some variables associated with this SDADC/DMA channel. */
	p->var->idx  = 0;	// Buffer half that is available index reset
	p->var->flag = 0;	// Data ready flag reset
	p->var->recalib_ctr = p->var->recalib_ld; // Recalib counter reset

	/* Set DMA peripheral address for SDADCx (x = 3-5) SDADC_JDATAR */
	DMA2_CPAR(p->dmachan) = (u32)&SDADC_JDATAR(p->sdbase);	// Address of SDADCx injected data register

	/* Set DMA memory address into which the data flows (silently & fast). */
	DMA2_CMAR(p->dmachan) = (u32)p->base;

	/* DMA stream configuration register.  Data direction is periheral-to-memory. */ 
	//                            Mem 16b  |  Per 16b |   MINC   |  CIRC  |  HTIE  |  TCIE  
	DMA2_CCR(p->dmachan)  =    ( (0x1<<10) | (0x1<<8) | (0x1<<7) | (1<<5) | (1<<2) | (1<<1) );
	DMA2_CCR(p->dmachan) |= (p->dma_priority & 0x3) << 12;	// DMA channel priority
	
	/* Set the number of bytes in the buff */
	DMA2_CNDTR(p->dmachan) = p->size;	// Number of data items before wrap-around
	
	/* NOTE: returning with DMA not enabled.  Someone later enables it. */

	return;
}
/******************************************************************************
 * static int dma_sdadcx_init(const struct SDADCFIX* p);
 * @brief 	: Initialize the DMA for the SDADCx
 * @param	: p = pointer to control block for this SDADC
*******************************************************************************/
static int dma_sdadcx_init(const struct SDADCFIX* p)
{	
	int tmp = 0;

	/* Set dma stream interrupt to re-vector to this routine, and check if this dma stream is already in use. */
	//                              local dma IRQ handler, pointer passed to us,   irq number
	tmp = nvic_dma_channel_vector_addf3( (void(*)(u32*))&DMA2_SDADC_IRQHandler, (u32*)p, p->dma_irq_num,p->dmachan);
	
	if (tmp != 0) return (-30 + tmp); // Return a negative error code.

	/* --------- Set up one DMA channel ------------------------------------------------------------------------ */
	setup_one_dma_chan(p);	// Return with DMA still not enabled

	/* Low level (SDADCx) interrupt for doing filtering */
	NVICIPR (p->llvl_irq, p->llvl_priority);// Set interrupt priority
	NVICISER(p->llvl_irq);			// Enable interrupt

	/* Enable DMA channel interrupt */
	NVICIPR (p->dma_irq_num,p->dma_priority);	// Set dma interrupt priority (tx)
	NVICISER(p->dma_irq_num);			// Enable dma interrupt (tx)

	/* Final enabling of DMA */
	DMA2_CCR(p->dmachan) |= 0x1;	// Enable dma channel

	return 0;
}
/******************************************************************************
 * Re-init DMA rather than just initial init
*******************************************************************************/
static int dma_sdadcx_re_init(const struct SDADCFIX* p)
{
	setup_one_dma_chan(p);
	DMA2_CCR(p->dmachan) |= 0x1;	// Enable dma channel
	return 0;
}
/******************************************************************************
 * Pin configuration
*******************************************************************************/
//  ADC pin configuration  */
const struct PINCONFIG	inputadc = { \
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
	f3gpiopins_Config ((volatile u32*)GPIOB, 0,  (struct PINCONFIG*)&inputadc);	// 1  8  6
	f3gpiopins_Config ((volatile u32*)GPIOB, 1,  (struct PINCONFIG*)&inputadc);	// 2  7  5
	f3gpiopins_Config ((volatile u32*)GPIOB, 2,  (struct PINCONFIG*)&inputadc);	//*3  6  4
	f3gpiopins_Config ((volatile u32*)GPIOE, 7,  (struct PINCONFIG*)&inputadc);	//*4  5  3
	f3gpiopins_Config ((volatile u32*)GPIOE, 8,  (struct PINCONFIG*)&inputadc);	// 5  4  8
	f3gpiopins_Config ((volatile u32*)GPIOE, 9,  (struct PINCONFIG*)&inputadc);	//*6  3  7
	f3gpiopins_Config ((volatile u32*)GPIOE, 10, (struct PINCONFIG*)&inputadc);	// 7  2  2
	f3gpiopins_Config ((volatile u32*)GPIOE, 11, (struct PINCONFIG*)&inputadc);	//*8  1  1
	f3gpiopins_Config ((volatile u32*)GPIOE, 12, (struct PINCONFIG*)&inputadc);	//*9  -  0

	// SDADC2
//T	f3gpiopins_Config ((volatile u32*)GPIOB, 2,  (struct PINCONFIG*)&inputadc);	//*3  -  6
//T	f3gpiopins_Config ((volatile u32*)GPIOE, 7,  (struct PINCONFIG*)&inputadc);	//*4  -  5
//T	f3gpiopins_Config ((volatile u32*)GPIOE, 8,  (struct PINCONFIG*)&inputadc);	// 5  -  8 L
//T	f3gpiopins_Config ((volatile u32*)GPIOE, 9,  (struct PINCONFIG*)&inputadc);	//*6  -  7 L
//T	f3gpiopins_Config ((volatile u32*)GPIOE, 11, (struct PINCONFIG*)&inputadc);	//*8  -  4 L
//T	f3gpiopins_Config ((volatile u32*)GPIOE, 12, (struct PINCONFIG*)&inputadc);	//*9  2  3 L
	f3gpiopins_Config ((volatile u32*)GPIOE, 13, (struct PINCONFIG*)&inputadc);	// 1  3  2 L
	f3gpiopins_Config ((volatile u32*)GPIOE, 14, (struct PINCONFIG*)&inputadc);	// 2  5  1 L
	f3gpiopins_Config ((volatile u32*)GPIOE, 15, (struct PINCONFIG*)&inputadc);	// 3  4  0 L

	// SDADC3
	f3gpiopins_Config ((volatile u32*)GPIOB, 14, (struct PINCONFIG*)&inputadc);	// 1  8  8 L
	f3gpiopins_Config ((volatile u32*)GPIOB, 15, (struct PINCONFIG*)&inputadc);	// 2  7  7 L
	f3gpiopins_Config ((volatile u32*)GPIOD, 8,  (struct PINCONFIG*)&inputadc);	// 3  6  6
	f3gpiopins_Config ((volatile u32*)GPIOD, 9,  (struct PINCONFIG*)&inputadc);	// 4  5  5
	f3gpiopins_Config ((volatile u32*)GPIOD, 10, (struct PINCONFIG*)&inputadc);	// 5  4  4
	f3gpiopins_Config ((volatile u32*)GPIOD, 11, (struct PINCONFIG*)&inputadc);	// 6  3  3
	f3gpiopins_Config ((volatile u32*)GPIOD, 12, (struct PINCONFIG*)&inputadc);	// 7  2  2
	f3gpiopins_Config ((volatile u32*)GPIOD, 13, (struct PINCONFIG*)&inputadc);	// 8  1  1
	f3gpiopins_Config ((volatile u32*)GPIOD, 14, (struct PINCONFIG*)&inputadc);	// 9  -  0

	/* Power Control Register: Supply power to the SDADCs. */
	RCC_APB1ENR |= (1 << 28);	// Enable Power module
	PWR_CR |= (0x7 << 9);		// Enable SDs 1,2,3

	/* Reset bus clocking for SDADC1,2,3 */
	RCC_APB2RSTR = (1<<26) | (1<<25) | (1<<24);
	RCC_APB2RSTR = 0;

	timedelay(1000 * ticksperusec);  // 1 ms jic

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
	//            Use J DMA | JSYNC 
	SDADC1_CR1 |= (1 << 16);
	SDADC2_CR1 |= (1 << 16);// | (1<<14);
	SDADC3_CR1 |= (1 << 16);// | (1<<14);

	/* Control Register 2 */
	//  J continuous conv  |  1 cal cyc
	SDADC1_CR2 |= (1 << 5) | (0x0 << 1);
	SDADC2_CR2 |= (1 << 5) | (0x0 << 1);
	SDADC3_CR2 |= (1 << 5) | (0x0 << 1);

	/* SDADC injected channel group selection register */
	// NOTE: scan sequence starts with highest channel first
	SDADC1_JCHGR = SDADC1PORTS;
	SDADC2_JCHGR = SDADC2PORTS;
	SDADC3_JCHGR = SDADC3PORTS;
	
	/* SDADC channel configuration register SDADCX_CONFCHR1,2 */
	// Use '00' default - all channels use THE SDADC_CONF0R configuration

	/* SDADC configuration 0 register (SDADC_CONF0R) */
	//      common mode Vdd/2 | single-ended zero-volt ref mode (default Gain = 1x)
	#define CONF0R_COMMON_MODE (0x1<<30)
	SDADC1_CONF0R = CONF0R_COMMON_MODE | (0x3 << 26);
	SDADC2_CONF0R = CONF0R_COMMON_MODE | (0x3 << 26);
	SDADC3_CONF0R = CONF0R_COMMON_MODE | (0x3 << 26);

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

	/* Enable End of calibration interrupt. */
	SDADC1_CR1 |= (1 << 0);
	SDADC2_CR1 |= (1 << 0);
	SDADC3_CR1 |= (1 << 0);	

	return;
}
/******************************************************************************
 * void sdadc_request_calib(void);
 * @brief 	: Stops J continuous, and causes a calibration sequence (w interrupt)
*******************************************************************************/
void sdadc_request_calib(void)
{
	//  Stop J continuous conv
	SDADC1_CR2 &= ~(1 << 5);
	SDADC2_CR2 &= ~(1 << 5);
	SDADC3_CR2 &= ~(1 << 5);

	/* Set "start calibration" bit. */
	SDADC1_CR2 |= (1 << 4);	// Start calibration
	SDADC2_CR2 |= (1 << 4);	// Start calibration
	SDADC3_CR2 |= (1 << 4);	// Start calibration

	/* SDADCx interrupts will re-init DMA and re-start conversions. */
	return;	
}
/******************************************************************************
 * void sdadc_set_recalib_ct(uint8_t sdnum, uint32_t count);
 * @brief 	: Loads a new count into the re-calibration counter
 * @param	: sdnum = use SDADC number (1,2,or 3) to count sequences
 * @param	: count = number of *dma interrupts* of ' SDADC sdnum' between re-calibrations 
 * @param	:         0 = no recalibrations
*******************************************************************************/
void sdadc_set_recalib_ct(uint8_t sdnum, uint32_t count)
{
	if (sdnum > NUMBERSDADCS) return;	// BPT (Bozo Programmer Test)
	if (sdnum == 0) return;

	/* Load the reload count that stops the speeding train. */
	sdadcfix[sdnum-1].var->recalib_ld  = count; // Set new re-load count
	sdadcfix[sdnum-1].var->recalib_ctr = count; // Set working counter

	return;
}
/*#######################################################################################
 * ISR DMA2 CH 3,4,5: Entered from nvic_dma_mgr.c which dispatches the DMAx_STREAMy interrupts
 * void DMA2_SDADC_IRQHandler(volatile u32* pctl);
 * @param	: pctl = pointer to sdadcfix control block
 *####################################################################################### */
void DMA2_SDADC_IRQHandler(volatile u32* pall)
{
	/* The following is for consistency in the code in this file. ('nvic_dma_mgr.c' uses volatile u32*) */
	struct SDADCFIX* pctl = (struct SDADCFIX*)pall;

	/* Half buffer flag (i.e. 1st of double buffer). */
	if ( (DMA2_ISR & pctl->dmamskh) != 0)
	{ // Here, half flag on.  First half of buffer ready
		pctl->var->idx = 0;
	}
	else
	{ // Here, end flag is on.  Last half of buffer ready
		pctl->var->idx = 1;
	}
	/* Clear all interrupt flags for this DMA channel */
	DMA2_IFCR = pctl->dmamska;

	/* Raise the data flag for all patriots. */
	pctl->var->flag = 1;

	/* Recalibration check. Skip if counter is zero. */
	if (pctl->var->recalib_ctr != 0)
	{
		pctl->var->recalib_ctr -= 1;	// Countdown counter
		if (pctl->var->recalib_ctr == 0) // Time to re-calib?
		{ // Here, trigger a calibration cycle

			/* Note: the re-load counter for next delay is setup when
			   dma is re-initialized during SDADCx interrupt
			   handling (e.g. see 'SDADC1_IRQHandler' below) and 
			   'setup_one_dma_chan(const struct SDADCFIX* p)'
			*/

			/*  Stop injected continuous conv mode, which is necessary for
			    the calibration to see the injected conversion complete. */
			SDADC_CR2(pctl->sdbase) &= ~(1 << 5); // JCONT = 0;

			/* Set "start calibration" request bit. */
			SDADC_CR2(pctl->sdbase) |= (1 << 4);	// STARTCALIB = 1;

			/* Disable the DMA channel as is known to do at least two 
			   and maybe more stores during the calibration. */
			DMA2_CCR(pctl->dmachan) &= ~0x1;

			recalibctr += 1;	// Running count of these events (debug)
		}
	}

	/* Set pending low level interrupt (e.g. SDADC1_Handler) */
	NVICISPR(pctl->llvl_irq);

	return;
}
/*#######################################################################################
 * ISR routines for handling lower priority procesing AND end of calibration interrupts.
 *
 * Triggered when DMA channel passes 1/2 or end of buffer points.
 *
 * ENOCAL bit is ON when there is an end-of-calibration (which requires re-initializing
 * the dma, j-continous bit, and starting the j-conversions.
 *####################################################################################### */

/*############## SDADC1 low level interrupt ############################################ */
u32 sdadcDebug0;
u32 sdadcDebug1;
u32 sdadcDebug2;
u32 sdadcDebug3;

void (*sdadc1_ll_ptr)(const struct SDADCFIX *p ) = NULL;	// Point processing routine
void SDADC1_IRQHandler(void)
{
	if ((SDADC1_ISR & 0x1) != 0) // End of calibration flag?
	{
		SDADC1_CLRISR = 0x1;		  // Reset interrupt flag
		dma_sdadcx_re_init(&sdadcfix[0]); // Re-init DMA
		SDADC1_CR2 |= (1 << 5) | (1<<15); // Continuous | Start
		sdadcfix[0].var->recalib_n += 1;  // Count recalibrations
	}
sdadcDebug0=DTWTIME;
	/* Call other routines if an address is set up */
	if (sdadc1_ll_ptr != NULL)	// Skip call if no address
		(*sdadc1_ll_ptr)(&sdadcfix[0]);	// Go do something
sdadcDebug1= (DTWTIME - sdadcDebug0);
if ((int)sdadcDebug1 > (int)sdadcDebug2) {sdadcDebug2 = sdadcDebug1;}
if (sdadcDebug3++ > 100)
{
  sdadcDebug3 = 0;
  sdadcDebug2 = 0;
}

	return;
}
/*############## SDADC2 low level interrupt ############################################ */
u32 sdadcDebug20;
u32 sdadcDebug21;
u32 sdadcDebug22;
u32 sdadcDebug23;

void (*sdadc2_ll_ptr)(const struct SDADCFIX *p) = NULL;	// Where show we go?
void SDADC2_IRQHandler(void)
{
	if ((SDADC2_ISR & 0x1) != 0) // End of calibration flag?
	{
		SDADC2_CLRISR = 0x1;		  // Reset interrupt flag
		dma_sdadcx_re_init(&sdadcfix[1]); // Re-init DMA
		SDADC2_CR2 |= (1 << 5) | (1<<15); // Continuous | Start
		sdadcfix[1].var->recalib_n += 1;  // Count recalibrations
	}
sdadcDebug20=DTWTIME;
	/* Call other routines if an address is set up */
	if (sdadc2_ll_ptr != NULL)	// Skip call if no address
		(*sdadc2_ll_ptr)(&sdadcfix[1]);	// Go do something
sdadcDebug21= (DTWTIME - sdadcDebug20);
if ((int)sdadcDebug21 > (int)sdadcDebug22) {sdadcDebug22 = sdadcDebug21;}
if (sdadcDebug23++ > 100)
{
  sdadcDebug23 = 0;
  sdadcDebug22 = 0;
}
	return;
}
/*############## SDADC3 low level interrupt ############################################ */
u32 sdadcDebug30;
u32 sdadcDebug31;
u32 sdadcDebug32;
u32 sdadcDebug33;

void (*sdadc3_ll_ptr)(const struct SDADCFIX *p) = NULL;	// Where show we go?
void SDADC3_IRQHandler(void)
{
	if ((SDADC3_ISR & 0x1) != 0) // End of calibration flag?
	{
		SDADC3_CLRISR = 0x1;		  // Reset interrupt flag
		dma_sdadcx_re_init(&sdadcfix[2]); // Re-init DMA
		SDADC3_CR2 |= (1 << 5) | (1<<15); // Continuous | Start
		sdadcfix[2].var->recalib_n += 1;  // Count recalibrations
	}

sdadcDebug30=DTWTIME;
	/* Call other routines if an address is set up */
	if (sdadc3_ll_ptr != NULL)	// Skip call if no address
		(*sdadc3_ll_ptr)(&sdadcfix[2]);	// Go do something
sdadcDebug31= (DTWTIME - sdadcDebug30);
if ((int)sdadcDebug31 > (int)sdadcDebug32) {sdadcDebug32 = sdadcDebug31;}
if (sdadcDebug33++ > 100)
{
  sdadcDebug33 = 0;
  sdadcDebug32 = 0;
}
	return;
}

