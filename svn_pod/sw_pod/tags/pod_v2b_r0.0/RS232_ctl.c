/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : RS232_ctl.c
* Author             : deh
* Date First Issued  : 09/01/2011
* Board              : STM32F103VxT6_pod_mm (USART1)
* Description        : Power up/down, initialization contorl for USART1, UART4
*******************************************************************************/
/*
Strategy:

Coming out of STANDBY the "state" is zero.  The only way to normal_run is if the pushbutton 
had been pushed.   A reset out of STANDBY cause by the RTC Alarm will go directly to 
'deepsleep_run'.  Therefore, the main polling loop calling RS232_ctl() will cause the
MAX232 to be brought up, then USART1 (PC), UART4 (GPS) to be initialized.  If there is no
activity on the PC for a time perior, and the GPS has reached its limit, the MAX232 will
be shut back down.  
Subsequent calls by the main polling loop will check the pushbutton.  If it is on the
MAX232 will be brought up again, however USART1, and UART4 do not need intialization.

*/


/*
Subroutine call references shown as "@n"--
@1 = svn_pod/stm32/trunk/devices/Podpinconfig.h
@2 = svn_pod/stm32/trunk/lib/libopenstm32/gpio.h
@3 = svn_pod/stm32/trunk/lib/libmiscstm32/SYSTICK_getcount32.c
@4 = svn_pod/stm32/trunk/devices/adcpod.c
@5 = svn_pod/stm32/trunk/lib/libusartstm32/USART1_txint_putc.c
@6 = svn_pod/stm32/trunk/devices/32KHz.c

*/

/* USART/UART */
#include "libusartstm32/usartallproto.h"


/* The following are in 'svn_pod/stm32/trunk/lib' */
#include "libopenstm32/systick.h"
#include "libmiscstm32/systick1.h"
#include "libmiscstm32/printf.h"// Tiny printf
#include "libmiscstm32/clockspecifysetup.h"

#include "libopenstm32/gpio.h"

#include "pod_v1.h"		// Tiny main routine
#include "p1_common.h"		// Variables in common
#include "RS232_ctl.h"		// MAX232, and USART1, UART4 bring-up & initialization
#include "rtctimers.h"		// RTC countdown timers
#include "p1_initialization.h"	// Some things for the following


/* The following are in 'svn_pod/stm32/trunk/devices' */
#include "32KHz_p1.h"		// RTC & BKP routines
#include "ad7799_comm.h"	// ad7799 routines (which use spi1 routines)
#include "adcpod.h"		// ADC1 routines
#include "PODpinconfig.h"	// gpio configuration for board
#include "pwrctl.h"		// Low power routines
#include "spi1ad7799.h"		// spi1 routines 


#define RS232SWITCHDELAY	50	// 0.1 ms count to wait

static unsigned int uiSystick_rs232;	// Time for bringing up MAX232 converter
//static char cRS232_skip_flag;		// 0 = initialize uarts; 1 = skip uart initialization
static char cRS232_state;		// State.  This is zero coming out of a reset.

char cRS232_skip_flag;		// Debugging



/******************************************************************************
 * int RS232_ctl(void);
 * @brief 	: Get started with the sequence when reset is comes out of STANDBY
 * @return	: 0 = Not ready, 1 = ready
*******************************************************************************/
 int RS232_ctl(void)
{
	switch (cRS232_state)	// Note: out of reset the state = 0
	{
	case 0:	// Beginning of start up.
		MAX3232SW_on		// Turn on RS-232 level converter (if doesn't work--no RS-232 chars seen) (@1)

		/* Set PA0 for output (since dual-use pin) while RS232 is up */
//		PA0_reconfig(0);	

		/* NOTE: Systick counter counts DOWN (not up).  Get  time switch turned on, and computed the count
			that will assure at least the minimum delay required for voltages to build up.  */
		uiSystick_rs232 = SYSTICK_getcount32() - (RS232SWITCHDELAY*(sysclk_freq/10000));	// Turn time (@3)		
		
		cRS232_state = 1;	// Next state will test for time delay to expire
		return 0;		// Return not yet ready.
		break;	// JIC

	case 1:	// Has enough time elapsed so that the converter voltages are sufficiently stabilized?
		if ( ((int)(SYSTICK_getcount32() - uiSystick_rs232)) > 0 )	return 0; // Return no.
		{ // Here time has expired so we can now initialize USART1 and USART4

				/* Initialize usart 
			USARTx_rxinttxint_init(...,...,...,...);
			Receive:  rxint	rx into line buffers
			Transmit: txint	tx with line buffers
			ARGS: 
				baud rate, e.g. 9600, 38400, 57600, 115200, 230400, 460800, 921600
				rx line buffer size, (long enough for the longest line)
				number of rx line buffers, (must be > 1)
				tx line buffer size, (long enough for the longest line)
				number of tx line buffers, (must be > 1)
											*/
			/* USART1 is TX/RX comm with the PC */
			if (cRS232_skip_flag == 0)	// Is USART1 & UART4 already initialized?
			{ // Here, no.  Setup the buffers
				cRS232_skip_flag = 1;	// Set flag. Only do initialization once!
				// Initialize USART and setup control blocks and pointer
		/* WARNING: Be sure not to exceed the block size that mymalloc uses */
//				USART1_rxinttxint_initRTC(115200,32,2,PACKETREADBUFSIZE,3);
				USART1_rxinttxint_initRTC(115200,64,2,256,3);
				// Initialize UART4 (GPS)
				UART4_rxinttxint_initRTC(4800,96,2,32,2);
			}
			/* Setup timer for RTC timer to time the no-activity shutdown of the MAX232 */
			nTimercounter[0] = 2048 * RS232TIMEOUT;	// Set a tickcount

			/* Initialize for limiting the GPS logging */
			// Code goes here.


			cRS232_state = 2;	// Next state
			return 1;		// Return not yet ready.		
		}
		break; // JIC

	case 2:	// Here USART1 and UART4 are up.  
		if (cGPS_flag == 0) return 1;		// Return GPS still needed
		if (nTimercounter[0] > 0) return 1;	// Return, not yet timed out
		// Here, timeout with no activity. So shut down MAX232
		MAX3232SW_off		// Turn off RS-232 level converter (@1)

		cRS232_state = 3;	// Next state

		/* Set PA0 for input so that pushbutton will wake up RS232 */
		PA0_reconfig(1);	

		return 0;		// Return not ready.

	case 3: // Here, MAX232 was shutdown, so see if we want it back up.
		/* PA0 (pushbutton) */
		if ((GPIO_IDR(GPIOA) & 0x01) == 0)  return 0; // Return zero for "Not running & no pushbutton"
		// Here, pushbutton is on, so get it started (again)	
		cRS232_state = 0;	// Restart initialization

		/* This will cause a new menu to be gloriously displayed */		
		p1_PC_handler_restart();

		return 0;
	}
	return 0;	// JIC

}
/******************************************************************************
 * void RS232_ctl_reset_timer(void);
 * @brief 	: Reset the no-acitivity shutdown timer
*******************************************************************************/
 void RS232_ctl_reset_timer(void)
{
	/* Reset timer for RTC timer to time the no-activity shutdown of the MAX232 */
	nTimercounter[0] = 2048 * RS232TIMEOUT;
	return;
}


