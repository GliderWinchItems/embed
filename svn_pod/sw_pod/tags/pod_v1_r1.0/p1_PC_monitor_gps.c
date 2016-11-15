/******************************************************************************
* File Name          : p1_PC_monitor_gps.h
* Author             : deh
* Date First Issued  : 09/13/2011
* Board              : STM32F103VxT6_pod_mm (USART1)
* Description        : Output current gps to PC
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




/* Subroutine prototypes */
void output_gps_cnt (unsigned int uiT);

/******************************************************************************
 * void p1_PC_monitor_gps(void);
 * @brief	: Output current GPS & tick ct to PC
 ******************************************************************************/
static short state;
static short gps_mn_pkt_ctrPrev;	// Previous packet ready count

void p1_PC_monitor_gps(void)
{
	unsigned int uitemp;

	switch (state)
	{
	case 0:
		if (gps_mn_pkt_ctr == gps_mn_pkt_ctrPrev) return;
		gps_mn_pkt_ctrPrev = gps_mn_pkt_ctr;	// Update for next round

		if (usMonHeader == 0)	// Check if a header has been placed
			state = 1;	// Here, no.  Place a column header
		else
			state = 2;	// Here, yes. Just put a line of data
		usMonHeader = 1;	// Only one header per menu selection
		break;

	case 1: /* Do a column header */
		/* Check if there is an available buffer */
		if (USART1_txint_busy() == 1) return;		// Return: all buffers full (@1)
//		USART1_txint_puts("    RTC sec--rem  Diff: high     low   sub   32 KHz    8 MHz   Offset w temp  temperr therm  deg C     linux time  converted\n\r");
//		USART1_txint_send();//  (@1)
		state = 2;
		break;

	case 2:
		/* Check if there is an available buffer */
		if (USART1_txint_busy() == 1) return;		// Return: all buffers full
		
		/* 'c' command stops the time adjusting with the gps (@8) */
		if (gps_timeadjustflag == 1)
		{ // Here, there is a 'c' command in effect
			USART1_txint_putc('c');
		}
		else
		{ // Here, 'g' command is in effect
			USART1_txint_putc('g');
		}

		output_gps_cnt ( (strAlltime.TIC.ul[0]) );	// Setup rtc_cnt as whole (secs) | remainder (ticks)

		/* Setup the ascii extracted GPS time as MM/DD/YY HH:MM:SS */
/*		printf ("%c%c/%c%c/%c%c %c%c:%c%c:%c%c",\
			pkt_gps_mn.g[ 8],pkt_gps_mn.g[ 9],
			pkt_gps_mn.g[ 6],pkt_gps_mn.g[ 7],
			pkt_gps_mn.g[10],pkt_gps_mn.g[11],
			pkt_gps_mn.g[ 0],pkt_gps_mn.g[ 1],
			pkt_gps_mn.g[ 2],pkt_gps_mn.g[ 3],
			pkt_gps_mn.g[ 4],pkt_gps_mn.g[ 5]);
*/
		/* Difference in ticks between GPS and rtc tick counter (print long long as separate longs) */
		printf("%10d %10d %4u",strAlltime.DIF.ul[1],strAlltime.DIF.ul[0],strAlltime.sPreTickTIC);

		/* 32 KHz osc bus ticks (nominal 24000000), system bus ticks between 1 PPS interrupts (nominal 48000000) */
		printf (" %9d %9d", uiTim1onesec,strAlltime.uiTim2diff );

//int nNumerator = (int)(2*uiTim1onesec - strAlltime.uiTim2diff);
//int nRecip = (int)strAlltime.uiTim2diff / nNumerator;
//int nX = (1000000000 / nRecip);

		/* Filtered freq offset and unfiltered offset (ppm*1000 = ppb) */
		printf (" %6d %6d",strAlltime.nOscOffFilter,Debug_nError);

		/* Average thermistor ADC reading (xxxxxx.x)  (@2) */
		printf(" %6d ",strAlltime.uiAdcTherm );
	
		/* Display the thermistor temperature in degrees C   (@2) */
		printf ("%4d.%02u",strAlltime.uiThermtmp/100,(strAlltime.uiThermtmp % 100) );	
				
//		printf ("%4d.%02u %6d",Debug_nErrorT/100,(Debug_nErrorT % 100),strAlltime.nOscOffset8);	

		/* Offset computed by polynomial for 32 KHz and 8 MHz oscs */	
		printf ("%6d %6d",strAlltime.nPolyOffset8,strAlltime.nOscOffset8);

		/* 'c' command stops the time adjusting with the gps (@8) */
		if ( (uiConsecutiveGoodGPSctr > 37) && (gps_timeadjustflag == 0) )
		{ // Here, there is a 'c' command in effect
			USART1_txint_putc('*');
		}
		else
		{ // Here, 'g' command is in effect
			USART1_txint_putc(' ');
		}

		/* Linux format time.  Tick count shifted right 11 bits (2048) */
		uitemp = (  (pkt_gps_mn.alltime.GPS.ull >> ALR_INC_ORDER) );
		printf (" %10u",  uitemp );

		/* Get epoch out of our hokey scheme for saving a byte to the 'ctime' routine basis */
		uitemp += PODTIMEEPOCH;		// Adjust for shifted epoch

		/* Reconvert to ascii (should match the above ascii where appropriate) */
		printf ("  %s\r", ctime((const time_t*)&uitemp));		

//union LL_L_S w;
//w.ull = (strAlltime.SYS.ull >> 11);
//printf ("%10u %s\r", w.ul[0], ctime((const time_t*)&w.ul[0]));



		USART1_txint_send();		// Start line sending.

//Debugging
//printf("Offset 32: %d 8: %d xtal_o8: %d\n\r",strAlltime.nOscOffset32,strAlltime.nOscOffset8,strDefaultCalib.xtal_o8);USART1_txint_send();

		state = 0;			// Reset state back to beginning 		
	}

	return;
}
/*****************************************************************************************
Setup an int as a seconds and tick within second
*****************************************************************************************/

void output_gps_cnt (unsigned int uiT)
{
	printf (" %8u %4u ",(uiT >> ALR_INC_ORDER),(uiT & ( (1<<ALR_INC_ORDER)-1) ) ) ;	// Setup the output

	return;
}
