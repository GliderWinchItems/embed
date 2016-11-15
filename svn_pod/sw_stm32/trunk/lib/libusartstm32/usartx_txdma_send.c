/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : usartx_txdma_send.c
* Hackor             : deh
* Date First Issued  : 10/06/2010
* Description        : Step to the next line buffer, thus making this line buffer ready
*******************************************************************************/
#include "../libusartstm32/usartprotoprivate.h"
#include "../libusartstm32/usartall.h"


/*******************************************************************************
* void usartx_txdma_send(struct USARTCBT* pUSARTcbtx);
* @brief	: Step to next line tx line buffer; if DMA not sending, start it now.
* @param	: pUSARTcbtx	Pointer to struct--pUSARTcb1,2,3
* @return	: none
*******************************************************************************/
void usartx_txdma_send(struct USARTCBT* pUSARTcbtx)
{
	/* Compute number of chars to send (less zero terminator) 
           and place in array that holds the counts */
	*pUSARTcbtx->ptx_ctary_now_m = pUSARTcbtx->ptx_now_m - pUSARTcbtx->ptx_begin_m;

	if (*pUSARTcbtx->ptx_ctary_now_m == 0) 
	{
		return;	// We should not be coming here!
/* If USARTx_txdma_send(); is executed in the mainline, then there is a possbility
that we end up here, since 'send()' can be executed twice, and this results in
testbusy never exiting. */
	}

	/* Advance pointers to the next line buffer.  */		
	usartx_txmain_advlnptr(pUSARTcbtx);
					
	return;	
}

