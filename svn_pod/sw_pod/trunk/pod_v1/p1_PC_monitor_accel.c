/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : p1_PC_accel.c
* Author             : deh
* Date First Issued  : 10/01/2011
* Board              : STM32F103VxT6_pod_mm (USART1)
* Description        : Output current accelerometer data to PC
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

*/


#include "p1_common.h"
#include <math.h>

static	int *	pp = 0;	// Pointer to data

/******************************************************************************
 * void p1_PC_monitor_accel(void);
 * @brief 	: Output current data to PC
 * @param	: 0 = run; 1 = stop
*******************************************************************************/
static short state;
 
void p1_PC_monitor_accel(void)
{
	int	nT;	// Temp
	int 	nX;	// Whole part
	int 	nR;	// Fractional part
	int	i;	// for index for FORTRAN type loop

	/* Sum of squares used for computing vector magnitude */
	float dS;	
	int long long llS;


	/*  */


	switch (state)
	{
	case 0:
		/* Check if new data ready */
		pp = adc_packetize_get_accel_monitor();	// (@4)
		if (pp == 0 )	return;		// Return no new data
		if (usMonHeader == 0)	// Check if a header has been placed
			state = 1;	// Here, no.  Place a column header
		else
			state = 2;	// Here, yes. Just put a line of data
		usMonHeader = 1;	// Only one header per menu selection
		break;

	case 1: /* Do a column header */
		/* Check if there is an available buffer */
		if (USART1_txint_busy() == 1) return;		// Return: all buffers full (@1)
		USART1_txint_puts("       Z         Y         X      |Vector|  (g's)\n\r");
		USART1_txint_send();//  (@1)
		state = 2;
		break;
	
	case 2:
		/* Check if there is an available buffer */
		if (USART1_txint_busy() == 1) return;		// Return: all buffers full (@1)
		
		/* Calibrate accelerometer: Z,Y,X into g-force as x.xx  */

		llS = 0;	// Vector magnitude sum initialize
		for ( i = 0; i < NUMBERACCELREADINGSPKT; i ++)
		{
			nT = *(pp+i) - strDefaultCalib.accel_offset[i];	// Adjust for zero point
			nT = (nT*1000)/strDefaultCalib.accel_scale[i];	// Scale to g * 100

			/* Compute vector magnitude */
			llS += nT * nT;	// Build sum of X^2 + Y^2 + Z^2

		/* Rather ugly mess for printf'ing signed x.xxx */
			if (nT < 0)
			{
				nT = -nT;				// Deal with as positive for now
				nX = nT/1000;				// Whole part
				nR = nT % 1000;				// Fractional part
				if (nX == 0)
				{	
					printf ("   -0.%03u ",nR);	// Zero or less and negative
				}	
				else
				{
					printf (" %4d.%03d ",-nX,nR);	// Negative and greater than 0
				}
			}
			else
			{ // Positive values are easier 
				nX = nT/1000;				// Whole part
				nR = nT % 1000;				// Fractional part
				printf (" %4d.%03u ",nX,nR);		// Setup the output
			}
		}

		/* Vector magnitude = square root of sum of squares */
		dS = llS;	// Convert to floating pt for lazy man's way of taking square root
		nT = sqrt(dS);	// Yes my friends, this does the square root.
		/* Print vector magnitude in same format as X,Y,Z */
		nX = nT/1000;				// Whole part
		nR = nT % 1000;				// Fractional part
		printf (" %4d.%03u ",nX,nR);		// Setup the output

		USART1_txint_puts("\n\r");	

		/* Setup line in USART buffer */
		USART1_txint_send();//  (@1)

		state = 0;		
	}
	
	return;
}

