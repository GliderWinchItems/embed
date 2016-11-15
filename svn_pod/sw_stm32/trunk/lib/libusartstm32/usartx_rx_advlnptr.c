/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : usartx_rx_advlnptr.c 
* Hackor             : deh
* Date First Issued  : 10/09/2010 deh
* Description        : Advance pointers for line buffered receive (dma or int)
*******************************************************************************/ 
#include "../libusartstm32/usartprotoprivate.h"
#include "../libusartstm32/usartall.h"


/******************************************************************************
 * void usartx_rxmain_advlnptr (struct USARTCBR* pUSARTcbrx);
 * @brief	: Advance line pointers for main
******************************************************************************/
void usartx_rxmain_advlnptr (struct USARTCBR* pUSARTcbrx)
{
	/* If no chars, then we are caught up with incoming lines */
	if (pUSARTcbrx->prx_ctary_now_m == 0) return;

	/* Advance to next line buffer and companion array of char cts */
	pUSARTcbrx->prx_begin_m     += pUSARTcbrx->rx_ln_sz;	// Line buffer
	pUSARTcbrx->prx_ctary_now_m += sizeof(u16);	// Array of char cts

	/* Check if at the end of all the line buffers */
	if (pUSARTcbrx->prx_begin_m >= pUSARTcbrx->prx_end)
	{ // Here, end of line buffers, reset pointers to beginning
		pUSARTcbrx->prx_begin_m     = pUSARTcbrx->prx_begin;	// Line buffer
		pUSARTcbrx->prx_ctary_now_m = pUSARTcbrx->prx_ctary_begin;// Array of char cts
	}
	return;
}
/******************************************************************************
 * void usartx_rxisr_advlnptr (struct USARTCBR* pUSARTcbrx);
 * @brief	: Advance line pointers for isr
******************************************************************************/
void usartx_rxisr_advlnptr (struct USARTCBR* pUSARTcbrx)
{
	/* If no chars, then we are caught up with incoming lines */
	if (pUSARTcbrx->prx_ctary_now_i == 0) return;

	/* Advance to next line buffer and companion array of char cts */
	pUSARTcbrx->prx_begin_i     += pUSARTcbrx->rx_ln_sz;	// Line buffer
	pUSARTcbrx->prx_ctary_now_i += sizeof(u16);	// Array of char cts

	/* Check if at the end of all the line buffers */
	if (pUSARTcbrx->prx_begin_i >= pUSARTcbrx->prx_end)
	{ // Here, end of line buffers, reset pointers to beginning
		pUSARTcbrx->prx_begin_i     = pUSARTcbrx->prx_begin;	// Line buffer
		pUSARTcbrx->prx_ctary_now_i = pUSARTcbrx->prx_ctary_begin;// Array of char cts
	}
	return;
}

