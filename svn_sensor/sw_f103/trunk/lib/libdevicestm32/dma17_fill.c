/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : dma17_fill.c
* Composer           : deh
* Date First Issued  : 01/03/2013
* Board              : STM32F103VxT6_pod_mm
* Description        : Fill a block of 32b words using DMA1 CH7
*******************************************************************************/
/*

Ref Man rev 14 p 267--
"The DMA controller performs direct memory transfer by sharing the system bus with the
CortexTM-M3 core. The DMA request may stop the CPU access to the system bus for some
bus cycles, when the CPU and DMA are targeting the same destination (memory or
peripheral). The bus matrix implements round-robin scheduling, thus ensuring at least half
of the system bus bandwidth (both to memory and peripheral) for the CPU."


*/
#include "libopenstm32/rcc.h"
#include "libopenstm32/dma.h"
#include "bit_banding.h"


/******************************************************************************
 * void dma17_fill_init(void);
 * @brief	: Initialize 
*******************************************************************************/
void dma17_fill_init(void)
{
	/* Setup DMA1 (p 265) */
	RCC_AHBENR |= RCC_AHBENR_DMA1EN;	// Enable DMA1 clock (p 109)

	// Channel configurion reg (p 278)
	//           mem2mem  | priority low | 32b mem xfrs| 32b p xfrs | mem inc    | read from m |   enable
	DMA1_CCR7 = (1 << 14) | ( 0x0 << 12) | (0x2 << 10) | (0x2 << 8) | (0x1 << 7) | (0x0 << 4)  | (0x0);

	return;
}
/******************************************************************************
 * int dma17_fill_busy(void);
 * @brief	: Check if DMA17 is busy
 * @return	: 0 = busy; not zero = transfer complete.
*******************************************************************************/
int dma17_fill_busy(void)
{
	return (DMA1_ISR & (1 << 25));
}
/******************************************************************************
 * void dma17_fill_(int * address, int * fill, unsigned short count);
 * @brief	: Start DMA17 filling at 'address', storing 'fill', for 'count' times
 * @param	: address--address of first memory location to be filled
 * @param	: fill--address of 32b word to be repetitively stored
 * @param	: count--number of words to stored
 * @return	: none
*******************************************************************************/
void dma17_fill(int * address, int * fill, unsigned short count)
{
	MMIO32_BIT_BAND(&DMA1_CCR7,0x00) = 0;	// Disable CH 7
	DMA1_IFCR &= ~(0xf << 24);		// Clear all CH7 flags
	DMA1_CMAR7 = (unsigned int)address;	// DMA1 chan 7 memory buffer start address
	DMA1_CPAR7 = (unsigned int)fill;	// DMA1 chan 7 periperal address
	DMA1_CNDTR7 = count;	// Number of data items
	MMIO32_BIT_BAND(&DMA1_CCR7,0x00) = 1;	// Enable CH7
	return ;
}

