/******************************************************************************
* File Name          : p1_initialization_vcal.c
* Date First Issued  : 10/04/2015
* Board              : STM32F103VxT6_pod_mm (USART1)
* Description        : Initialization for AD7799 voltage calibror using POD board
*******************************************************************************/
/*

*/

/* The following are in 'svn_pod/sw_stm32/trunk/lib' */
#include "libopenstm32/systick.h"
#include "libmiscstm32/systick1.h"

//#include "libmiscstm32/printf.h"// Tiny printf

#include "libmiscstm32/clockspecifysetup.h"

#include "libopenstm32/rcc.h"
#include "libopenstm32/gpio.h"
#include "libopenstm32/usart.h"
#include "libopenstm32/bkp.h"	// Used for #defines of backup register addresses
#include "libopenstm32/systick.h"
#include "libopenstm32/bkp.h"	// #defines for addresses of backup registers
#include "libopenstm32/timer.h"	// Used for #defines of backup register addresses
#include "PODpinconfig.h"
#include "spi1ad7799_vcal.h"

#include "ad7799_vcal_comm.h"
#include "ad7799_vcal_filter.h"
#include "adcsensor_vcal.h"
#include "tim3_ten.h"
#include "spi1ad7799_vcal.h"

/* Subroutine prototyes used only in this file */
void complete_ad7799(void);
/* ----------------------------------------------------------------------------- 
 * void timedelay (u32 ticks);
 * ticks = processor ticks to wait, using DTW_CYCCNT (32b) tick counter
 ------------------------------------------------------------------------------- */
static void timedelay (unsigned int ticks)
{
	u32 t1 = ticks + *(volatile unsigned int *)0xE0001004; // DWT_CYCNT
	WAITDTW(t1);
	return;
}
/******************************************************************************
 * int p1_initialization_vcal(void);
 * @brief 	: Initialize everything needed for all operationg modes
 * @return	: 0 = OK; not zero = error
*******************************************************************************/
int p1_initialization_vcal(void)
{
	int ret = 0;
	unsigned int one_ms = sysclk_freq/1000;

	/* Turn on some POD board power switches. */
//	TOPCELLADC_on		// PC4 Battery top cell sense switch

//	BOTTOMCELLADC_on	// PC5 Battery bottom cell sense switch

	ENCODERGPSPWR_on;	// Turn on 5.0v encoder regulator hacked to CAN driver

	ANALOGREG_on		// PE7	3.2v Regulator enable, Analog: gpio
	timedelay(one_ms * 5);

	STRAINGAUGEPWR_on;	// 5.0v strain gauge regulator enable
	timedelay(one_ms * 5);

	ADC7799VCCSW_on;	// 3.3v digital switch 
	timedelay(one_ms * 5);

	SDCARDREG_on;		// 3.3v SD card regulator enable

	timedelay(one_ms * 500);// Wait for things to settle down.

	/* Now that the ad7799 power has stabilized, continue initializations. */
	ret |= spi1ad7799_vcal_init();	// 

	/* Poll AD7799 for ready at 2048/sec using this timer. */
//	tim3_ten_init(pclk1_freq/2048);

	return ret;
}

/******************************************************************************/
/* Complete AD7799 setup                                                      */
/**************************************************************************** */

#ifdef USEcomplete_ad7799	// Skip, but don't throw this code away just yet.

void complete_ad7799(void)
{
	ad7799_vcal_reset();

/* The following code was lifted out of (someplace...) */
// The commented out code was useful for debugging ad7799 initialization
//	unsigned int ui;

	AD7799_RD_ID_REG;		// Read ID register (macro in ad7799_comm.h)
	while (ad7799_vcal_comm_busy != 0);	// Wait for write/read sequence to complete
//	ui = ad7799_8bit_reg;
//	printf ("ID    :  %6x \n\r",ui);	USART1_txint_send();	// Start the line buffer sending

	/* mode: continuous 470sps | channel 2, buffer amp 'in' | gain = 0 |reference detector on  */
	ad7799_vcal_1_initA(AD7799_CONT | AD7799_470SPS, AD7799_CH_2|AD7799_1NO|AD7799_BUF_IN|AD7799_REF_DET);
	while (ad7799_vcal_comm_busy != 0);	// Wait for write/read sequence to complete

	AD7799_RD_CONFIGURATION_REG;	// Read configuration register (macro in ad7799_comm.h)
	while (ad7799_vcal_comm_busy != 0);	// Wait for write/read sequence to complete
//	ui = ad7799_16bit.us;
//	printf ("Config:%8x \n\r",ui); USART1_txint_send();

	AD7799_RD_MODE_REG;		// Read mode register (macro in ad7799_comm.h)
	while (ad7799_vcal_comm_busy != 0);	// Wait for write/read sequence to complete
//	ui = ad7799_16bit.us;
//	printf ("Mode  :%8x \n\r",ui); USART1_txint_send();

	AD7799_RD_OFFSET_REG;		// Read offset register (macro in ad7799_comm.h)
	while (ad7799_vcal_comm_busy != 0);	// Wait for write/read sequence to complete
//	printf ("Offset:%8x \n\r",ad7799_24bit.n); USART1_txint_send();

	AD7799_RD_FULLSCALE_REG;	// Read full scale register (macro in ad7799_comm.h)
	while (ad7799_vcal_comm_busy != 0);	// Wait for write/read sequence to complete
//	printf ("FScale:%8x \n\r",ad7799_24bit.n); USART1_txint_send();

	ad7799_vcal_wr_mode_reg(AD7799_CONT | AD7799_470SPS );	// Continuous conversion (see ad7799_comm.h)

	

	return;
}
#endif


