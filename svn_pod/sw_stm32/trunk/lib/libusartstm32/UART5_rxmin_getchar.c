/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : UART5_rxmin_getchar.c 
* Hackor             : deh
* Date First Issued  : 03/05/2013 deh
* Description        : USART1 rx minimal, check receive
*******************************************************************************/ 
#include "../libusartstm32/usartprotoprivate.h"
#include "../libusartstm32/usartall.h"

extern volatile char UART5_rcvflag;
extern volatile char UART5_rcvchar;
/******************************************************************************
 * char UART5_rxmin_getchar(void);
 * @brief	: Get a char from the single USART char buffer
 * @return	: Char received
******************************************************************************/
char UART5_rxmin_getchar(void)
{
	while (UART5_rcvflag == 0);
	UART5_rcvflag = 0;
	return UART5_rcvchar;
}
