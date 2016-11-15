/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : dmabitband.h
* Hackor             : deh
* Date First Issued  : 10/07/2010
* Description        : dma addresses for bit banding operations
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DMABITBAND_H
#define __DMABITBAND_H

#include "../libopenstm32/dma.h"	// Use the libopenstm32 base defines
#include "../libusartstm32/commonbitband.h" // Bitband macros

/* DMA1 is picked up from libopenstm32/dma.h */
#define DMA1CCR1 (DMA1 + 0x08 + 0x14 * 0)	// DMA CCR1 register address
#define DMA1CCR2 (DMA1 + 0x08 + 0x14 * 1)	// DMA CCR2 register address
#define DMA1CCR3 (DMA1 + 0x08 + 0x14 * 2)	// DMA CCR3 register address
#define DMA1CCR4 (DMA1 + 0x08 + 0x14 * 3)	// DMA CCR4 register address
#define DMA1CCR5 (DMA1 + 0x08 + 0x14 * 4)	// DMA CCR5 register address
#define DMA1CCR6 (DMA1 + 0x08 + 0x14 * 5)	// DMA CCR6 register address
#define DMA1CCR7 (DMA1 + 0x08 + 0x14 * 6)	// DMA CCR7 register address



#define DMA1EN	0		// Bit 0 = enable


#endif

