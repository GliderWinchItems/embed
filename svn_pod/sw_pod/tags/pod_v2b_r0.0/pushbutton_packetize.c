/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : pushbutton_packetize.c
* Hacker	     : deh
* Date First Issued  : 09/06/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Handle the packeting of the pushbutton
*******************************************************************************/
/*
Subroutine call references shown as "@n"--
@1 = svn_pod/stm32/trunk/devices/rtctimers.c
@2 = svn_pod/stm32/trunk/lib/libusartstm32/usartallproto.c
@3 = svn_pod/stm32/trunk/lib/libusartstm32/UART4_rxint_getlineboth.c
@4 = svn_pod/stm32/trunk/lib/libsupportstm32/rtctimers.c



*/

#include "pushbutton_packetize.h"
#include "libopenstm32/gpio.h"
#include "p1_common.h"

/* Pushbutton packeting (@7) */
struct PKT_PUSHBUTTON pkt_pushbutton_sd;	// Packet with time stamp for pushbutton
int pushbutton_sd_pkt_ctr;		// Ready flag, (increments)


static unsigned int pbtime;
static short	state_pushbutton;	// Stately state that starts at zero
 short	pbcount;		// Count PB presses for shutdown
#define PBSHUTDOWNCT	5		// Number of consecutive (i.e. before timeout) pulses to cause shutdown
#define PBSHUTDOWNTIME	10		// Number seconds for 5 pushbutton presses to cause shutdown

/******************************************************************************
 * struct PKT_PTR  pushbutton_packetize_poll(void);
 * @brief	: Handle the pushbutton
 ******************************************************************************/
struct PKT_PTR pushbutton_packetize_poll(void)
{
//char vv[32]; // Debugging

/* 
This routine is polled from 'p1_logging_handler.c' 
*/
	struct PKT_PTR pp = {0,0};		// This holds the return values

	/* Set PA0 for input (since dual-use pin) */
	PA0_reconfig(1);	// Reconfigure so pushbutton can be seen

	/* Ignore if the configuration is not setup for input.  This is needed because this
	   pin is also used for output to flash the LED and the LED flashing and we don't want
	   the flashing to cause pushbutton packets. */

	if ( ( GPIO_CRL(GPIOA) & (0x000f) ) !=  ( ( (GPIO_CNF_INPUT_FLOAT<<2) | (GPIO_MODE_INPUT) ) << (4*0) ) )
		return pp;

	switch (state_pushbutton)
	{
	case 0:	// Beginning state

		/* Timer for timing shutdown via pushbutton presses */
		if (nTimercounter[3] == 0)//Has time duration for 5 pushbutton presses expired?
			pbcount = 0;

		/* PA0 (pushbutton) */
		if ((GPIO_IDR(GPIOA) & 0x01) == 0) break;	// Return Pushbutton not pushed

		/* Pushbutton is pressed */
		if (nTimercounter[3] == 0) // Were we still timing?
		{ // Here, no.  This is new possibility for getting 5 "quick" button presses
			pbcount = 0;
			nTimercounter[3] = PBSHUTDOWNTIME* 2048; // Set timer ticks (@4)
		}

		/* Setup the packet */
		pkt_pushbutton_sd.id = PKT_PUSHBUTTON_ID;
		pkt_pushbutton_sd.U.ull =  strAlltime.SYS.ull;	// Add extended linux format time

		/* If the time changed during the above, try again on the next poll entry */
		if (pkt_pushbutton_sd.U.ull !=  strAlltime.SYS.ull) break; 

		/* Setup struct returned */
		pp.ptr = (char*)&pkt_pushbutton_sd;		// Pointer to packet
		pp.ct  = sizeof (struct PKT_PUSHBUTTON);	// Byte count of packet

		/* Show it is ready to the sd logging writer */
		pushbutton_sd_pkt_ctr += 1;

		/* Pushbutton made contact.  Setup packet */
		pbtime = SYSTICK_getcount32() - PUSHBUTTONONDELAY*(sysclk_freq/10000);	// Get time

		state_pushbutton = 1;		// Next state
		break;

/* Debounce the switch--
Record/store the time when the switch first makes contact.  When it goes off
keep checking for a period of time.  If it goes on, wait for it go off.  When
it has remained off for an uninterrupted period of time, it is ready for another
cycle.
*/


	case 1:	// Wait for switch to debounce before looking for it to turn OFF
		if ( ((int)(SYSTICK_getcount32() - pbtime)) > 0  ) break;

		if ((GPIO_IDR(GPIOA) & 0x01) != 1) 
		{
			pbtime = SYSTICK_getcount32() - PUSHBUTTONONDELAY*(sysclk_freq/10000);	// Get time
			break;	// Return still ON
		}

		// Swtich was ON, and now it is the first case off OFF
		pbtime = SYSTICK_getcount32() - PUSHBUTTONOFFDELAY*(sysclk_freq/10000);	// Get time
		state_pushbutton = 2;		// Next state
		break;

	case 2:	// Switch was stably ON, now it is OFF, but might bounce back ON
		if ( ( (int)(SYSTICK_getcount32() - pbtime) ) > 0) break;

		if ((GPIO_IDR(GPIOA) & 0x01) != 0) 
		{
			pbtime = SYSTICK_getcount32() - PUSHBUTTONOFFDELAY*(sysclk_freq/10000);	// Get time	
			break;
		}

		/* If we counted 5 presses or more within the allotted time, then it is a signal to shutdown */
		pbcount += 1;
// Debug button pushing
//char vv[32];
//sprintf(vv,"pbcount %2u\n\r",pbcount);
//USART1_txint_puts(vv);USART1_txint_send();

		if (pbcount >= PBSHUTDOWNCT)
		{
			shutdownflag = 5;	// This will cause the shutdown to deepsleep to take place
			pbcount = 0;
		}

		state_pushbutton = 0;	// Ready for another button push
		break ;
	}
	return pp;
}
