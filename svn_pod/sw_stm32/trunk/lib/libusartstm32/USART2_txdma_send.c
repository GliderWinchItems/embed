/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : USART2_txdma_send.c
* Hackor             : deh
* Date First Issued  : 10/06/2010
* Description        : Step to the next line buffer, thus making this line buffer ready
*******************************************************************************/
#include "../libopenstm32/dma.h"

#include "../libusartstm32/usartprotoprivate.h"
#include "../libusartstm32/commonbitband.h" 	// Bitband macros
#include "../libusartstm32/dmabitband.h" 	// Register defines for bit band use

/*******************************************************************************
* void USART2_txdma_send(void);
* @brief	: Step to next line tx line buffer; if DMA not sending, start it now.
* @return	: none
*******************************************************************************/
void USART2_txdma_send(void)
{
	/* Common to all three USARTS */
	usartx_txdma_send(pUSARTcbt2);
					
	/* If DMA is idle, start it up, otherwise the DMA interrupt will pick up the 
           line buffer when it's done with the current (or lines) */
	if ((MEM_ADDR(BITBAND(DMA1CCR7,0))) == 0)			// Is DMA active?
	{ // Here, no
		DMA1_CMAR7  = (u32)pUSARTcbt2->ptx_begin_d; 	// Load DMA with addr of line buffer
		DMA1_CNDTR7 =     *pUSARTcbt2->ptx_ctary_now_d;	// Load DMA transfer count
		MEM_ADDR(BITBAND(DMA1CCR7,0)) = 0x01;		// Enale DMA

	}
	return;	
}

