/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : adctherm_cal.h
** Hackee             : deh
* Date First Issued  : 11/01/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Compute 32 KHz xtal error due to temperature
*******************************************************************************/
/*
The 32 KHz xtal freq varies with the temperature.  The nominal formula is--
Error (ppm) = (alpha * (T- To)^2);
Where:
T is the temperature in deg C.
To is turnpoint temperature (nominally 25 deg C)
alpha = 0.034 nominal, +/- .006.

The values are scaled, so that--
To of 25 deg C -> 2500.
Likewise T is deg C * 100.
An alpha of .034 -> 3400.

*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ADCPPM_CAL_1
#define __ADCPPM_CAL_1

/******************************************************************************/
unsigned short adcppm_cal_1 (unsigned int uiDegC);
/* @brief	: Compute error of 32 KHz xtal due to temperature
 * @param	: Temperature in deg C * 100
 * @return	: Error due to temp in (ppm * 100)
*******************************************************************************/



#endif 

