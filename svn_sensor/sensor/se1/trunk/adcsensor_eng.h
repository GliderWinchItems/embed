/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : adcsensor_eng.h
* Author             : deh
* Date First Issued  : 06/24/2013
* Board              : RxT6
* Description        : ADC routines for f103 sensor--engine configuration
*******************************************************************************/
/*


*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ADCSENSOR_ENG
#define __ADCSENSOR_ENG

/* Includes ------------------------------------------------------------------*/
#include "libopenstm32/common.h"	// Has things like 'u16' defined
#include "libopenstm32/gpio.h"
#include "libopenstm32/spi.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/rtc.h"

#define NUMBERADCCHANNELS_SE	3	// Number of channels each ADC is to scan
#define NUMBERSEQUENCES		1	// Number of sequences in 1/2 of the buffer
#define DECIMATION_SE		32	// Decimation ratio
#define DISCARD_SE		32	// Number of readings to discard before filtering starts
#define CICSCALE		18	// Right shift count to scale result

/* ADC usage
PA0 ADC123-IN0	Throttle potentiometer
PA1 ADC123-IN1  Thermistor
PA3 ADC123-IN2	Pressure sensor  
*/

struct ADCDR_ENG
{
	int in[2][NUMBERSEQUENCES][NUMBERADCCHANNELS_SE];	// ADC_DR is stored for each channel in the regular sequence
	unsigned int cnt;		// DMA interrupt counter
	unsigned short flg;		// Index of buffer that is not busy (0, or 1)
};


/******************************************************************************/
void adc_init_se_eng_sequence(u32 iamunitnumber);
/* @brief 	: Call this routine to do a timed sequencing of power up and calibration
*******************************************************************************/

/* ADC readings buffer */
extern struct ADCDR_ENG    strADC1dr;
extern struct ADCDR_ENG*   strADC1resultptr;		// Pointer to struct holding adc data stored by DMA

/* Pointer to functions to be executed under a low priority interrupt, forced by DMA interrupt. */
// These hold the address of the function that will be called
extern void 	(*dma_ll_ptr)(void);		// DMA -> FSMC  (low priority)
extern long	adc_last_filtered[NUMBERADCCHANNELS_SE];	// Last computed value for each channel
extern u32 cic_sync_err[NUMBERADCCHANNELS_SE];	// CIC sync errors

extern long	adc_temperature;		// Thermistor filter/decimate to 2/sec
extern int	adc_temp_flag;			// Signal main new filtered reading ready
extern int	adc_calib_temp;			// Temperature in deg C 



extern void 	(*systickHIpriority3_ptr)(void);	// SYSTICK handler (very high priority)
extern void 	(*systickLOpriority3_ptr)(void);	// SYSTICK handler (low high priority) continuation--1/2048th
extern void 	(*systickLOpriority3X_ptr)(void);	// SYSTICK handler (low high priority) continuation--1/64th

extern unsigned int cicdebug0,cicdebug1;

#endif 
