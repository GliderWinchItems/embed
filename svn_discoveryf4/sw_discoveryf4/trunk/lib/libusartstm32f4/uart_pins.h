/******************************************************************************
* File Name          : uart_pins.h
* Date First Issued  : 11/06/2013
* Board              : Discovery F4 (F405 or F407)
* Description        : Edit and check pins for uart
*******************************************************************************/
#ifndef __UART_PINS
#define __UART_PINS


#include "libopencm3/stm32/memorymap.h"
#include "libopencm3/cm3/common.h"

/* IRQ number for USART/UART */
/* 0 = WWDG Window Watchdog interrupt 0x0000 0040 */

#define	UART1_IRQ_NUMBER	37
#define	UART2_IRQ_NUMBER	38	
#define	UART3_IRQ_NUMBER	39	
#define	UART4_IRQ_NUMBER	52	
#define	UART5_IRQ_NUMBER	53	
#define	UART6_IRQ_NUMBER	71	

struct UARTPINS
{
	u32 pclk;	// Supplies appropriate bus clock
	u32 irqnumber;	// Returns uart irq number
};

/******************************************************************************/
int uart_pins(u32 iuart, struct UARTPINS* pu);
 /* @brief	: Set up GPIO pins for UART (and enable port clocking)
 * @param	: p = pointer to register base
 * @param	: pu = pointer to struct with return values
 * @return	: Edit check--0 = OK; not OK - panic_leds(8)
******************************************************************************/	

#endif 

