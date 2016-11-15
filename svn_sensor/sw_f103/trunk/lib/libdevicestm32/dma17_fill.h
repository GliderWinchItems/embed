/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : dma17_fill.h
* Composer           : deh
* Date First Issued  : 01/03/2013
* Board              : STM32F103VxT6_pod_mm
* Description        : Fill a block of 32b words using DMA1 CH7
*******************************************************************************/
#ifndef __DMA17_FILL
#define __DMA17_FILL
/*
*/

/******************************************************************************/
void dma17_fill_init(void);
/* @brief	: Initialize 
*******************************************************************************/
int dma17_fill_busy(void);
/* @brief	: Check if DMA17 is busy
 * @return	: 0 = busy; not zero = transfer complete.
*******************************************************************************/
void dma17_fill(int * address, int * fill, unsigned short count);
/* @brief	: Start DMA17 filling at 'address', storing 'fill', for 'count' times
 * @param	: address--address of first memory location to be filled
 * @param	: fill--address of 32b word to be repetitively stored
 * @param	: count--number of words to stored
 * @return	: none
*******************************************************************************/

#endif
