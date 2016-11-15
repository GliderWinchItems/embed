/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : usartx_txmin_init.c 
* Hackor             : deh
* Date First Issued  : 10/03/2010 deh
* Description        : USARTx tx minimal, initialization
*******************************************************************************/ 
#include "../libopenstm32/usart.h"

#include "../libusartstm32/usartprotoprivate.h"
#include "../libusartstm32/usartall.h"

/******************************************************************************
 * void usartx_txmin_init (u32 USARTx, u32 BaudRate);
 * @brief	: Initializes the USARTx to 8N1 and the baudrate 
 * @param	: u32 Baudrate		- baudrate 		(e.g. 115200)
 * @return	: none
******************************************************************************/
void usartx_txmin_init (u32 USARTx, u32 BaudRate)
{
	/* Set baud rate */
	usartx_setbaud (USARTx,BaudRate);	// Compute divider settings and load BRR

	 	/* Setup CR2 ------------------------------------------------------------------- */
		/* After reset CR2 is 0x0000 and this is just fine */
		/* Set up CR1 (page 771) ------------------------------------------------------- */
		USART_CR1(USARTx) |= USART_UE | USART_TX_ENABLE;// Set UE, TE
		// Note Tx interrupt enable is set when there is data ready to be sent		

	return;	
}

