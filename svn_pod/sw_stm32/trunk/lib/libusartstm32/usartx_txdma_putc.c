/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : Uusartx_txdma_putc.c 
* Hackor             : deh
* Date First Issued  : 10/06/2010 deh
* Description        : USART2 tx dma, add one char to the current line buffer
*******************************************************************************/
#include "../libusartstm32/usartprotoprivate.h"
#include "../libusartstm32/usartall.h"


/*******************************************************************************
* char usartx_txdma_putc(char c, struct USARTCBT* pUSARTcbtx);
* @brief	: Put char.  Add a char to output buffer
* @param	: Char to be sent
* @return	: == 0 for buffer did not overflow; == 1 overflow, chars lost
*******************************************************************************/
char usartx_dma_putc(char c, struct USARTCBT* pUSARTcbtx)
{
 	*pUSARTcbtx->ptx_now_m++ = c;	// Store a char, advance pointer
	if (pUSARTcbtx->ptx_now_m >= (pUSARTcbtx->ptx_begin_m + pUSARTcbtx->tx_ln_sz) ) 
	{
		pUSARTcbtx->ptx_now_m--; // Don't go beyond the end of the buffer
		return 1;		// Show caller we hit the end
	}
 	return 0;	
}
