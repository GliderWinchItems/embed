/******************************************************************************
* File Name          : nvic_dma_mgrf3.h
* Date First Issued  : 01/24/2015
* Board              : F373 F303
* Description        : Manage interrupt addresses and usage conflicts
*******************************************************************************/
#ifndef __NVIC_DMA_MGRF3
#define __NVIC_DMA_MGRF3

#include "libopencm3/cm3/common.h"
#include "libopencm3/stm32/memorymap.h"
#include "libopencm3/stm32/dma.h"

/* Interrupt vector positions for DMA|STREAM */
/* 0 = WWDG Window Watchdog interrupt 0x0000 0040 */
// DMA1
#define DMASTRM11	11	// DMA 1 Channel 1
#define DMASTRM12	12
#define DMASTRM13	13
#define DMASTRM14	14
#define DMASTRM15	15
#define DMASTRM16	16
#define DMASTRM17	17

// DMA2
#define DMASTRM21	56	// DMA 2 Channel 1
#define DMASTRM22	57
#define DMASTRM23	58
#define DMASTRM24	59
#define DMASTRM25	60


struct DMAINTJUMP
{
	void(*jmp)(u32*);	// Interrupt handler call
	u32* base;		// Control block address
};

/******************************************************************************/
int nvic_dma_channel_vector_addf3(void(*p)(u32*), u32* q, u32 dma_irq_number, u32 channel_number);
/* @brief	: Set dispatch address for DMA channel interrupts; check for usage conflicts
 * @param	: p = pointer to interrupt handler
 * @param	: q = pointer that is passed to the interrupt handler (e.g. control block address)
 * @param	: dma_irq_number = nvic irq number
 * @param	: channel_number = 1-7 for channel selected
 * @return	: 0 = OK; -1 = irq number out of range; -2 or less = channel already in use
******************************************************************************/
#endif 

