/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : USART2_rxmin_getchar.c 
* Hackor             : deh
* Date First Issued  : 10/04/2010 deh
* Description        : USART2 rx minimal, check receive
*******************************************************************************/ 
#include "../libusartstm32/usartprotoprivate.h"
#include "../libusartstm32/usartall.h"

extern volatile char USART2_rcvflag;
extern volatile char USART2_rcvchar;
/******************************************************************************
 * char USART2_rxmin_getchar(void);
 * @brief	: Get a char from the single USART char buffer
 * @return	: Char received
******************************************************************************/
char USART2_rxmin_getchar(void)
{
	while (USART2_rcvflag == 0);
	USART2_rcvflag = 0;
	return USART2_rcvchar;
}
