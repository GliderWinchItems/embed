/******************************************************************************
* File Name          : p1_initialization.c
* Date First Issued  : 07/03/2015
* Board              : STM32F103VxT6_pod_mm (USART1)
* Description        : Initialization for winch tension with POD board
*******************************************************************************/
/*
08-30-2011

The initialization takes place in two steps.  First there is a basic
initialization required in all cases.  This main routine then calls
'p1_get_reset_mode' which does some RTC initialization which in turn
determines the type of reset.  For a "normal" reset (where the unit
comes out of STANDBY) additional initialization is required to get
the SD Card and AD7799 working.  For "deepsleep" mode cleaning up the
adc initialization is all that is needed.


*/

/* The following are in 'svn_pod/sw_stm32/trunk/lib' */
#include <stdio.h>
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
#include "tim3_ten.h"
#include "ad7799_ten_comm.h"
#include "ad7799_filter_ten.h"
#include "adcsensor_tension.h"
#include "spi1sad7799_ten.h"
#include "DTW_counter.h"
#include "panic_leds_pod.h"

unsigned char AD7799_num;

/* Subroutine prototyes used only in this file */
 static int complete_ad7799(int n);

/******************************************************************************
 * static int wait_busy_ad7799(unsigned int ticks);
 * @param	: ticks = number of usec to wait for busy to go to zero
 * @return	: >0 for OK; =<0 timed out
*******************************************************************************/
static int wait_busy_ad7799(unsigned int ticks)
{
	unsigned int t0 = DTWTIME + ticks * (sysclk_freq/1000000);
	while (  (ad7799_ten_comm_busy != 0) && (((int)DTWTIME - (int)t0) > 0) );
	return ((int)DTWTIME - (int)t0);
}
/******************************************************************************
 * static void timedelay_usec (unsigned int usec);
 * ticks = processor ticks to wait, using DTW_CYCCNT (32b) tick counter
 *******************************************************************************/
static void timedelay_usec (unsigned int usec)
{
	unsigned int ticks = sysclk_freq/1000000; // Sysclk ticks per usec
	u32 t1 = (ticks * usec) + DTWTIME;	// Time to quit looping	
	WAITDTW(t1);				// Loop
}
/******************************************************************************
 * void p1_initialization(void);
 * @brief 	: Initialize everything needed for all operationg modes
*******************************************************************************/
void p1_initialization(void)
{
	int ret;	// Return code from function call

	/* Get some switches turned on */
	TOPCELLADC_on		// PC4 Battery top cell switch

	BOTTOMCELLADC_on	// PC5 Battery bottom cell switch

	ENCODERGPSPWR_on;	// Turn on 5.0v encoder regulator hacked to CAN driver

	ANALOGREG_on		// PE7	3.2v Regulator EN, Analog: gpio
	timedelay_usec(5000);	// Time delay (ms)

	STRAINGAUGEPWR_on;	// 5.0v strain gauge regulator 
	timedelay_usec(5000);

	ADC7799VCCSW_on;	// 3.3v digital switch 
	timedelay_usec(5000);

	SDCARDREG_on;		// 3.3v SD card regulator 
	timedelay_usec(500000);

	/* Now that the ad7799 power has stabilized, initialize SPI1for AD7799 use. */
	ret = spi1ad7799_ten_init();	// 
	if (ret != 0)
	{
		printf("p1_initialization:spi1ad7799_ten_init() failed: %d\n\r", ret);
		while(1==1);;
	}

	/* Setup: Zero calibration, sampling rate, and continuous conversions */
	// AD7799_1 (should be present)
	// AD7799_2 (may not be present on all pod boards)

printf ("AD7799_1 init registers\n\r");
	ret = complete_ad7799(0);	// AD7799_1 #####
	if (ret != 0)
	{
		printf("AD7799_1 failed to init. HANG LOOP\n\r");
		timedelay_usec(500000);
		panic_leds_pod(6);
	}
	
printf("AD7799_1 setup succeeded\n\r");timedelay_usec(500000);

printf ("AD7799_2 init registers\n\r");
	ret = complete_ad7799(1);	// AD7799_2 #####
	if (ret != 0)
	{
		printf("AD7799_2 failed to init\n\r"); 
		AD7799_num = 1;		// Show that AD7799_1 is only one good
	}
	else
	{	
		AD7799_num = 2;		// Show that AD7799_1 & 2 init'ed OK
		printf("AD7799_2 setup succeeded\n\r");timedelay_usec(500000);
	}

ad7799_1_select();	// /CS set to select #1 and not #2

while (ad7799_ten_comm_busy != 0);

	printf ("Number AD7799 ready: %d\n\r",AD7799_num);

	/* Get SAR ADC initialized and calibrated */
	adcsensor_tension_sequence();	// Initialization sequence for the adc

	/* Timer interrupt callback -> ad7799_poll (in 'ad7799_filter.c'). */
	tim3_ten_ptr = &ad7799_poll_rdy_ten_both;

	/* Poll AD7799 for ready at 2048/sec using this timer. */
	tim3_ten_init(pclk1_freq/2048);

	return;
}

/******************************************************************************
 * static int complete_ad7799(int n);
 * @brief	: Complete AD7799 setup
 * @return	: 0 = OK; -1 = ID register return not 0x4b
 ******************************************************************************/
//Help for degubbing
extern unsigned char ad7799_ten_8bit_reg;		// Byte with latest status register from ad7799_1
extern union SHORTCHAR	ad7799_ten_16bit;
extern union INTCHAR	ad7799_ten_24bit;

static int complete_ad7799(int n)
{
	unsigned int ui;
	int idreg;

	/* Select AD7799_1 or '_2 */
	if (n == 0)
		ad7799_1_select();	// /CS set to select #1 and not #2
	else
		ad7799_2_select();	// /CS set to select #2 and not #1

	wait_busy_ad7799(4);		// Allow time to set up /CS line (usec)
	ad7799_ten_reset();		// Send reset sequence
while (ad7799_ten_comm_busy != 0);

/* The following code was lifted out of some of the POD work. */
// The commented out code was useful for debugging ad7799 initialization
//	unsigned int ui;

	AD7799_RD_ID_REG;		// Read ID register (macro in ad7799_comm.h)
	while (ad7799_ten_comm_busy != 0);	// Wait for write/read sequence to complete
	idreg = ad7799_ten_8bit_reg;
	printf ("ID    :  %6x \n\r",idreg);

	/* Setup: Zero calibration, sampling rate, and continuous conversions */
	ad7799_ten_1_initA(AD7799_4p17SPS);	// Setup for strain gauge, (Arg = sampling rate)
	while (ad7799_ten_comm_busy != 0);	// Wait for write/read sequence to complete

	AD7799_RD_CONFIGURATION_REG;	// Read configuration register (macro in ad7799_comm.h)
	while (ad7799_ten_comm_busy != 0);	// Wait for write/read sequence to complete
	ui = ad7799_ten_16bit.us;
	printf ("Config:%8x \n\r",ui);

	AD7799_RD_MODE_REG;		// Read mode register (macro in ad7799_comm.h)
	while (ad7799_ten_comm_busy != 0);	// Wait for write/read sequence to complete
	ui = ad7799_ten_16bit.us;
	printf ("Mode  :%8x \n\r",ui);

	AD7799_RD_OFFSET_REG;		// Read offset register (macro in ad7799_comm.h)
	while (ad7799_ten_comm_busy != 0);	// Wait for write/read sequence to complete
	printf ("Offset:%8x \n\r",ad7799_ten_24bit.n);

	AD7799_RD_FULLSCALE_REG;	// Read full scale register (macro in ad7799_comm.h)
	while (ad7799_ten_comm_busy != 0);	// Wait for write/read sequence to complete
	printf ("FScale:%8x \n\r",ad7799_ten_24bit.n);

	ad7799_ten_wr_mode_reg(AD7799_CONT | AD7799_470SPS );	// Continuous conversion (see ad7799_comm.h)
while (ad7799_ten_comm_busy != 0);

	if (idreg != 0x4b) return -1;	// ID register return failed with AD7799
	return 0;
}

