/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : usartx_txcir_putc.c
* Hackor             : deh
* Date First Issued  : 10/14/2010
* Description        : Add a char to the circular tx buffer
*******************************************************************************/
#include "../libusartstm32/usartprotoprivate.h"
#include "../libusartstm32/usartall.h"
#include "../libusartstm32/usartbitband.h" 	// Register defines for bit band use


/*******************************************************************************
* void usartx_txcir_putc(char c, struct USARTCBT* pUSARTcbtx);
* @brief	: Put char.  Add a char to output buffer
* @param	: Char to be sent
* @param	: Pointer to struct control block for transmit
* @return	: none
*******************************************************************************/
void usartx_txcir_putc(char c, struct USARTCBT* pUSARTcbtx)
{
 	*pUSARTcbtx->ptx_now_m++ = c;	// Store a char

	/* Are we at the end of the buffer? */
	if (pUSARTcbtx->ptx_now_m >= pUSARTcbtx->ptx_end ) 
	{ // Here we hit the end of the line buffer, so reset to beginning
		pUSARTcbtx->ptx_now_m = pUSARTcbtx->ptx_begin;
	}
 	return ;	
}
