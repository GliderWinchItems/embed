/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : adctherm_cal.h
* Hackee             : deh
* Date First Issued  : 08/10/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Convert thermistor adc reading to degree C * 100
*******************************************************************************/
/*

The table only covers the useful range of temperatures.  Hence, the first entry of the
table does not correspond to a zero adc reading.  The offset value and size of the table
is generated by the table generating program.

When the ADC reading exceeds the table limits the highest or lowest converted value
is returned.

*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ADCTHERM_CAL
#define __ADCTHERM_CAL

/******************************************************************************/
unsigned short adctherm_cal (unsigned short usAdc);
/* @brief	: Convert adc reading to degree C * 100
 * @param	: usAdc: 12b adc reading
 * @param	: sTmpoff: temp calibration offset
 * @return	: degree C * 100
*******************************************************************************/



#endif 
