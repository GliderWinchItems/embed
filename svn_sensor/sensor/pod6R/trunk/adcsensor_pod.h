/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : adcsensor_pod.h
* Hackeroos          : deh
* Date First Issued  : 11/11/2012
* Board              : STM32F103VxT6_pod_mm
* Description        : ADC routines for f103 sensor--pod board
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
#ifndef __ADCSENSOR_POD
#define __ADCSENSOR_POD

/* Includes ------------------------------------------------------------------*/
#include "libopenstm32/common.h"	// Has things like 'u16' defined
#include "libopenstm32/gpio.h"
#include "libopenstm32/spi.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/rtc.h"

#define NUMBERADCCHANNELS_SE	1		// Number of channels each ADC is to scan

/* These define the initial values for the ADC watchdog registers */
#define ADC1_HTR_INITIAL	550		// High threshold registor setting, ADC1
#define ADC1_LTR_INITIAL	300		// Low  threshold register setting, ADC1
#define ADC2_HTR_INITIAL	1100		// High threshold registor setting, ADC2
#define ADC2_LTR_INITIAL	800		// Low  threshold register setting, ADC2


/* Note: The lower four bits of the priority byte are not used.
 The higher the priority number, the lower the priority */
#define ADC1_2_PRIORITY	0x40	// Interrupt priority for ADC1 or ADC2 interrupt

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
void adc_init_sequence_se(void);
/* @brief 	: Call this routine to do a timed sequencing of power up and calibration
*******************************************************************************/
void adc_regulator_turn_on_se(void);
/* @brief 	: Turn on 3.2v regulator
*******************************************************************************/

/* Counters of great interest to the general public and the occasional programmer. */
extern volatile s32 encoder_ctr;	// Shaft encoder running count (+/-)
extern volatile  u32 encoder_error;	// Cumulative count--shaft encoding errors
extern volatile  u32 encoder_Actr;	// Cumulative count--photocell A
extern volatile  u32 encoder_Bctr;	// Cumulative count--photocell B
extern volatile  u32 adc_encode_time;	// DTW_CYCCNT time of last transition
extern u32 adc_encode_time_prev;		// Previous time at SYSTICK
extern s32 encoder_ctr_prev;			// Previous encoder running count


#endif 
