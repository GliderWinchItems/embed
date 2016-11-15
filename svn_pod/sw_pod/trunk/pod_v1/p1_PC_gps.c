/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : p1_PC_gps.c
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
@2 = svn_pod/sw_pod/trunk/lib/libsupportstm32/gps_packetize.h
@3 = svn_pod/sw_stm32/trunk/lib/libusartstm32/usartallproto.c

*/


/******************************************************************************
 * void p1_PC_gps(void);
 * @brief 	: Output current data to PC
 * @param	: 0 = run; 1 = stop
*******************************************************************************/
static unsigned int pkt_gps_mn_ts_Prev;	// Previous tick counter
 
void p1_PC_gps(void)
{
	/* See if the store tick count changed */
	if (pkt_gps_mn.ts.cnt == pkt_gps_mn_ts_Prev) return; // New data?  Return for no. (@2)

	/* Check if there is an available buffer */
	if (USART1_txint_busy() == 1) return;		// Return: all buffers full (@1)

	pkt_gps_mn_ts_Prev = pkt_gps_mn.ts.cnt;		// Update previous (@2)

	/* GPS time as MM/DD/YY HH:MM:SS (@1) */
	printf ("%c%c/%c%c/%c%c %c%c:%c%c:%c%c ",pkt_gps_mn.ts.g[8],pkt_gps_mn.ts.g[9],pkt_gps_mn.ts.g[10],pkt_gps_mn.ts.g[11],pkt_gps_mn.ts.g[6],pkt_gps_mn.ts.g[7],pkt_gps_mn.ts.g[0],pkt_gps_mn.ts.g[1],pkt_gps_mn.ts.g[2],pkt_gps_mn.ts.g[3],pkt_gps_mn.ts.g[4],pkt_gps_mn.ts.g[5]); 
		
	/* Setup tick count as xxxxxx|zzzz */
	printf ("%6u|%04u ",pkt_gps_mn.ts.cnt/2048, pkt_gps_mn.ts.cnt%2048 );	

	/* Setup line in USART buffer */
	USART1_txint_send();//  (@3)

	return;
}



