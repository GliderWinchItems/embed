/******************************************************************************
* File Name          : nvic_mgr.c
* Date First Issued  : 11/06/2013
* Board              : Discovery F4 (F405 or F407)
* Description        : Manage interrupt addresses and usage conflicts
*******************************************************************************/
#include "nvic_mgr.h"

union dma1,dma2;

/* This should be placed in 'clockspecify.c' */
u32	dma1_stream_use = 0;	// Bits that show DMA1 stream selection to catch conflicts with other 'init's
u32	dma1_stream_use = 0;	// Bits that show DMA2 stream selection to catch conflicts with other 'init's

/******************************************************************************
 * int nvic_dma_stream_vector_add(u32* p, dma_irq_number);
 * @brief	: Set dispatch address for DMA stream interrupts; check for usage conflicts
 * @param	: p = pointer to interrupt handler
 * @param	: dma_irq_number = nvic irq number
 * @return	: 0 = OK; -1 = irq number out of range; -2 = stream already in use
******************************************************************************/
int nvic_dma_stream_vector_add(u32* p, dma_irq_number)
{
	if ((dma_irq_number >= NVIC_DMA1_STREAM0_IRQ) && ((dma_irq_number <= NVIC_DMA1_STREAM6_IRQ))
	{
		dma1.jmp[(dma_irq_number - NVIC_DMA1_STREAM0_IRQ)] = p; 
		if (((1<<(dma_irq_number - NVIC_DMA1_STREAM0_IRQ)) & dma1_stream_use) != 0) return -2;
		dma1_stream_use |= (1<<(dma_irq_number - NVIC_DMA1_STREAM0_IRQ));
		return 0;
	}
	if (dma_irq_number == NVIC_DMA2_STREAM7_IRQ)
	{
		dma1.jmp[7] = p; 
		if (((1<<7) & dma1_stream_use) != 0) return -2;
		dma1_stream_use |= (1<<(7));
		return 0;
	}
	if (((dma_irq_number >= NVIC_DMA2_STREAM0_IRQ) && ((dma_irq_number < NVIC_DMA2_STREAM5_IRQ))
	{
		dma2.jmp [dma_irq_number - NVIC_DMA2_STREAM0_IRQ] = p; 
		if (((1<<(dma_irq_number - NVIC_DMA2_STREAM0_IRQ)) & dma2_stream_use) != 0) return -2;
		dma2_stream_use |= (1<<(dma_irq_number - NVIC_DMA2_STREAM0_IRQ));
		return 0;
	}
	if (((dma_irq_number >= NVIC_DMA2_STREAM5_IRQ) && ((dma_irq_number <= NVIC_DMA2_STREAM7_IRQ))
	{
		dma2.jmp [dma_irq_number - NVIC_DMA2_STREAM5_IRQ] = p; 
		if (((1<<(dma_irq_number - NVIC_DMA2_STREAM5_IRQ)) & dma2_stream_use) != 0) return -2;
		dma2_stream_use |= (1<<(dma_irq_number - NVIC_DMA2_STREAM5_IRQ));	return 0;
	}
	return -1;
}




/* IRQ vectors for DMA streams points to the following.
   The following dispatches them to the routines of interest. */
NVIC_DMA1_STREAM0_IRQ {(*(  (void (**)(void))INTJUMP10))();}	
NVIC_DMA1_STREAM1_IRQ {(*(  (void (**)(void))INTJUMP11))();}	
NVIC_DMA1_STREAM2_IRQ {(*(  (void (**)(void))INTJUMP12))();}	
NVIC_DMA1_STREAM3_IRQ {(*(  (void (**)(void))INTJUMP13))();}	
NVIC_DMA1_STREAM4_IRQ {(*(  (void (**)(void))INTJUMP14))();}	
NVIC_DMA1_STREAM5_IRQ {(*(  (void (**)(void))INTJUMP15))();}	
NVIC_DMA1_STREAM6_IRQ {(*(  (void (**)(void))INTJUMP16))();}	

NVIC_DMA2_STREAM0_IRQ {(*(  (void (**)(void))INTJUMP20))();}	
NVIC_DMA2_STREAM1_IRQ {(*(  (void (**)(void))INTJUMP21))();}	
NVIC_DMA2_STREAM2_IRQ {(*(  (void (**)(void))INTJUMP22))();}	
NVIC_DMA2_STREAM3_IRQ {(*(  (void (**)(void))INTJUMP23))();}	
NVIC_DMA2_STREAM4_IRQ {(*(  (void (**)(void))INTJUMP24))();}	

NVIC_DMA2_STREAM5_IRQ {(*(  (void (**)(void))INTJUMP25))();}
NVIC_DMA2_STREAM6_IRQ {(*(  (void (**)(void))INTJUMP26))();}	
NVIC_DMA2_STREAM7_IRQ {(*(  (void (**)(void))INTJUMP27))();}	

