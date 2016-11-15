/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : usartx_tx_advlnptr.c
* Hackor             : deh
* Date First Issued  : 10/17/2010
* Description        : Advance pointers for line buffered transmit (dma or int)
*******************************************************************************/
#include "../libusartstm32/usartprotoprivate.h"
#include "../libusartstm32/usartall.h"


/******************************************************************************
 * void usartx_txmain_advlnptr (struct USARTCBT* pUSARTcbtx);
 * @brief	: Advance line buffer pointers for main
******************************************************************************/
void usartx_txmain_advlnptr (struct USARTCBT* pUSARTcbtx)
{
	/* Advance to next line buffer and companion array of char cts */
	pUSARTcbtx->ptx_begin_m     += pUSARTcbtx->tx_ln_sz;	// Line buffer
	pUSARTcbtx->ptx_ctary_now_m += 1;	// Array of char cts

	/* Check if at the end of all the line buffers */
	if (pUSARTcbtx->ptx_begin_m >= pUSARTcbtx->ptx_end)
	{ // Here, end of line buffers, reset pointers to beginning
		pUSARTcbtx->ptx_begin_m     = pUSARTcbtx->ptx_begin;	// Line buffer
		pUSARTcbtx->ptx_ctary_now_m = pUSARTcbtx->ptx_ctary_begin;// Array of char cts
	}
	pUSARTcbtx->ptx_now_m = pUSARTcbtx->ptx_begin_m;		// Set new working-within-line pointer

	return;
}
/******************************************************************************
 * void usartx_txisr_advlnptr (struct USARTCBT* pUSARTcbtx);
 * @brief	: Advance line buffer pointers for isr
******************************************************************************/
void usartx_txisr_advlnptr (struct USARTCBT* pUSARTcbtx)
{
	/* Advance to next line buffer and companion array of char cts */
	pUSARTcbtx->ptx_begin_d     += pUSARTcbtx->tx_ln_sz;	// Line buffer
	pUSARTcbtx->ptx_ctary_now_d += 1;		// Array of char cts

	/* Check if at the end of all the line buffers */
	if (pUSARTcbtx->ptx_begin_d >= pUSARTcbtx->ptx_end)
	{ // Here, end of line buffers, reset pointers to beginning
		pUSARTcbtx->ptx_begin_d     = pUSARTcbtx->ptx_begin;	// Line buffer
		pUSARTcbtx->ptx_ctary_now_d = pUSARTcbtx->ptx_ctary_begin;// Array of char cts
	}
	pUSARTcbtx->ptx_now_d = pUSARTcbtx->ptx_begin_d;		// Set new working-within-line pointer

	return;
}

