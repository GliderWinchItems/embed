/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : fotocell.c
* Hackerees          : deh
* Date First Issued  : 10/04/2012
* Board              : Sensor - f373 (STM32F373CxT6)
* Description        : Photocell/comparator/adc/timer setup
*******************************************************************************/
/* Refer to the following for relief--
p279 of Ref Manual (RM0313) Comparator section
../svn_sensor/hw/trunk/eagle/f373/README.pins
../svn_sensor/hw/trunk/eagle/f373/README.functions
*/

/* Pin configuration (see README.pins)

Photocell A -> PA1 -> COMP1_INp
DAC1_OUT1 -> COMP1_INn
COMP1_OUT -> PA6/TIM3_CH1


Photocell B -> PA3 -> COMP2_INp
DAC1_OUT2 -> COMP2_INn
COMP2_OUT -> PA7/TIM3_CH2


*/


#include "cm3/f4/libdevicesf4/gpio.h"

/******************************************************************************
 * void fotocell (void);
 * @brief 	: 
 * @param	: 
 * @return	: 
*******************************************************************************/
void fotocell (void)
{
	unsigned int temp;

/* Assume the GPIO clocks were enabled in the clocksetup routine */

	/*  Setup ADC pins for ANALOG INPUT p 132, 139 */
	//  PA1 (ADC_IN1), PA3 (ADC_IN3) - Input from photocells A & B emitter resistor.
	//  PA4 (ADC_IN4), PA5 (ADC_IN5) - Input from DAC1 OUT1,2
	GPIO_MODER(GPIOA) |=  ( ((0x03 ) << (2*1)) |
				((0x03 ) << (2*3)) |
				((0x03 ) << (2*4)) |
				((0x03 ) << (2*5))  );	// Analog input: MODER = 0b11

	RCC_APB1ENR |= (1<<29);		// DAC1 clocking enable



	/* Alternate function p 144 */
	GPIOA_AFRL &=  ~(  (0xff << (4*6)) |	/* Clear AF field */
			   (0xff << (4*7)) );
	
	GPIOA_AFRL |= ( ( 2 << (4*6)) |		/* Set PA6:AF2 (TIM3_CH1) */
			( 2 << (4*7)) );	/* Set PA7:AF2 (TIM3_CH2) */


	GPIOA_AFRH &=  ~(  ( 15 << (4*6)) |	/* Clear AF field */
			   ( 15 << (4*7)) );
	
	GPIOA_AFRH |= ( ( 8 << (4*6)) |		/* Set PA6:AF8 (COMP1_OUT) */
			( 8 << (4*7)) );	/* Set PA7:AF8 (COMP2_OUT) */

	/* Setup DAC1 p 251, */
	DAC1_CR  |= EN1 bit;

	/* The DAC channel output buffer can be enabled and disabled using the BOFF1 bit in the
	   DAC_CR register. p 254 */
	DAC1_CR &= ~DAC_CR_BOFF2;	// Enable buffer (which is reset default)


	/* Setup timer for encoding A & B photocells p 308 */


=================== OLD f103 code =============================
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

/*#######################################################################################
 * ISR routine for DMA1 Channel1 reading ADC regular sequence of adc channels
 *####################################################################################### */
void DMA1CH1_IRQHandler(void)
{
	return;
}

