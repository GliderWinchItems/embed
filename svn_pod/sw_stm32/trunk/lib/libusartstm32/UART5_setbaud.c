/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : UART5_setbaud.c 
* Authorized         : deh
* Date Issued        : 03/28/2012
* Description        : Re-set baud for UART5
*******************************************************************************/ 
#include "../libopenstm32/usart.h"
#include "../libopenstm32/dma.h"
#include "../libusartstm32/usartprotoprivate.h"
#include "../libusartstm32/usartall.h"

/*******************************************************************************
* void USART2_setbaud(u32 Baudrate);
* @brief	: Set baud rate
*******************************************************************************/
void UART5_setbaud(u32 Baudrate)
{
	usartx_setbaud(UART5,Baudrate);
	return;
}

