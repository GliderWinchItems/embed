/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : p1_normal_run.c
* Hackeroos          : caw, deh
* Date First Issued  : 08/30/2011
* Board              : STM32F103VxT6_pod_mm (USART1)
* Description        : Main program for version implementation
*******************************************************************************/
/*
Subroutine call references shown as "@n"--
@1 = svn_pod/sw_stm32/trunk/devices/rtctimers.c
@2 = svn_pod/sw_pod/trunk/pod_v1/p1_initialization.h
@3 = svn_pod/sw_pod/trunk/pod_v1/RS232_ctl.h
@4 = svn_pod/sw_pod/trunk/pod_v1/p1_PC_handler.h
@5 = svn_pod/sw_pod/trunk/pod_v1/gps_packetize.h
@6 = svn_pod/sw_pod/trunk/pod_v1/logging_handler.h
@7 = svn_pod/sw_pod/trunk/pod_v1/tickadjust.c


*/
/*
RTC timer usage:
timer 0 - MAX232 shutdown
timer 1 - No tension activity shutdown

*/

#include "p1_common.h"

unsigned int uiDebug_norm0;
unsigned int uiDebug_norm1;
unsigned int uiDebug_normX;
unsigned int uiDebug_normC;


/******************************************************************************
 * void p1_normal_run(void);
 * @brief 	: Get started with the sequence when reset is comes out of STANDBY
*******************************************************************************/
void p1_normal_run(void)
{
USART1_txint_puts("p1_norm_run \n\r");USART1_txint_send();			

	/* Initialize things needed for active mode */
	nTimercounter[1] = INACTIVELOGGINGTICKCOUNT;	// Set active mode timeout counter (@1)

	/* Setup time that has been saved in the RTC memory */
	RTC_tickadjust_init_exiting_standby();

/* Debugging: Don't uncomment this unless the USART initialization/setup is uncommented in 'pod_v1.c'
otherwise it will hang because of an uninitialized USART routine. */
//printf ("Debug_dif: %10d \n\r",Debug_dif); USART1_txint_send();

	p1_initialization_active();	// Get ADC, AD7799, SD Card ready (@2)
	
// Debugging: Initial time for measuring longest time around the polling loop
uiDebug_norm0 = SYSTICK_getcount32();	

/* ======================== ENDLESS POLLING LOOP ========================================== */
	/* Escape from loop is via going into STANDBY in 'shutdown_handler' */
	while (1==1)
	{
// Measure longest time around the polling loop
uiDebug_norm1 = uiDebug_norm0 - SYSTICK_getcount32();
uiDebug_norm0 = SYSTICK_getcount32();
//uiDebug_normX = uiDebug_norm1;
if ( uiDebug_norm1 > uiDebug_normX) uiDebug_normX = uiDebug_norm1;
if ( uiDebug_normC++ > 100000) {uiDebug_normX = 0; uiDebug_normC = 0;}

		/* Compute estimate of osc freq error due to temperature */
		RTC_tickadjust_poly_compute();	// (@7)

		/* Handle powering up/down MAX232 for PC and GPS */
		if (RS232_ctl() == 1)	// Is USART1 & UART4 ready? (@3)
		{ // Here, yes. UART4 & USART1 are up & ready.
			p1_PC_handler();	// This takes care of comm with PC (@4)
		}

		/* Setting up gps packets is not done under '32KHz_p1.c' ticks so we poll it here */
		gps_packetize_poll();

		/* This polling drives the writing of logging of Pushbutton, GPS, AD7799, accelerometer, battery|thermistor data, etc. */
		p1_logging_handler();		// Check for any packets ready and write them to the SD card (@6)

		/* LED pulse control: poll to turn off.  Someone else can turn it on. */
		LED_ctl();

		/* Check for shutdown: no activity timeout, battery low, PC shutdown command, whatever */
		p1_shutdown_normal_run();	// Upon timout we never return from this, i.e. go into STANDBY deepsleep
	}
}

