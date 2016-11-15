/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : p1_logging_handler.c
* Hacker	     : deh
* Date First Issued  : 09/09/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Logging of packets to SD card
*******************************************************************************/
/*
This routine checks for packets that are ready to be sent, and sends them.

This routine is in the 'main' endless polling loop.
*/

/*
Key for finding routines, definitions, variables, and other clap-trap
@1 = svn_pod/sw_pod/trunk/lib/libsupportstm32/ad7799_packetize.c
@2 = svn_pod/sw_pod/trunk/lib/libsupportstm32/gps_packetize.c
@3 = svn_pod/sw_pod/trunk/lib/libsupportstm32/adc_packetize.c
@4 = svn_pod/sw_pod/trunk/lib/libsupportstm32/pushbutton_packetize.c

*/

#include "p1_common.h"



unsigned int Debugpktctr;


/* The following is an array of subroutine pointers for checking for packets.
The routines return a pointer to the first byte and byte count when a packet
is ready; otherwise, the pointer and count returned are both zero. */


struct PKT_PTR (*packetize_ptr[NUMBERPKTTYPES])(void) = {
	&ad7799_packetize_get_main,	/* ID = 1 Tension reading packets (@2) */
	&gps_packetize_get,		/* ID = 2 GPS time v rtc counter packet (@1)*/
	&adc_packetize_get_battmp,	/* ID = 3 Battery|thermistor packet (@3)*/
	&pushbutton_packetize_poll,	/* ID = 4 Pushbutton v rtc counter packet (@4)*/
	&adc_packetize_get_accel,	/* ID = 5 ADC accelerometer readings (@3)*/
	};

int sdlog_keepalive_ctr;	// Counts "work to do" return from 'keepalive'
int sdlog_keepalive_max;	// Max 'ctr
/******************************************************************************
 * void p1_logging_handler(void);
 * @brief	: Handle the logging of packets to the SD card
 ******************************************************************************/
void p1_logging_handler(void)
{

	struct PKT_PTR pp;	// Pointer and count (@1)
	int i;

	/* Check if we are read/writing too fast  */
	if (sdlog_keepalive() == (1==1)) 	// Work to do?
		sdlog_keepalive_ctr += 1;	// Yes. Build a count
	else
	{ // Here, no work to do, so retain the highest count.
		if (sdlog_keepalive_ctr > sdlog_keepalive_max)
		{
			sdlog_keepalive_max = sdlog_keepalive_ctr;
			sdlog_keepalive_ctr = 0; // Reset counter
		}
	}
	
	/* When the shutdown flag is up, stop adding new packets to send */
	if (shutdownflag == 0)
	{ // Here, not a shutdown situation
		/* Check each "packetizer" for a packet ready */
		for (i = 0; i < NUMBERPKTTYPES; i++)
		{
			pp = (*packetize_ptr[i])();	// Check if packet ready
			if (pp.ct > 0)				// Skip if no bytes
			{
// Debugging: Skip types of packets such as the high rate ones
//if ( (i== 1)||(i==2)||(i==3) ) // Selected
//{
//Debugpktctr += 1;
				sdlog_write(pp.ptr, pp.ct);	// Write packet
//}
			}
		}
	}

	return;	
}

