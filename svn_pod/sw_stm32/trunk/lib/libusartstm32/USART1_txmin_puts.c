/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : USART1_txmin_puts.c
* Hackor             : deh
* Date First Issued  : 10/04/2010 deh
* Description        : USART1 tx minimal; send zero terminated string
*******************************************************************************/
#include "../libusartstm32/usartall.h"
#include "../libusartstm32/usartallproto.h"
/*******************************************************************************
* void USART1_txmin_puts(char* p);
* @brief	: Send a zero terminated string
* @param	: Pointer to zero terminated string
* @return	: none
*******************************************************************************/
void USART1_txmin_puts(char* p)
{
	while ( *p != 0 ) USART1_txmin_putc(*p++); 
	return;
}

