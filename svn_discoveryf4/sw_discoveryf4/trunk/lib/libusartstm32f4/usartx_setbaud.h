/******************************************************************************
* File Name          : usartx_setbaud.h
* Date First Issued  : 11/06/2013
* Description        : Compute and set BRR register, given baud rate
*******************************************************************************/

#ifndef __USART_SETBAUD
#define __USART_SETBAUD

#include "libopencm3/cm3/common.h"
#include "libopencm3/stm32/memorymap.h"
#include "libopencm3/stm32/f4/dma_common_f24.h"


/******************************************************************************/
void usartx_setbaud (u32 usartx, u32 pclk_freq, u32 u32BaudRate);
/* @brief	: set baudrate register (BRR)
 * @param	: u32BaudRate is the baud rate.
******************************************************************************/
#endif 

