/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : interrupt_priority.h
* Hackerees          : deh
* Date First Issued  : 11/10/2012
* Board              : STM32F103VxT6_pod_mm
* Description        : interrupt priority settings
*******************************************************************************/

#ifndef __INTERRUPT_PRIORITY
#define __INTERRUPT_PRIORITY

/* We don't have TIM6 on this processor so we will use the interrupt */
#define TIM6_IRQ_PRIORITY	0xE0	// Interrupt priority for TIM6


#endif 

