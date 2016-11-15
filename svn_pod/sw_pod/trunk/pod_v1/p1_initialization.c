/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : p1_initialization.c
* Hackeroos          : caw, deh
* Date First Issued  : 08/30/2011
* Board              : STM32F103VxT6_pod_mm (USART1)
* Description        : Main program for version implementation
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

USART1, UART4 initialization is handled in the 'normal_run' since it
is dependent on the pushbutton caused wakeup.

Subroutine call references shown as "@n"--
@1  = svn_pod/sw_stm32/trunk/devices/Podpinconfig.c
@2  = svn_pod/sw_stm32/trunk/lib/libopenstm32/gpio.h
@3  = svn_pod/sw_stm32/trunk/lib/libmiscstm32/SYSTICK_getcount32.c
@4  = svn_pod/sw_stm32/trunk/devices/adcpod.c
@5  = svn_pod/sw_stm32/trunk/lib/libusartstm32/USART1_txint_putc.c
@6  = svn_pod/sw_stm32/trunk/devices/spi1ad7799.h
@7  = svn_pod/sw_stm32/trunk/libsupportstm32/sdlog.h
@8  = svn_pod/sw_stm32/trunk/devices/Tim2_pod.c
@9  = svn_pod/sw_pod/trunk/pod_v1/adc_packetize.c
@10 = svn_pod/sw_pod/trunk/pod_v1/ad7799_packetize.c
@11 = svn_pod/sw_stm32/trunk/ad7799test/ad7799test.c
@12 = svn_pod/sw_pod/trunk/pod_v1/p1_common.c
@13 = svn_pod/sw_pod/trunk/pod_v1/p1_calibration_sd.c

*/

/* The following are in 'svn_pod/sw_stm32/trunk/lib' */
#include "libopenstm32/systick.h"
#include "libmiscstm32/systick1.h"
#include "libmiscstm32/printf.h"// Tiny printf
#include "libmiscstm32/clockspecifysetup.h"

#include "libopenstm32/rcc.h"
#include "libopenstm32/gpio.h"
#include "libopenstm32/usart.h"
#include "libopenstm32/bkp.h"	// Used for #defines of backup register addresses
#include "libopenstm32/systick.h"
#include "libopenstm32/bkp.h"	// #defines for addresses of backup registers
#include "libopenstm32/timer.h"	// Used for #defines of backup register addresses

#include "pod_v1.h"		// Tiny main routine
#include "p1_common.h"		// Variables in common

/* Subroutine prototyes used only in this file */
void complete_adc(void);
void complete_ad7799(void);

/* (@2) 'struct CLOCKS clocks' is used to setup the clock source, PLL, dividers, and bus clocks 
See P 84 of Ref Manual for a useful diagram.
../lib/libmiscstm32/clockspecifysetup.h has the 'enum' values that may help for making mistakes(!) */
/* NOTE: Bus for ADC (APB2) must not exceed 14 MHz */
const struct CLOCKS clocks = { \
HSOSELECT_HSE_XTAL,	/* Select high speed osc 			*/ \
PLLMUL_6X,		/* Multiplier PLL: 0 = not used 		*/ \
1,			/* Source for PLLMUL: 0 = HSI, 1 = PLLXTPRE (1 bit predivider)	*/ \
0,			/* PLLXTPRE source: 0 = HSE, 1 = HSE/2 (1 bit predivider on/off)	*/ \
APBX_2,			/* APB1 clock = SYSCLK divided by 1,2,4,8,16; freq <= 36 MHz */ \
APBX_4,			/* APB2 prescalar code = SYSCLK divided by 1,2,4,8,16; freq <= 72 MHz */ \
AHB_1,			/* AHB prescalar code: SYSCLK/[2,4,8,16,32,64,128,256,512] (drives APB1,2) */ \
8000000			/* Oscillator source frequency, e.g. 8000000 for an 8 MHz xtal on the external osc. */ \
};

/* Subroutine proto's */
void putc ( void* p, char c);	// For tiny printf (in case we use it!  Which it turns out we use copiously.)

/******************************************************************************
 * void p1_initialization_basic(void);
 * @brief 	: Initialize everything needed for all operationg modes
*******************************************************************************/
void p1_initialization_basic(void)
{
	clockspecifysetup((struct CLOCKS *)&clocks);		// Get the system clock and bus clocks running

	PODgpiopins_default();	// Set gpio port register bits for low power (@1)
	PODgpiopins_Config();	// Now, configure pins (@1)

	/* Configure pin that senses GPS ON/OFF switch for input, pull-up */
	configure_pin_in_pd( GPSONOFF_CONFIG, GPSONOFF_BIT, 1);	// (@12)

	/* If we are coming out of STANDBY PA0 was setup for wakeup.  Undo it */
	RCC_APB1ENR |= RCC_APB1ENR_PWREN;
	MMIO32_BIT_BAND(&PWR_CSR,8) = 0;

	/* Check pushbutton to see if that caused the wake up */
	PA0_reconfig(1);		// Configure PA0 for input
	cPA0_reset = GPIO_IDR(GPIOA) & 0x01;	// Get pushbutton status (@2)


	/* This jic we use it...might be useful for debugging */
	init_printf(0,putc);	// This one-time initialization is needed by the tiny printf routine

	/* Systick is used for the some of the initialization timing */
	SYSTICK_init(0);	// Set SYSTICK for interrupting and to max count (24 bit counter) (@3)

	/* Get some switches turned on so that power stabilizes while we continue the setup */
	sys.t0 = adc_battery_sws_turn_on();		// Turn on time for switches to connect resistor dividers to battery (@4)
	sys.t1 = adc_regulator_turn_on();		// Turn on time for ADC 3.2v regulator and get time (systicks) (@4)

	/* If the pushbutton is on, turn the switch on to get the power up of the RS 232 converter started */
	if (cPA0_reset > 0)
	{
		RS232_ctl();	// Start sequence for bringing up the MAX232 
	}

/* NOTE: if the MAX3232 has not been turned on, USART1 chars will not be seen on a PC. 
   The 'cRS232_skip_flag' is used in the following code to skip the initialization in 'RS232_ctl.c'
   The code in 'RS232_ctl.c' is retained for the in case I change my mind (!) */	
	cRS232_skip_flag = 1;	// Set flag. Only do initialization once! (@5)
	/* Initialize USART and setup control blocks and pointer */
	USART1_rxinttxint_initRTC(115200,96,2,48,4);	//  (@5)
	/* Initialize UART4 (GPS) */
	UART4_rxinttxint_initRTC(57600,96,2,48,4);	//  (@5)
	/* Announce who we are */
	// $Revision: 606 $ This is updated automatically by SVN 
	USART1_txint_puts("\n\rPOD2 $Revision: 606 $\n\r"); USART1_txint_send();
	/* At this point the machine is somewhat(!) functional */	

	/* Setup default calibration values */
/* Note: This sets up the default calibrations.  When in deepsleep mode we don't take the time
(power) to initialize and setup the SD card, so we will depend on the ADC calibrations for the
temperature and battery voltages to be close enough */
	calibration_init_default(&strDefaultCalib);	// (@12)

	return;
}
/******************************************************************************
 * void p1_initialization_active(void);
 * @brief 	: Complete initialization required for active mode
*******************************************************************************/
void p1_initialization_active(void)
{
	STRAINGAUGEPWR_on;	// 5.0v strain gauge regulator (@1)
	ADC7799VCCSW_on;	// 3.3v digital switch (@1)
	SDCARDREG_on;		// 3.3v SD card regulator (@1)

	/* The following time needs to elapse before the adc and ad7799 power is stablized (about 500 ms) */
	sys.t1 = SYSTICK_getcount32() - (POWERUPDELAY1*(sysclk_freq/10000));	// Get time for this mess  (@3)

	/* Set up the ID's in the packet buffers */
	adc_packetize_init();	// (@9)
	ad7799_packetize_init();// (@10)

	/*  Setup TIM2 so that the GPS 1_PPS on PA1 causes time stamps */
	p1_Tim2_pod_init();	// Setup TIM2CH1  driven by 1_PPS via PA1 (spare RJ-45 pin) (@8)

	/* Setup TIM1 to measure processor clock using GPS 1_PPS */
	p1_GPS_1pps_init();	// Setup of the timers and pins for TIM1CH1* (* = remapped) driven from RTC

	/* Wait for the delay time (500 ms) time to expire for SD Card to power up and adc related power to stabilize */
	while ( ( (int)(SYSTICK_getcount32() - sys.t1) ) > 0 );	// Wait for regulator voltage to settle (@3)

	/* Now that the ad7799 power has stabilized, initialize SPI1 AD7799_1 */
	spi1ad7799_init();	// (@6)

	/* This is synchronous and will take time while it searches the last written packet location. */	
	sdlog_init();		// Initialize the SD Card. (@7)

	/* Get the calibrations (which might be in the SD card */
	p1_calibration_retrieve();	// (@12, @13)

	/* Get ADC initialized and calibrated */
	adc_init_sequence(&strADC1dr);	// Initialization sequence for the adc (@4)

	/* ADC completion */
	complete_adc();	// Time delay + calibration, then start conversions

	/* Setup: Zero calibration, sampling rate, and continuous conversions */
	complete_ad7799();

/* 
At this point the following two lines have been executed ('p1_get_reset_mode()'), setting of the 32 KHz 
interrupt subroutines chaining--
	rtc_secf_ptr  = &rtctimers_countdown;	// RTC_ISRHandler goes to 'rtctimers_countdown'
	rtc_timerdone_ptr = &rtc_tickadjust;	// rtc_timers goes to 'rtc_tickadjust' 
Now, setup the remainder for polling the AD7799 and the filtering/packetizing of those readings as well as 
the ADC data for accelerometer, battery voltage, and thermister readings.

Waiting for a rtc "tick" sychronizes the setting of the subroutine addresses so that an rtc interrupt doesn't
causes consternation of other attitudinal side effects.

This will setup the following sequence--
32KHz secf interrupt (every 4 32768 KHz osc ticks) counts down a count-down-by-four counter.  When this counter
expires it sets a TIM6 interrupt pending.  The TIM6 (very low priority) interrupt servicing then calls--
	'rtctimers_countdown'	Counts down (three) timer counters, which calls--
	'rtc_tickadjust'	Does adjustment to the count-down-by-four counter (in 32KHz.c) to time adjustment, which calls--
	'ad7799_poll_rdy'	Checks AD7799 for data ready, and filters data, which calls--
	'ad7799_packetize'	Packetizes the AD7799 data, which calls--
	'adc_packetize'		Filters ADC data, which 'returns' back up through the chain to the TIM6 isr handler.

*/

/* This is the timer polling chain in reverse order */
	adc_filterdone_ptr           = &rtc_tickadjust;		// adc_filterdone_ptr -> adc_packetize
	rtc_ad7799_packetizedone_ptr = &adc_packetize;		// rtc_ad7799_filterdone_ptr -> adc_packetize
	ad7799_filterdone_ptr        = &ad7799_packetize_add;	// ad7799_poll_rdy -> ad7799_filterdone_ptr
	rtc_timerdone_ptr            = &ad7799_poll_rdy;	// tickadjust -> ad7799_poll_rdy
//	rtc_secf_ptr                 = &rtctimers_countdown;	// TIM6 isr -> rtctimers_countdown
	


	return;
}
/******************************************************************************
 * void p1_initialization_deepsleep(void);
 * @brief 	: Complete initialization required for deepsleep mode
*******************************************************************************/
void p1_initialization_deepsleep(void)
{
	/* Get ADC initialized and calibrated */
	adc_init_sequence(&strADC1dr);	// Initialization sequence for the adc (@4)

	/* ADC completion */
	complete_adc();	// Time delay + calibration, then start conversions	

/* This is the timer (TIM6) polling chain in reverse order */
	rtc_tickadjustdone_ptr       = &adc_packetize;		// tickadjust -> adc_packetize
	rtc_timerdone_ptr            = &rtc_tickadjust;		// rtctimers_countdown -> tickadjust
//	rtc_secf_ptr                 = &rtctimers_countdown;	// TIM6 isr -> rtctimers_countdown
	

	return;
}
/******************************************************************************/
/* This is for the tiny printf ...jic we use it */
// Note: the compiler will give a warning about conflicting types
//       for the built in function 'putc'.
/******************************************************************************/
void putc ( void* p, char c)
	{
		p=p;	// Get rid of the unused variable compiler warning
		USART1_txint_putc(c);	// (@5)
	}
/******************************************************************************/
/* Complete ADC setup                                                         */
/******************************************************************************/
void complete_adc(void)
{
	unsigned int temp;

	/* This assumes the caller has waited for the adc regulator to stabilize */	

	/* Get ADC initialized and calibrated */
	adc_init_sequence(&strADC1dr);	// Initialization sequence for the adc (@4)

	/* Start the reset process for the calibration registers */
	temp = adc_start_cal_register_reset() - (9*(sysclk_freq/1000000)); // 9 * ticks per microsecond
	while ( ((int)(SYSTICK_getcount32() - temp)) > 0  );	// Wait for time to elapse

	/* Start the register calibration process */
	adc_start_calibration();		// Start calibration 
	temp = adc_start_calibration() - (80*(sysclk_freq/1000000)); // 80 * ticks per microsecond
	while ( ((int)(SYSTICK_getcount32() - temp)) > 0  );	// Wait for time to elapse

	/* Check that enough time has elapsed since it was saved in 'p1_initialization_basic()'  */
	while ( ((int)(SYSTICK_getcount32() - sys.t0)) > 0  );	// Wait for resistive divider to settle (@3)

	/* ADC is now ready for use.  Start ADC conversions */
	adc_start_conversion((volatile struct ADCDR*)&strADC1dr);	// Start ADC conversions into struct via DMA (@4)

	return;
}
/******************************************************************************/
/* Complete AD7799 setup                                                      */
/******************************************************************************/
void complete_ad7799(void)
{
	ad7799_reset ();

/* The following code was lifted out of (@11) */
// The commented out code was useful for debugging ad7799 initialization
//	unsigned int ui;

	AD7799_RD_ID_REG;		// Read ID register (macro in ad7799_comm.h)
	while (ad7799_comm_busy != 0);	// Wait for write/read sequence to complete
//	ui = ad7799_8bit_reg;
//	printf ("ID    :  %6x \n\r",ui);	USART1_txint_send();	// Start the line buffer sending

	/* Setup: Zero calibration, sampling rate, and continuous conversions */
	ad7799_1_initA(AD7799_4p17SPS);	// Setup for strain gauge, (Arg = sampling rate)
	while (ad7799_comm_busy != 0);	// Wait for write/read sequence to complete

	AD7799_RD_CONFIGURATION_REG;	// Read configuration register (macro in ad7799_comm.h)
	while (ad7799_comm_busy != 0);	// Wait for write/read sequence to complete
//	ui = ad7799_16bit.us;
//	printf ("Config:%8x \n\r",ui); USART1_txint_send();

	AD7799_RD_MODE_REG;		// Read mode register (macro in ad7799_comm.h)
	while (ad7799_comm_busy != 0);	// Wait for write/read sequence to complete
//	ui = ad7799_16bit.us;
//	printf ("Mode  :%8x \n\r",ui); USART1_txint_send();

	AD7799_RD_OFFSET_REG;		// Read offset register (macro in ad7799_comm.h)
	while (ad7799_comm_busy != 0);	// Wait for write/read sequence to complete
//	printf ("Offset:%8x \n\r",ad7799_24bit.n); USART1_txint_send();

	AD7799_RD_FULLSCALE_REG;	// Read full scale register (macro in ad7799_comm.h)
	while (ad7799_comm_busy != 0);	// Wait for write/read sequence to complete
//	printf ("FScale:%8x \n\r",ad7799_24bit.n); USART1_txint_send();

	ad7799_wr_mode_reg(AD7799_CONT | AD7799_470SPS );	// Continuous conversion (see ad7799_comm.h)

	return;
}

