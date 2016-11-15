/******************************************************************************
* File Name          : nvic_dma_mgrf3.c
* Date First Issued  : 01/24/2015
* Board              : F373 F303
* Description        : Manage interrupt addresses and usage conflicts
*******************************************************************************/
/* 
This routine handles dispatching DMA interrupts to DMA interrupt handlers.

A routine that uses DMA calls the appropriate routine to add the address of the
IRQ handler and a parameter that is passed to the IRQ handler.

This routine checks if the DMA channel has been used to catch two routines that
try to use the same DMA channel.

Note: DMA1 , or DMA2, clocking is enabled in this routine.
*/

#include "nvic_dma_mgrf3.h"
#include "libopencm3/stm32/rcc.h"

static u8 dma1_channel_use = 0;	// Bits that show DMA1 channels 0-7 selections
static u8 dma2_channel_use = 0;	// Bits that show DMA2 channels 0-5 selections

/* Hold IRQ handler addresses, and a pointer to pass for each DMA channel. */
/* TODO initialize to Default_Handler to trap */
static struct DMAINTJUMP dma1[7];	
static struct DMAINTJUMP dma2[5];


/******************************************************************************
 * int nvic_dma_channel_vector_addf3(void(*p)(u32*), u32* q, u32 dma_irq_number, u32 channel_number);
 * @brief	: Set dispatch address for DMA channel interrupts; check for usage conflicts
 * @param	: p = pointer to interrupt handler
 * @param	: q = pointer that is passed to the interrupt handler (e.g. control block address)
 * @param	: dma_irq_number = nvic irq number
 * @param	: channel_number = 1-7 for channel selected
 * @return	: 0 = OK; -1 = irq number out of range; -2 or less = channel already in use
******************************************************************************/
int nvic_dma_channel_vector_addf3(void(*p)(u32*), u32* q, u32 dma_irq_number, u32 channel_number)
{

	/* Convert DMA irq number (vector position) to an index 0 - 7 */
	// --------- DMA1 -----------
	if ((dma_irq_number >= DMASTRM11) && (dma_irq_number <= DMASTRM17))
	{
		if (channel_number > 7) return -1;	// DMA1 is CH1-7
		if ((dma1_channel_use & (1 << (channel_number-1))) != 0) return -2; 	// Already setup
		dma1_channel_use |=     (1 << (channel_number-1));			// Set bit to show used
		dma1[(dma_irq_number - DMASTRM11)].jmp  = p; // Pointer to IRQ handler
		dma1[(dma_irq_number - DMASTRM11)].base = q; // Pointer to control block
		RCC_AHBENR |= RCC_AHBENR_DMA1EN; // Enable DMA1 clocking
		return 0;
	}

	// --------- DMA2 -----------
	if ( (dma_irq_number >= DMASTRM21) && (dma_irq_number <= DMASTRM25 ))
	{
		if (channel_number > 5) return -1;	// DMA2 is CH1-5
		if ((dma2_channel_use & (1 << (channel_number-1))) != 0) return -4; 	// Already setup
		dma2_channel_use |=     (1 << (channel_number-1));			// Set bit to show used
		dma2[dma_irq_number - DMASTRM21].jmp  = p; 
		dma2[dma_irq_number - DMASTRM21].base = q;
		RCC_AHBENR |= RCC_AHBENR_DMA2EN; // Enable DMA2 clocking
		return 0;
	}
	return -7;
}


/* IRQ vectors for DMA channels points to the following.
   The following dispatches them to the routines of interest, along with one 32b pointer. */
//                                (*(  (void (**)(void))APPJUMP)   )(            );


void DMA1CH1_IRQHandler (void){(*dma1[0].jmp)(dma1[0].base); return;} 
void DMA1CH2_IRQHandler (void){(*dma1[1].jmp)(dma1[1].base); return;}	
void DMA1CH3_IRQHandler (void){(*dma1[2].jmp)(dma1[2].base); return;}	
void DMA1CH4_IRQHandler (void){(*dma1[3].jmp)(dma1[3].base); return;}	
void DMA1CH5_IRQHandler (void){(*dma1[4].jmp)(dma1[4].base); return;}	
void DMA1CH6_IRQHandler (void){(*dma1[5].jmp)(dma1[5].base); return;}	
void DMA1CH7_IRQHandler (void){(*dma1[6].jmp)(dma1[6].base); return;}	

void DMA2CH1_IRQHandler (void){(*dma2[0].jmp)(dma2[0].base); return;}	
void DMA2CH2_IRQHandler (void){(*dma2[1].jmp)(dma2[1].base); return;}	
void DMA2CH3_IRQHandler (void){(*dma2[2].jmp)(dma2[2].base); return;}	
void DMA2CH4_IRQHandler (void){(*dma2[3].jmp)(dma2[3].base); return;}	
void DMA2CH5_IRQHandler (void){(*dma2[4].jmp)(dma2[4].base); return;}	


