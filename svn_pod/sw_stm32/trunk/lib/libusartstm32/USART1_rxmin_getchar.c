/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : USART1_rxmin_getchar.c 
* Hackor             : deh
* Date First Issued  : 10/04/2010 deh
* Description        : USART1 rx minimal, check receive
*******************************************************************************/ 
#include "../libusartstm32/usartprotoprivate.h"
#include "../libusartstm32/usartall.h"

extern volatile char USART1_rcvflag;
extern volatile char USART1_rcvchar;
/******************************************************************************
 * char USART1_rxmin_getchar(void);
 * @brief	: Get a char from the single USART char buffer
 * @return	: Char received
******************************************************************************/
char USART1_rxmin_getchar(void)
{
	while (USART1_rcvflag == 0);
	USART1_rcvflag = 0;
	return USART1_rcvchar;
}
