/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : adcsensor_foto.h
* Author             : deh
* Date First Issued  : 07/08/2013
* Board              : RxT6
* Description        : ADC routines for f103 sensor--sensor board
*******************************************************************************/
/*
11/11/2012 This is a hack of svn_pod/sw_stm32/trunk/devices/adcpod.h

Strategy--
Two photocell emitters are measured using on ADC1 and ADC2.  The threshold registers
are used to trigger an interrupt when there is a transition between states of the 
sensed photo reflections.


11/15/2012 - POD prototype setup
Emitter resistors--3.3K
Diode resistors--330
ADC voltage applied--3.2v

*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ADCSENSOR_FOTO
#define __ADCSENSOR_FOTO

/* Includes ------------------------------------------------------------------*/
#include "libopenstm32/common.h"	// Has things like 'u16' defined
#include "libopenstm32/gpio.h"
#include "libopenstm32/spi.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/rtc.h"
#include "common_misc.h"

#define NUMBERADCCHANNELS_FOTO	1		// Number of channels each ADC is to scan
#define DISCARD_FOTO		32	// Number of readings to discard before filtering starts
/* Decimation|CICSCALE (for unity scaling) 
    1    3
    2	 6
    4	 9
    8	12
   16	15
   32	18                                                                     */
#define DECIMATION_FOTO		4	// Decimation ratio
#define CICSCALE_FOTO		9	// Right shift count to scale result

#define ADCERRORCTRSIZE	4	// Number of error counters for adcsensor.c

/* These define the initial values for the ADC watchdog registers */
#define ADC1_HTR_INITIAL	1100		// High threshold registor setting, ADC1
#define ADC1_LTR_INITIAL	800		// Low  threshold register setting, ADC1
#define ADC2_HTR_INITIAL	1100		// High threshold registor setting, ADC2
#define ADC2_LTR_INITIAL	700		// Low  threshold register setting, ADC2


/* Note: The lower four bits of the priority byte are not used.
 The higher the priority number, the lower the priority */


/* ADC usage on POD board--This is also the order that the adc conversions fill the struct.
   The '###' channels are the *only* ones used with 'adcsensor_pod.c'.

0 PA 3 ADC123-IN3	Thermistor on 32 KHz xtal
1 PB 0 ADC12 -IN8	Bottom cell of battery
2 PB 1 ADC12 -IN9	Top cell of battery 
3 PC 0 ADC12 -IN10	Accelerometer X	 ### Photocell emitter A
4 PC 1 ADC12 -IN11	Accelerometer Y  ### Photocell emitter B
5 PC 2 ADC12 -IN12	Accelerometer Z	
6 PC 3 ADC12 -IN13	Op-amp
7 -- - ADC1  -IN16	Internal temp ref
8 -- - ADC1  -IN17	Internal voltage ref (Vrefint)
*/

/******************************************************************************/
void adc_init_sequence_foto(u32 iamunitnumber);
/* @brief 	: Call this routine to do a timed sequencing of power up and calibration
 * @param	: iamunitnumber = CAN unit id for this unit, see 'common_can.h'
*******************************************************************************/
void adc_regulator_turn_on_se(void);
/* @brief 	: Turn on 3.2v regulator
*******************************************************************************/

extern long speed_filteredA;		// Most recent computed & filtered rpm
extern u32 encoder_ctrA;		// Most recent encoder count
extern u32 adc_encode_time_prev;	// Previous time at SYSTICK
extern s32 encoder_ctr_prev;		// Previous encoder running count

/* Error counters */
extern u32 adcsensor_foto_err[ADCERRORCTRSIZE];	// Error counters

#endif 
