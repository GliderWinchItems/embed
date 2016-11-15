/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : UART5_txmin_putc.c
* Hackor             : deh
* Date First Issued  : 03/05/2013 deh
* Description        : USARTx tx minimal, send a char
*******************************************************************************/ 
#include "../libopenstm32/usart.h"
#include "../libusartstm32/usartprotoprivate.h"
#include "../libusartstm32/usartallproto.h"
/*******************************************************************************
* void UART5_txmin_putc(char c);
* @brief	: Put char.  Send a char
* @param	: Char to be sent
* @return	: none
*******************************************************************************/
void UART5_txmin_putc(char c)
{
	/* Wait until transmit register is empty */
	while ( (UART5_SR & USART_FLAG_TXE) == 0 );
	/* Place char in register */
	UART5_DR = c;
	return;
}
