/******************************************************************************
* File Name          : systick1.h
* Date First Issued  : 10/21/2010
* Description        : SYSTICK setup and related
*******************************************************************************/
// (Programming Manual, PM0056, page 152 for SYSTICK registers)


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SYSTICK1_H
#define __SYSTICK1_H

#include "libopencm3/stm32/f4/scb.h"

#define SYSTICK_PRIORITY 0X10			// Next to highest priority
//#define SCB_SHPR3 MMIO32(SCB_BASE + 0x0020) 	// System handler priority register 3

//#define SCB_SHCSR MMIO32(SCB_BASE + 0x0024) 	// System handler control and state register
#define SYSTICKACT	(1<<11)			// Bit 11 SYSTICKACT: SysTick exception active bit, reads as 1 if exception is active

//#define SCB_CFSR  MMIO32(SCB_BASE + 0x0028) 	// Configurable fault status register 
#define PENDSTCLR	(1<<25)			// Bit 25: Write 1 to clear SYSTICK exception)

/*******************************************************************************/
void SYSTICK_init(u32 systick_reloadct);
/* @brief	: Initializes SYSTICK for interrupting
 * @param	: systick_reloadct: Use N-1 for N bus ticks per interrupt
 * @return	: none
 *******************************************************************************/
u32 SYSTICK_getcount32(void);
/* @brief	: Gets 32 bit systick count
 * @param	: none
 * @return	: Get current 32 bit systick count
 *******************************************************************************/
unsigned long long SYSTICK_getcount64(void);
/* @brief	: Gets 64 bit systick count
 * @param	: none
 * @return	: Get current 64 bit systick count
 *******************************************************************************/
u32 SYSTICK_systickduration(u32 old_systick);
/* @brief	: Read SYSTICK counter, and compute difference from old reading
 * @param	: none
 * @return	: Difference between current counter reading and previous saved reading
 *******************************************************************************/
u32 SYSTICK_24bitdiff(u32 new_systick, u32 old_systick);
/* @brief	: Gets 32 bit systick count and returns difference from old
 * @param	: A previous systick reading
 * @return	: Return difference with wrap-around adjustment
 *******************************************************************************/
u32 SYSTICK_getcountdiv256(void);
/* @brief	: Gets 64 bit systick count shift right 8 bits
 * @param	: none
 * @return	: Get current systick/256
 *******************************************************************************/

extern volatile u8 systick_hibyte;	// High order byte for extending 24 bit SYSTICK count to 32
extern volatile u32 systick_u32hi;	// High order word for extending 32 bit count to 64 bits

#endif

