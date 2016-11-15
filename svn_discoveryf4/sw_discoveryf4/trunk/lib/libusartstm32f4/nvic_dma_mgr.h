/******************************************************************************
* File Name          : nvic_dma_mgr.h
* Date First Issued  : 11/06/2013
* Board              : Discovery F4 (F405 or F407)
* Description        : Manage interrupt addresses and usage conflicts
*******************************************************************************/
#ifndef __NVIC_DMA_MGR
#define __NVIC_DMA_MGR

#include "libopencm3/cm3/common.h"
#include "libopencm3/stm32/memorymap.h"
#include "libopencm3/stm32/f4/dma_common_f24.h"

/* Interrupt vector positions for DMA|STREAM */
/* 0 = WWDG Window Watchdog interrupt 0x0000 0040 */
// DMA1
#define DMASTRM10	11
#define DMASTRM11	12
#define DMASTRM12	13
#define DMASTRM13	14
#define DMASTRM14	15
#define DMASTRM15	16
#define DMASTRM16	17
#define DMASTRM17	47

// DMA2
#define DMASTRM20	56
#define DMASTRM21	57
#define DMASTRM22	58
#define DMASTRM23	59
#define DMASTRM24	60
#define DMASTRM25	68
#define DMASTRM26	69
#define DMASTRM27	70


struct DMAINTJUMP
{
	void(*jmp)(u32*);	// Interrupt handler call
	u32* base;		// Control block address
};

/******************************************************************************/
int nvic_dma_stream_vector_add(void(*p)(u32*), u32* q, u32 dma_irq_number, u32 stream_number);
/* @brief	: Set dispatch address for DMA stream interrupts; check for usage conflicts
 * @param	: p = pointer to interrupt handler
 * @param	: q = pointer that is passed to the interrupt handler (e.g. control block address)
 * @param	: dma_irq_number = nvic irq number
 * @param	: stream_number = 0-7 for stream selected
 * @return	: 0 = OK; -1 = irq number out of range; -2 or less = stream already in use
******************************************************************************/
#endif 

