/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : USART2_setbaud.c 
* Authorized         : deh
* Date Issued        : 03/28/2012
* Description        : Re-set baud for USART2
*******************************************************************************/ 
#include "../libopenstm32/usart.h"
#include "../libopenstm32/dma.h"
#include "../libusartstm32/usartprotoprivate.h"
#include "../libusartstm32/usartall.h"

/*******************************************************************************
* void USART2_setbaud(u32 Baudrate);
* @brief	: Set baud rate
*******************************************************************************/
void USART2_setbaud(u32 Baudrate)
{
	usartx_setbaud(USART2,Baudrate);
	return;
}

