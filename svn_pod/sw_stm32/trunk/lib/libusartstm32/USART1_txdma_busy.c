/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : USART1_txdma_busy.c 
* Hackor             : deh
* Date First Issued  : 10/05/2010 deh
* Description        : Check that line buffer is available
*******************************************************************************/ 
#include "../libusartstm32/usartprotoprivate.h"
#include "../libusartstm32/usartall.h"
#include "../libopenstm32/dma.h"

/*******************************************************************************
* u16 USART1_txdma_busy(void);
* @brief	: Check that line buffer is available
* @return	: 1 = all line buffers filled; 0 = free line buffer(s)
*******************************************************************************/
u16 USART1_txdma_busy(void)
{	
/* If DMA is enabled (sending) AND the pointers for main and dma are both pointing to the same
   line buffer, then there the line buffer is not free, i.e. main has them all loaded.
*/
	if ( (((DMA1_CCR4) & DMA_CCR4_EN) == 1) && (pUSARTcbt1->ptx_begin_m == pUSARTcbt1->ptx_begin_d) )
		return 1;
	return 0;
}

