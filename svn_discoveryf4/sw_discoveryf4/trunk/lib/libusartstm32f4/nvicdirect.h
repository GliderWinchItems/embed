/******************************************************************************
* File Name          : nvicdirect.h
* Date First Issued  : 11/04/2013
* Description        : nvic addresses for bit operations
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __NVICBITBAND_H
#define __NVICBITBAND_H

#include "../libopencm3/stm32/f4/memorymap.h"		// Use the libopenstm32 base defines
#include "commonbitband.h"	// Bitband macros

/* NVIC_BASE is picked up from memorymap.h */

/* Interrupt priority */
#define NVICIPR(irqnumber,priority)	( *(u32*)(NVIC_BASE + 0x300 + ((irqnumber / 4) * 4)) ) |= (priority << ((irqnumber % 4) * 8))

/* Interrupt set enable */
#define NVICISER(irqnumber) 	(*(u32*)(NVIC_BASE + 0x000 + (irqnumber/32)*4)) |= (1<<(irqnumber % 32))

/* Interrupt clear enable */
#define NVICICER(irqnumber) 	(*(u32*)(NVIC_BASE + 0x080 + (irqnumber/32)*4)) |= (1<<(irqnumber % 32))

/* Interrupt set pending */
#define NVICISPR(irqnumber) 	(*(u32*)(NVIC_BASE + 0x0100 + (irqnumber/32)*4)) |= (1<<(irqnumber % 32))

/* Interrupt clear pending */
#define NVICICPR(irqnumber) 	(*(u32*)(NVIC_BASE + 0x180 + (irqnumber/32)*4)) |= (1<<(irqnumber % 32))

/* Interrupt active bit register */
#define NVICIABR(irqnumber) 	(*(u32*)(NVIC_BASE + 0x200 + (irqnumber/32)*4)) |= (1<<(irqnumber % 32))




#endif

