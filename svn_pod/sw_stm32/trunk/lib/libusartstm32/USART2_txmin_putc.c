/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : USART2_txmin_putc.c 
* Hackor             : deh
* Date First Issued  : 10/04/2010 deh
* Description        : USARTx tx minimal, send a char
*******************************************************************************/ 
#include "../libopenstm32/usart.h"
#include "../libusartstm32/usartprotoprivate.h"
#include "../libusartstm32/usartallproto.h"
/*******************************************************************************
* void USART2_txmin_putc(char c);
* @brief	: Put char.  Send a char
* @param	: Char to be sent
* @return	: none
*******************************************************************************/
void USART2_txmin_putc(char c)
{
	/* Wait until transmit register is empty */
	while ( (USART2_SR & USART_FLAG_TXE) == 0 );
	/* Place char in register */
	USART2_DR = c;
	return;
}
