/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : adcppm_cal_1.c
* Hackee             : deh
* Date First Issued  : 11/01/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Compute 32 KHz xtal error due to temperature
*******************************************************************************/
/*
This version replaces the table lookup version 'adcppm_cal_1.c,h'.

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
/*
Key for finding routines, definitions, variables, and other clap-trap
@1 = svn_pod/sw_stm32/trunk/lib/libusupportstm32/calibration.c,h

*/

#include "calibration.h"

extern struct CALBLOCK strDefaultCalib;	 // Default calibration (@1)

/******************************************************************************
 * unsigned short adcppm_cal_1 (unsigned int uiDegC);
 * @brief	: Compute error of 32 KHz xtal due to temperature
 * @param	: Temperature in deg C * 100
 * @return	: Error due to temp in (ppm * 100)
*******************************************************************************/
unsigned short adcppm_cal_1 (unsigned int uiDegC)
{
	/* Temp difference around the xtal temp curve turnpoint */
	int ntemp = (int)(uiDegC - strDefaultCalib.xtal_To);	// (@1)
	ntemp *= ntemp; 
	return ( ((ntemp /1000) * strDefaultCalib.xtal_alpha) / 10000);
}


