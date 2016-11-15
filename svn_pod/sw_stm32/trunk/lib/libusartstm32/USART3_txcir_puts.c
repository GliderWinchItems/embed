/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : USART3_txcir_puts.c 
* Hackor             : deh
* Date First Issued  : 10/14/2010 deh
* Description        : Put string: USART3 tx char-by-char interrupt with circular buffer
*******************************************************************************/ 
#include "../libusartstm32/usartprotoprivate.h" // Subroutine prototypes for internal use
#include "../libusartstm32/usartbitband.h" 	// Register defines for bit band use

/*******************************************************************************
* void USART3_txcir_puts(char* p);
* @brief	: Add a zero terminated string to the output buffer.
* @param	: Pointer to zero terminated string
* @return	: none
*******************************************************************************/
void USART3_txcir_puts(char* p)
{	
	while ( *p != 0) USART3_txcir_putc(*p++);
	return;
}


