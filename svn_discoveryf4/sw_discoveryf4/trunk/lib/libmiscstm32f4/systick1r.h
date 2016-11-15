/******************************************************************************
* File Name          : systick1r.h
* Date First Issued  : 10/21/2010
* Description        : Add some missing registers in libopenstm32/systick.h
*******************************************************************************/
// (Programming Manual, PM0056, page 135 for system registers)


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SYSTICK1R_H
#define __SYSTICK1R_H

#define SYSTICK_PRIORITY 0X10			// Next to highest priority
#define SCB_SHPR3 MMIO32(SCB_BASE + 0x0020) 	// System handler priority register 3

#define SCB_SHCSR MMIO32(SCB_BASE + 0x0024) 	// System handler control and state register
#define SYSTICKACT	(1<<11)			// Bit 11 SYSTICKACT: SysTick exception active bit, reads as 1 if exception is active

#define SCB_CFSR  MMIO32(SCB_BASE + 0x0028) 	// Configurable fault status register 
#define PENDSTCLR	(1<<25)			// Bit 25: Write 1 to clear SYSTICK exception)



#endif

