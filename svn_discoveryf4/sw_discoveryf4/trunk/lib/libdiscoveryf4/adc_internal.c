/******************************************************************************
* File Name          : adc_internal.c
* Date First Issued  : 08/09/2014
* Board              : Discovery F4
* Description        : ADC routines for Vrefint and Temp
*******************************************************************************/
/*
2.95v = 1685 @ T 1087
3.30v 1685 




*/

#include "libopencm3/cm3/common.h"
#include "adc_internal.h"

int vrefint_tco = 30;	// Temp coefficient in ppm per deg C

/******************************************************************************
 * unsigned int adc_internal_vref(unsigned int chan17);
 * @brief 	: Adjust voltage reference reading for temperature
 * @param	: ADC reading of chan 16 (Vrefint)
 * @return	: Vdda voltage
*******************************************************************************/
unsigned int adc_internal_vref(unsigned int chan17)
{
	return (VREFIN_CAL * 3300)/chan17;
}


