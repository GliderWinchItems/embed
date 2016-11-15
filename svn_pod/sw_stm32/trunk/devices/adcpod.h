/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : adcpod.h
* Hackeroos          : deh
* Date First Issued  : 07/06/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : ADC routines for pod
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ADCPOD
#define __ADCPOD

/* Includes ------------------------------------------------------------------*/
#include "libopenstm32/common.h"	// Has things like 'u16' defined
#include "libopenstm32/gpio.h"
#include "libopenstm32/spi.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/rtc.h"

#define NUMBERADCCHANNELS	9	// Number of ADC channels read in a sequence

#define ADCSAVESIZE	4	// ADC averaging size



/* Note: The lower four bits of the priority byte are not used.
 The higher the priority number, the lower the priority */
#define DMA1_CH1_PRIORITY	0x30	// Interrupt priority for DMA1 Channel1 interrupt

/* ADC usage on POD board--This is also the order that the adc conversions fill the struct
0 PA 3 ADC123-IN3	Thermistor on 32 KHz xtal
1 PB 0 ADC12 -IN8	Bottom cell of battery
2 PB 1 ADC12 -IN9	Top cell of battery 
3 PC 0 ADC12 -IN10	Accelerometer X	
4 PC 1 ADC12 -IN11	Accelerometer Y
5 PC 2 ADC12 -IN12	Accelerometer Z	
6 PC 3 ADC12 -IN13	Op-amp
7 -- - ADC1  -IN16	Internal temp ref
8 -- - ADC1  -IN17	Internal voltage ref (Vrefint)
*/
struct ADCDR
{
	int in[2][NUMBERADCCHANNELS];	// ADC_DR is stored for each channel in the regular sequence
	unsigned int cnt;		// Sequence complete counter
	unsigned short flg;		// Index of buffer that is not busy (0, or 1)
};

/******************************************************************************/
void adc_init_sequence(volatile struct ADCDR *strADC1dr);
/* @brief 	: Call this routine to do a timed sequencing of power up and calibration
*******************************************************************************/
unsigned int adc_init(volatile struct ADCDR *pstrADC1dr);
/* @brief 	: Initialize adc for dma channel 1 transfer
 * @param	: struct with int array to receive data, plus busy flag
 * @return	: SYSTICK count for end of CR2_ADON delay
*******************************************************************************/
unsigned int adc_start_conversion(volatile struct ADCDR *strADC1dr);
/* @brief 	: Start a conversion of the regular sequence
 * @param	: struct with int array to receive data, plus busy flag
 * @return	: Current 32 bit SYSTICK count
*******************************************************************************/
unsigned char adc_busy(void);
/* @brief 	: Check for busy
 * @param	: struct with int array to receive data, plus busy flag
 * @return	: Flag: 0 = not busy, 1 = busy.
*******************************************************************************/
unsigned int adc_regulator_turn_on(void);
/* @brief 	: Turn on 3.2v regulator for ADC and get time
 * @return	: 32 bit SYSTICK count of end of delay duration
*******************************************************************************/
unsigned int adc_battery_sws_turn_on(void);
/* @brief 	: Turn on switches to battery cells and get time
 * @return	: 32 bit SYSTICK count of end of delay duration
*******************************************************************************/
unsigned int adc_start_cal_register_reset(void);
/* @brief 	: Start calibration register reset
 * @return	: Current 32 bit SYSTICK count
*******************************************************************************/
unsigned int adc_start_calibration(void);
/* @brief 	: Start calibration
 * @return	: Current 32 bit SYSTICK count
*******************************************************************************/
unsigned int adc_battery_sws_turn_on(void);
/* @brief 	: Turn on switches to battery cells and get time
 * @return	: 32 bit SYSTICK count of end of delay duration
*******************************************************************************/

/* ADC readings buffer */
extern volatile struct ADCDR	strADC1dr;
extern volatile struct ADCDR*   strADC1resultptr;		// Pointer to struct holding adc data stored by DMA
extern volatile struct ADCDR	strADCsave[ADCSAVESIZE];

#endif 
