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
@4 = svn_pod/sw_stm32/trunk/lib/libusupportstm32/adc_packetize.c
@5 = svn_pod/sw_pod/trunk/pod_v1/p1_PC_monitor_batt.c

*/


#include "p1_common.h"


/* Subroutine prototypes for this file */
static void output_calibrated_adc (unsigned int uiT);
static void output_100_scale(int uiS);

static	struct PKT_BATTMP *pp_batt = 0;
static short state;
 
/******************************************************************************
 * void p1_PC_monitor_batt(void);
 * @brief 	: Output current data to PC
 * @param	: 0 = run; 1 = stop
*******************************************************************************/
void p1_PC_monitor_batt(void)
{
	struct PKT_PTR	pp;

	/* Vars for conversion of ADC readings of thermistor (@1) */
	unsigned int uiThermtmp;
	unsigned int uiAdcTherm;
	unsigned int uiAdcTherm_m;
	unsigned int uiThermtmpF;
	unsigned int uiBotCellCal;
	unsigned int uiTopCellCal;
	unsigned int uiTopCellDif;


	switch (state)
	{
	case 0: /* Output a row */

		/* Check if there is an available buffer */
		if (USART1_txint_busy() == 1) return;		// Return: all buffers full (@1)
		
		if (usMonHeader == 0)	// Check if a header has been placed (@5)
			state = 1;	// Here, no.  Place a column header
		else
			state = 2;	// Here, yes. Just put a line of data
		usMonHeader = 1;	// Only one header per menu selection
		break; 

	case 1: /* Place a column header */
		/* Check if there is an available buffer */
		if (USART1_txint_busy() == 1) return;		// Return: all buffers full (@1)
		
		/* Output a header the gives the hapless op clues about the columns that follow */
		USART1_txint_puts("  Bottom Cell   Total battery    Top Cell Raw Therm  Temp deg C   Temp Deg F\n\r");

		/* Start sending line in USART buffer */
		USART1_txint_send();//  (@1)

		state = 2;
		break;

	case 2:

		/* Check if there is an available buffer */
		if (USART1_txint_busy() == 1) return;		// Return: all buffers full (@1)

		if (usMonHeader == 0)	// Check if a header has been placed (@5)
		{
			state = 0;
			break;
		}
		
		/* Check if new data ready */
		pp = adc_packetize_get_battmp_monitor();	// (@4)
		if (pp.ct == 0 )	return;		// Return no new data
	
		pp_batt = (struct PKT_BATTMP*)pp.ptr;	// Convert to packet ptr

		/* Output current battery voltages Top, Bottom cell, and difference, which is the Top cell */
				
		/* Calibrate & scale voltages to (volts * 100) */
		uiBotCellCal = ( strDefaultCalib.adcbot * pp_batt->adc[1] )>>16;
		uiTopCellCal = ( strDefaultCalib.adctop * pp_batt->adc[2] )>>16;
		uiTopCellDif = uiTopCellCal - uiBotCellCal;	// Top cell = (Top of battery volts - lower cell volts)

// Debug: show raw adc readings
// printf ("%6u %6u \n\r",(pp_batt->adc[1] ),(pp_batt->adc[2]) );
// printf ("%6u %6u ",batttemp_monbuf.adc[1],batttemp_monbuf.adc[2]);
	
		/* Setup voltages for output */
		output_calibrated_adc(  uiBotCellCal );	// Bottom cell
		output_calibrated_adc(  uiTopCellCal );	// Top of battery
		output_calibrated_adc(  uiTopCellDif );	// Top cell
	
		uiAdcTherm_m = pp_batt->adc[0];		// Thermistor adc reading
	
		/* Average thermistor ADC reading (xxxxxx.x) */
		printf(" %6d ",uiAdcTherm_m );
	
		/* Display the thermistor temperature in C and F (should Kelvin and Rankin be added?) */
		uiAdcTherm = uiAdcTherm_m;		// 
		uiThermtmp = adctherm_cal(uiAdcTherm);	// Convert ADC reading to temp (deg C)
		output_100_scale(uiThermtmp+strDefaultCalib.tmpoff);		// Output as XXXX.XX
		USART1_txint_puts(" C");	
		uiThermtmpF = ((uiThermtmp+strDefaultCalib.tmpoff)*9)/5 + 3200;	// Convert to Farenheit
		output_100_scale(uiThermtmpF);		// Output as XXXX.XX
		USART1_txint_puts(" F\n\r");	

		/* Start sending line in USART buffer */
		USART1_txint_send();//  (@1)

		/* Back to beginning */
		state = 0;		
	}
	
	return;
}
/*****************************************************************************************
Setup for output an unsigned with a 100* scale as xxxx.xx
*****************************************************************************************/
static void output_100_scale(int uiS)
{
	printf ("%8d.%02u",uiS/100,(uiS % 100) );

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

