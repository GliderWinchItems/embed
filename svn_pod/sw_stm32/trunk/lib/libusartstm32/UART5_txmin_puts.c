/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : UART5_txmin_puts.c
* Hackor             : deh
* Date First Issued  : 03/05/2013 deh
* Description        : UART5 tx minimal; send zero terminated string
*******************************************************************************/
#include "../libusartstm32/usartall.h"
#include "../libusartstm32/usartallproto.h"
/*******************************************************************************
* void UART5_txmin_puts(char* p);
* @brief	: Send a zero terminated string
* @param	: Pointer to zero terminated string
* @return	: none
*******************************************************************************/
void UART5_txmin_puts(char* p)
{
	while ( *p != 0 ) UART5_txmin_putc(*p++); 
	return;
}

