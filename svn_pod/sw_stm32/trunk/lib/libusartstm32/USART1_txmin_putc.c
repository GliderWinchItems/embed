/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : USART1_txmin_putc.c 
* Hackor             : deh
* Date First Issued  : 10/04/2010 deh
* Description        : USARTx tx minimal, send a char
*******************************************************************************/ 
#include "../libopenstm32/usart.h"
#include "../libusartstm32/usartprotoprivate.h"
#include "../libusartstm32/usartallproto.h"
/*******************************************************************************
* void USART1_txmin_putc(char c);
* @brief	: Put char.  Send a char
* @param	: Char to be sent
* @return	: none
*******************************************************************************/
void USART1_txmin_putc(char c)
{
	/* Wait until transmit register is empty */
	while ( (USART1_SR & USART_FLAG_TXE) == 0 );
	/* Place char in register */
	USART1_DR = c;
	return;
}
