/******************************************************************************
* File Name          : nvic_dma_mgr.c
* Date First Issued  : 11/06/2013
* Board              : Discovery F4 (F405 or F407)
* Description        : Manage interrupt addresses and usage conflicts
*******************************************************************************/
/* 
This routine handles dispatching DMA interrupts to DMA interrupt handlers.

A routine that uses DMA calls the appropriate routine to add the address of the
IRQ handler and a parameter that is passed to the IRQ handler.

This routine checks if the DMA stream has been used to catch two routines that
try to use the same DMA stream.

Note: DMA1 , or DMA2, clocking is enabled in this routine.
*/

#include "nvic_dma_mgr.h"
#include "libopencm3/stm32/f4/rcc.h"

static u8 dma1_stream_use = 0;	// Bits that show DMA1 streams 0-7 selections
static u8 dma2_stream_use = 0;	// Bits that show DMA2 streams 0-7 selections

/* Hold IRQ handler addresses, and a pointer to pass for each DMA stream. */
/* TODO initialize to Default_Handler to trap */
static struct DMAINTJUMP dma1[8];	
static struct DMAINTJUMP dma2[8];


/******************************************************************************
 * int nvic_dma_stream_vector_add(void(*p)(u32*), u32* q, u32 dma_irq_number, u32 stream_number);
 * @brief	: Set dispatch address for DMA stream interrupts; check for usage conflicts
 * @param	: p = pointer to interrupt handler
 * @param	: q = pointer that is passed to the interrupt handler (e.g. control block address)
 * @param	: dma_irq_number = nvic irq number
 * @param	: stream_number = 0-7 for stream selected
 * @return	: 0 = OK; -1 = irq number out of range; -2 or less = stream already in use
******************************************************************************/
int nvic_dma_stream_vector_add(void(*p)(u32*), u32* q, u32 dma_irq_number, u32 stream_number)
{
	if (stream_number > 7) return -1;	// JIC a bozo sent a bad number

	/* Convert DMA irq number (vector position) to an index 0 - 7 */
	// --------- DMA1 -----------
	if ((dma_irq_number >= DMASTRM10) && (dma_irq_number <= DMASTRM16))
	{
		if ((dma1_stream_use & (1 << stream_number)) != 0) return -2; 	// Already setup
		dma1_stream_use |=     (1 << stream_number);			// Set bit to show used
		dma1[(dma_irq_number - DMASTRM10)].jmp  = p; // Pointer to IRQ handler
		dma1[(dma_irq_number - DMASTRM10)].base = q; // Pointer to control block
		RCC_AHB1ENR |= (1<<21); // Enable DMA1 clocking
		return 0;
	}
	if (dma_irq_number == DMASTRM17)
	{
		if ((dma1_stream_use & 0x80) != 0) return -3; 	// Already setup
		dma1_stream_use |=     0x80;			// Set bit to show used
		dma1[7].jmp  = p; 
		dma1[7].base = q;
		RCC_AHB1ENR |= (1<<21); // Enable DMA1 clocking
		return 0;

	// --------- DMA2 -----------
	}
	if ( (dma_irq_number >= DMASTRM20) && (dma_irq_number <= DMASTRM24 ))
	{
		if ((dma2_stream_use & (1 << stream_number)) != 0) return -4; 	// Already setup
		dma2_stream_use |=     (1 << stream_number);			// Set bit to show used
		dma2[dma_irq_number - DMASTRM20].jmp  = p; 
		dma2[dma_irq_number - DMASTRM20].base = q;
		RCC_AHB1ENR |= (1<<22); // Enable DMA2 clocking
		return 0;
	}
	if ( (dma_irq_number >= DMASTRM25) && (dma_irq_number <= DMASTRM27 ))
	{
		if ((dma2_stream_use & (1 << stream_number)) != 0) return -5; 	// Already setup
		dma2_stream_use |=     (1 << stream_number);			// Set bit to show used
		dma2[dma_irq_number - DMASTRM25 + 5].jmp  = p; 
		dma2[dma_irq_number - DMASTRM25 + 5].base = q; 
		RCC_AHB1ENR |= (1<<22); // Enable DMA2 clocking
		return 0;
	}
	return -7;
}


/* IRQ vectors for DMA streams points to the following.
   The following dispatches them to the routines of interest, along with one 32b pointer. */
//                                (*(  (void (**)(void))APPJUMP)   )(            );


void DMA1_Stream0_IRQHandler (void){(*dma1[0].jmp)(dma1[0].base); return;} 
void DMA1_Stream1_IRQHandler (void){(*dma1[1].jmp)(dma1[1].base); return;}	
void DMA1_Stream2_IRQHandler (void){(*dma1[2].jmp)(dma1[2].base); return;}	
void DMA1_Stream3_IRQHandler (void){(*dma1[3].jmp)(dma1[3].base); return;}	
void DMA1_Stream4_IRQHandler (void){(*dma1[4].jmp)(dma1[4].base); return;}	
void DMA1_Stream5_IRQHandler (void){(*dma1[5].jmp)(dma1[5].base); return;}	
void DMA1_Stream6_IRQHandler (void){(*dma1[6].jmp)(dma1[6].base); return;}	
void DMA1_Stream7_IRQHandler (void){(*dma1[7].jmp)(dma1[7].base); return;}	

void DMA2_Stream0_IRQHandler (void){(*dma2[0].jmp)(dma2[0].base); return;}	
void DMA2_Stream1_IRQHandler (void){(*dma2[1].jmp)(dma2[1].base); return;}	
void DMA2_Stream2_IRQHandler (void){(*dma2[2].jmp)(dma2[2].base); return;}	
void DMA2_Stream3_IRQHandler (void){(*dma2[3].jmp)(dma2[3].base); return;}	
void DMA2_Stream4_IRQHandler (void){(*dma2[4].jmp)(dma2[4].base); return;}	

void DMA2_Stream5_IRQHandler (void){(*dma2[5].jmp)(dma2[5].base); return;}
void DMA2_Stream6_IRQHandler (void){(*dma2[6].jmp)(dma2[6].base); return;}	
void DMA2_Stream7_IRQHandler (void){(*dma2[7].jmp)(dma2[7].base); return;}	

