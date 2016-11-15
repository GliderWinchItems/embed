/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : sensor_threshold.h
* Hackeroos          : deh
* Date First Issued  : 11/15/2012
* Board              : STM32F103VxT6_pod_mm
* Description        : Routines for determining photocell thresholds
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SENSOR_THRESHOLD
#define __SENSOR_THRESHOLD

#include "libopenstm32/adc.h"

/* Histogram parameters */
struct	ADCHISTO
{
	int	nBins;		// Number of bins
	int	nMax;		// Highest ADC reading 
	unsigned int	uiDp;	// Number of data points
	unsigned int *	p;	// Pointer to bin[0];
};

/*******************************************************************************/
void adc_histogram(struct ADCHISTO *pH, u32 pADC);
/* @brief 	: Build histogram from ADC values
 * @param	: pH--pointer to struct with a bunch of things
 * @param	: ADC--ADC1, or ADC2
 * @return	: nothing to speak of
*******************************************************************************/
void adc_histogram_print(struct ADCHISTO *pH);
/* @brief 	: Printout the array
 * @param	: pH--pointer to struct with a bunch of things
 * @return	: nothing to speak of
*******************************************************************************/




#endif 
