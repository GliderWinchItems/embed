/******************************************************************************
* File Name          : vcal_cmd.c
* Date First Issued  : 11/17/2015
* Board              : f103
* Description        : Serial input PC commands
*******************************************************************************/
/*
This is used for the F103 since the launchpad compiler was not working for 
floating pt printf (which works for the 'F4).
*/
#include <stdio.h>
#include <string.h>
#include "printf.h"
#include "libusartstm32/usartallproto.h"



/* **************************************************************************************
 * int vcal_cmd_poll(void);
 * @brief	: Deal with PC incoming lines
 * @return	: Flags: 0 = none
 *		: bit 0 = R command 
 *		: bit 1 = K command
 * ************************************************************************************** */
int vcal_cmd_poll(void)
{
	int ret = 0;
	struct USARTLB strlb;	// Holds the return from 'getlineboth' of char count & pointer

	strlb = USART1_rxint_getlineboth();	// Get both char count and pointer

	/* Check if a line is ready. */
	if (strlb.p == (char*)0) return ret;

	// Here we have a pointer to the line and a char count
	printf("ECHO: (ct%d) %s\n\r",strlb.ct,strlb.p);USART1_txint_send();
	if (strlb.ct < 2) return ret;
	switch (*(strlb.p + 0)) // First letter of command
	{
	case 'r': // Reset accumulation
	case 'R':  
		ret |= 1;	// Set bit in returned variable

		break;

	default: // Here, hapless Op w fat fingers
		printf("PC: only R (either upper case or lower case) valid\n\r");
		break;
	}
	USART1_txint_send();
	return ret;
}

