/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : USART3_txmin_putc.c 
* Hackor             : deh
* Date First Issued  : 10/04/2010 deh
* Description        : USARTx tx minimal, send a char
*******************************************************************************/ 
#include "../libopenstm32/usart.h"
#include "../libusartstm32/usartprotoprivate.h"
#include "../libusartstm32/usartallproto.h"
/*******************************************************************************
* void USART3_txmin_putc(char c);
* @brief	: Put char.  Send a char
* @param	: Char to be sent
* @return	: none
*******************************************************************************/
void USART3_txmin_putc(char c)
{
	/* Wait until transmit register is empty */
	while ( (USART3_SR & USART_FLAG_TXE) == 0 );
	/* Place char in register */
	USART3_DR = c;
	return;
}
