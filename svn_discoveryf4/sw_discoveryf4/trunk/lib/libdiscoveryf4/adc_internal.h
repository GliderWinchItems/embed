/******************************************************************************
* File Name          : adc_internal.h
* Date First Issued  : 08/09/2014
* Board              : Discovery F4
* Description        : ADC routines for Vrefint and Temp
*******************************************************************************/
/*


*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ADC_INTERNAL
#define __ADC_INTERNAL

/******************************************************************************/
unsigned int adc_internal_vref(unsigned int chan17);
/* @brief 	: Adjust voltage reference reading for temperature
 * @param	: ADC reading of chan 16 (Vrefint)
 * @return	: Vdda voltage
*******************************************************************************/



#define VREFIN_CAL (*(unsigned short *)0x1FFF7A2A)  // Raw data acquired at 30 deg C and Vdda = 3.3v
#define TS_CAL1    (*(unsigned short *)0x1fff7a2c)  // Raw temp reading at 3.3v at 30 deg C (e.g. 935)
#define TS_CAL2    (*(unsigned short *)0x1fff7a2e)  // Raw temp reading at 3.3v at 110 deg C (e.g. 1207)

/* Computes Vdda * 1000, given Vrefint ADC reading */
#define VDDA_NO_TEMP(x) ((VREFIN_CAL * 3300)/x) // Voltage * 1000

/* Computes temp in degrees C * 10000, given ADC readings of internal temp sensor, and Vrefint */
#define TEMP_INTERN(temp,vref)  ((30*10000) + ( ( (  ( 100* (VREFIN_CAL * temp) )  / vref )   - 100 * TS_CAL1 ) * nTslope)/100)

extern int vrefint_tco;	// Temp coefficient in ppm per deg C


#endif

