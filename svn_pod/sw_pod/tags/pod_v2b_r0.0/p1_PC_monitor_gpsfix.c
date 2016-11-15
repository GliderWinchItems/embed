/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : p1_PC_monitor_gpsfix.c
* Author             : deh
* Date First Issued  : 06/26/2012
* Board              : STM32F103VxT6_pod_mm (USART1)
* Description        : Output current gps fix to PC
*******************************************************************************/

/*
Key for finding routines, definitions, variables, and other clap-trap
@1 = svn_pod/sw_stm32/trunk/lib/libusupportstm32/adc_packetize.c
@2 = svn_pod/sw_pod/trunk/pod_v1/p1_PC_monitor_batt.c
*/




#include "p1_gps_time_convert.h"
#include "p1_common.h"
#include "calendar_arith.h"
#include <time.h>




/******************************************************************************
 * void p1_PC_monitor_gpsfix(void);
 * @brief	: Output current GPS & tick ct to PC
 ******************************************************************************/
static unsigned short usGGAsavectrPrev;

static short state;


void p1_PC_monitor_gpsfix(void)
{
	switch (state)
	{
	case 0:
		if (usGGAsavectr == usGGAsavectrPrev) return;
		usGGAsavectrPrev = usGGAsavectr;	// Update for next round

		if (usMonHeader == 0)	// Check if a header has been placed
			state = 1;	// Here, no.  Place a column header
		else
			state = 2;	// Here, yes. Just put a line of data
		usMonHeader = 1;	// Only one header per menu selection
		break;

	case 1: /* Do a column header */
		/* Check if there is an available buffer */
		if (USART1_txint_busy() == 1) return;		// Return: all buffers full (@1)
		USART1_txint_puts("Time(gmt)  Latitude     Longitude      #Sat Ht MSL(m)\n\r");
		USART1_txint_send();//  (@1)
		state = 2;
		break;

	case 2:
		/* Check if there is an available buffer */
		if (USART1_txint_busy() == 1) return;		// Return: all buffers full

		/* Get pointer to extracted data from $GPGGA sentence set it up for output */	
		USART1_txint_puts(gpsfix_get_GGAsave());

		USART1_txint_send();		// Start line sending.


		state = 0;			// Reset state back to beginning 		
	}

	return;
}

