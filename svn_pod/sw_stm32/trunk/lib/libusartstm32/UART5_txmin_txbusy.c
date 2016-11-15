/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : UART5_txmin_txbusy.c 
* Hackor             : deh
* Date First Issued  : 03/05/2013 deh
* Description        : UART5 tx minimal, check transmit register emtpy
*******************************************************************************/ 

#include "../libopenstm32/usart.h"
#include "../libusartstm32/usartallproto.h"
#include "../libusartstm32/usartall.h" 

/******************************************************************************
 * u16 UART5_txmin_txbusy(void);
 * @brief	: Check for USART transmit register empty
 * @return	: 0 = busy, transmit register NOT emtpy; non-zero = emtpy, ready for char
******************************************************************************/
u16 UART5_txmin_busy(void)
{
	return (USART_SR(UART5) & USART_FLAG_TXE);
}
