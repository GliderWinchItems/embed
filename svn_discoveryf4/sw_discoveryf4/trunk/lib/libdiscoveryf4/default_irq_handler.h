/******************************************************************************
* File Name          : default_irq_handler.h
* Date First Issued  : 11/10/2013
* Board              : Discovery F4 (F405 or F407)
* Description        : Identify and trap interrupts without a handler
*******************************************************************************/

#ifndef __DEFAULT_IRQ_HANDLER
#define __DEFAULT_IRQ_HANDLER

#include "libopencm3/cm3/common.h"
#include "libopencm3/stm32/memorymap.h"

void Default_Handler(void);


#endif 

