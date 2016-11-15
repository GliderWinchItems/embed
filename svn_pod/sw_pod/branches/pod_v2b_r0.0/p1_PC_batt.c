/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : p1_PC_batt.c
* Author             : deh
* Date First Issued  : 09/10/2011
* Board              : STM32F103VxT6_pod_mm (USART1)
* Description        : Output current data to PC
*******************************************************************************/
/*
This routine "monitors" the current data, by outputting the readings in ASCII to the
PC. 

p1_PC_handler.c polls this routine if a 'm' command has been entered.  The polling
continues until a 'x' command is received.

The data 
*/
/*
Key for finding routines, definitions, variables, and other clap-trap
@1 = svn_pod/sw_stm32/trunk/gpstest2/gpstest2.c
@2 = svn_pod/sw_stm32/trunk/devices/32KHz_p1.c,h
@3 = svn_pod/sw_stm32/trunk/lib/libusartstm32/usartallproto.c

*/


#include "p1_common.h"

/* Subroutine prototypes for this file */
static void output_calibrated_adc (unsigned int uiT);
static void output_100_scale(int uiS);


/******************************************************************************
 * void p1_PC_batt(void);
 * @brief 	: Output current data to PC
 * @param	: 0 = run; 1 = stop
*******************************************************************************/
static unsigned int uiRTCsystemcounter;	// Previous tick counter
 
void p1_PC_batt(void)
{
	/* Vars for conversion of ADC readings of thermistor (@1) */
	unsigned int uiThermtmp;
	unsigned int uiAdcTherm;
	unsigned int uiThermtmpF;

	/* Check if it is time to output a reading (@2) */
	if ((uiRTCsystemcounter - (uiRTCsystemcounterPrev + ((PCBATINCREMENT * ALR_INC_ORDER)/10))) < 0 ) return;

	/* Check if there is an available buffer */
	if (USART1_txint_busy() == 1) return;		// Return: all buffers full (@1)
	
	uiRTCsystemcounterPrev = uiRTCsystemcounter;	// Update previous time

	/* Output current battery voltages Top, Bottom cell, and difference, which is the Top cell */
		
	/* Calibrate & scale voltages to (volts * 100) */
	uiBotCellCal = ((strDefaultCalib.adcbot * strADC1dr.in[0][1]) >> ADCAVERAGEORDER );
	uiTopCellCal = ((strDefaultCalib.adctop * strADC1dr.in[0][2]) >> ADCAVERAGEORDER );
	uiTopCellDif = uiTopCellCal - uiBotCellCal;	// Top cell = (Top of battery volts - lower cell volts)

	/* Setup voltages for output */
	output_calibrated_adc(  uiBotCellCal );	// Top of battery
	output_calibrated_adc(  uiTopCellCal );	// Bottom cell
	output_calibrated_adc(  uiTopCellDif );	// Top cell

	/* Average thermistor ADC reading (xxxxxx.x) */
	printf(" %6d.%01u ",uiAdcTherm_m/1024,( (uiAdcTherm_m*10)/1024) -( ( ( (uiAdcTherm_m*10)/1024)/10)*10) );

	/* Display the thermistor temperature in C and F */
	uiAdcTherm = uiAdcTherm_m/1024;		// Integer portion of ADC thermistor 1/2 sec average
	uiThermtmp = adctherm_cal(uiAdcTherm);	// Convert ADC reading to temp (deg C)
	output_100_scale(uiThermtmp+strDefaultCalib.tmpoff);		// Output as XXXX.XX
	USART1_txint_puts(" C");	
	uiThermtmpF = ((uiThermtmp+strDefaultCalib.tmpoff)*9)/5 + 3200;	// Convert to Farenheit
	output_100_scale(uiThermtmpF);		// Output as XXXX.XX
	USART1_txint_puts(" F");	

	/* Setup line in USART buffer */
	USART1_txint_send();//  (@1)

	return;
}
/*****************************************************************************************
Setup for output an unsigned with a 100* scale as xxxx.xx
*****************************************************************************************/
static void output_100_scale(int uiS)
{
	printf ("%4d.%02u",uiS/100,uiS % 100) );

	return;
}
/*****************************************************************************************
Setup an int for a floating pt type output
*****************************************************************************************/
static void output_calibrated_adc (unsigned int uiT)
{
	unsigned int uiX = uiT/1000;	// Whole part
	unsigned int uiR = uiT % 1000;	// Fractional part

	printf (" %8u.%03u ",uiX,uiR);	// Setup the output

	return;
}

