/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : usartx_rxmin_init.c 
* Hackor             : deh
* Date First Issued  : 10/04/2010 deh
* Description        : USARTx rx minimal one char receive initialization
*******************************************************************************/ 
#include "../libopenstm32/usart.h"
#include "../libopenstm32/memorymap.h"
#include "../libopenstm32/rcc.h"
#include "../libopenstm32/gpio.h"
#include "../libopenstm32/rcc.h"

#include "../libusartstm32/usartallproto.h"
#include "../libusartstm32/usartprotoprivate.h"

/******************************************************************************
 * void usartx_rxmin_init (u32 USARTx, u32 BaudRate);
 * @brief	: Setup USART baudrate, Rx interrupting with line buffers
 * @param	: USARTx = USART1, USART2, or USART3 base address
* @param	: u32 Baudrate		- baudrate 		(e.g. 115200)
******************************************************************************/
void usartx_rxmin_init (u32 USARTx, u32 BaudRate)
{
	/* Set baud rate */
	usartx_setbaud (USARTx,BaudRate);	// Compute divider settings and load BRR

 	/* Setup CR2 ------------------------------------------------------------------- */
	/* After reset CR2 is 0x0000 and this is just fine */
	/* Set up CR1 (page 771) ------------------------------------------------------- */
	USART_CR1(USARTx) |= (USART_UE | USART_RXNE_INTERRUPT_ENABLE | USART_RX_ENABLE);// Set UE, RNEIE, RE 



	return;	
}

