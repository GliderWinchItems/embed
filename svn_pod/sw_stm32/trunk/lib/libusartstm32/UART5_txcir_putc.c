/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : UART5_txcir_putc.c
* Hackor             : deh
* Date First Issued  : 10/14/2010
* Description        : Add a char to the circular tx buffer
*******************************************************************************/
#include "../libusartstm32/usartprotoprivate.h"
#include "../libusartstm32/usartall.h"
#include "../libusartstm32/usartbitband.h" 	// Register defines for bit band use

/* 
NOTE:
No check for buffer overrun is made.  All overrun will do with a circular
buffer is overwrite chars that have not been sent, meaning the buffer size is either
too small or the program is putting out chars faster on average than the usart can send.
*/

/*******************************************************************************
* void UART5_txcir_putc(char c);
* @brief	: Put char.  Add a char to output buffer
* @param	: Char to be sent
* @return	: none
*******************************************************************************/
void UART5_txcir_putc(char c)
{
	/* If caller did not check for busy, we must loop if buffer is full */
	while ( UART5_txcir_putcount() == 0);	

	/* Add char to circular buffer, and advance pointer circularly */
	usartx_txcir_putc(c, pUSARTcbt5);

	/* Make sure Tx interrupt is enabled */
	MEM_ADDR(BITBAND(UART5CR1,TXFLAG)) = 0x01;	// Enable Tx interrupts
 	return ;	
}
