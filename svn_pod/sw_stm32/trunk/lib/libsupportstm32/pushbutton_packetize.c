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



*/

#include "pushbutton_packetize.h"
#include "libopenstm32/gpio.h"
#include "p1_common.h"

/* Pushbutton packeting (@7) */
struct PKT_PUSHBUTTON pkt_pushbutton_sd;	// Packet with time stamp for pushbutton
int pushbutton_sd_pkt_ctr;		// Ready flag, (increments)


static unsigned int pbtime;
static short	state_pushbutton;	// Stately state that starts at zero
/******************************************************************************
 * struct PKT_PTR  pushbutton_packetize_poll(void);
 * @brief	: Handle the pushbutton
 ******************************************************************************/
struct PKT_PTR pushbutton_packetize_poll(void)
{
	struct PKT_PTR pp = {0,0};		// This holds the return values

	switch (state_pushbutton)
	{
	case 0:	// Beginning state

		/* PA0 (pushbutton) */
		if ((GPIO_IDR(GPIOA) & 0x01) == 0) break;	// Return Pushbutton not pushed

		/* Setup the packet */
		pkt_pushbutton_sd.id = PKT_PUSHBUTTON_ID;
		pkt_pushbutton_sd.U.ull =  strAlltime.LNX.ull;	// Add extended linux format time

		/* Setup struct returned */
		pp.ptr = (char*)&pkt_pushbutton_sd;		// Pointer to packet
		pp.ct  = sizeof (struct PKT_PUSHBUTTON);	// Byte count of packet

		/* Show it is ready to the sd logging writer */
		pushbutton_sd_pkt_ctr += 1;

		/* Pushbutton made contact.  Setup packet */
		pbtime = SYSTICK_getcount32() - PUSHBUTTONOFFDELAY*(sysclk_freq/10000);	// Get time

		state_pushbutton = 1;		// Next state
		break;

/* Debounce the switch--
Record/store the time when the switch first makes contact.  When it goes off
keep checking for a period of time.  If it goes on, wait for it go off.  When
it has remained off for an uninterrupted period of time, it is ready for another
cycle.
*/

	case 1:	// Watch for it go off
		if ((GPIO_IDR(GPIOA) & 0x01) != 0) break;	// Return still on
		state_pushbutton = 2;		// Next state
		break;

	case 2:	// 
		if ( ( (int)(SYSTICK_getcount32() - pbtime) ) > 0) break; // Return still waiting
			state_pushbutton = 0;	// Ready for another 
		break ;
	}
	return pp;
}
