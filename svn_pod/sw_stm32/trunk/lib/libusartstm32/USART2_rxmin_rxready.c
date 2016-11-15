/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : USART2_rxmin_rxready.c 
* Hackor             : deh
* Date First Issued  : 10/04/2010 deh
* Description        : USART2 rx minimal, check receive
*******************************************************************************/ 
#include "../libusartstm32/usartall.h"

extern char USART2_rcvflag;

/******************************************************************************
 * char USART2_rxmin_rxready(void);
 * @brief	: Check for USART transmit register empty
 * @return	: 0 = no char, 1 = char waiting, 2 or more = chars missed
******************************************************************************/
u16 USART2_rxmin_rxready(void)
{
	return USART2_rcvflag;
}

