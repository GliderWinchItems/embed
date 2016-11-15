/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : UART5_rxmin_rxready.c 
* Hackor             : deh
* Date First Issued  : 03/05/2013 deh
* Description        : UART5 rx minimal, check receive
*******************************************************************************/ 
#include "../libusartstm32/usartall.h"

extern char UART5_rcvflag;

/******************************************************************************
 * char UART5_rxmin_rxready(void);
 * @brief	: Check for USART transmit register empty
 * @return	: 0 = no char, 1 = char waiting, 2 or more = chars missed
******************************************************************************/
u16 UART5_rxmin_rxready(void)
{
	return UART5_rcvflag;
}

