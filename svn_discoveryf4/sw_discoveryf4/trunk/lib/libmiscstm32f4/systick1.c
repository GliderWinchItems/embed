/******************************************************************************
* File Name          : systick1.c
* Date First Issued  : 03/01/2012
* Description        : SYSTICK setup and related
*******************************************************************************/
#include "../libopencm3/stm32/systick.h"
#include "../libopencm3/stm32/f4/nvic_f4.h"
#include "../libusartstm32f4/nvicdirect.h" 	// Register macros
#include "../libmiscstm32f4/systick1r.h"
/*******************************************************************************/
/* With a 8 MHx xtal and 72 MHz bus--
24 bits -  0.2330168889 seconds per cycle
32 bits - 59.6523235556 seconds per cycle
64 bits - 2,963,333.087980574 days per cycle
*/
u8 systick_hibyte = -1;	// Extend 24 bit to 32 bit count, this is the hi ord byte
u32 systick_u32hi = -1;	// Extend 32 bit count to 64 bits
/*******************************************************************************/

/*******************************************************************************
 * void SYSTICK_init(u32 systick_reloadct);
 * @brief	: Initializes SYSTICK for interrupting
 * @param	: systick_reloadct: Use N-1 for N bus ticks per interrupt
 * @return	: none
 *******************************************************************************/
void SYSTICK_init (u32 systick_reloadct)
{
	STK_CTRL = 0;	// Be sure it is disabled

	/* Bits 31:24 Reserved, must be kept cleared, page 150 */
	systick_reloadct &= 0x0fffffff;

	/* Zero is NG.  Set max: 16,277,216 bus ticks */
	if ((systick_reloadct == 0) || (systick_reloadct > 0x0fffffff))
		systick_reloadct = 0x0fffffff;	// This is max

	/* Set reload counter ticks per interrupt */
	STK_LOAD = (systick_reloadct);

	/* Reset current value */
	STK_VAL = 0;

//	*(u32*)0xE000ED20 = (SYSTICK_PRIORITY << 24);	// Set SYSTICK priority
	SCB_SHPR3 |= (SYSTICK_PRIORITY << 24);	// Set SYSTICK interrupt priority
	
	/* Clock runs at AHB bus speed, interrupts when reaching zero | interrupt enabled */
	STK_CTRL = (STK_CTRL_CLKSOURCE | STK_CTRL_TICKINT | STK_CTRL_ENABLE);

	return;
}
/*******************************************************************************
 * void SYSTICK_IRQHandler(void);
 * @brief	: Interrupt handler for extending tick counts to 64 bits
 * @param	: none
 * @return	: none
 *******************************************************************************/
void SYSTICK_IRQHandler (void)
{
	SCB_CFSR &= ~PENDSTCLR;	// Clear interrupt
	systick_hibyte -= 1;	// Extend count to 32 bits	
	if (systick_hibyte == 0)
		systick_u32hi -= 1; // Extend count to 64 bits
	return;
}


