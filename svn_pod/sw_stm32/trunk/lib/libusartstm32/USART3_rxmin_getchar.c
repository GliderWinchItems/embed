/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : USART3_rxmin_getchar.c 
* Hackor             : deh
* Date First Issued  : 10/04/2010 deh
* Description        : USART3 rx minimal, check receive
*******************************************************************************/ 
#include "../libusartstm32/usartprotoprivate.h"
#include "../libusartstm32/usartall.h"

extern volatile char USART3_rcvflag;
extern volatile char USART3_rcvchar;
/******************************************************************************
 * char USART3_rxmin_getchar(void);
 * @brief	: Get a char from the single USART char buffer
 * @return	: Char received
******************************************************************************/
char USART3_rxmin_getchar(void)
{
	while (USART3_rcvflag == 0);
	USART3_rcvflag = 0;
	return USART3_rcvchar;
}
