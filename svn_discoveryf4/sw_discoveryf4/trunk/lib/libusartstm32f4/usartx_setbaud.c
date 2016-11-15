/******************************************************************************
* File Name          : usartx_setbaud.c
* Date First Issued  : 11/06/2013
* Description        : Compute and set BRR register, given baud rate
*******************************************************************************/
#include "libopencm3/stm32/memorymap.h"
#include "libopencm3/cm3/common.h"
#include "libopencm3/stm32/f4/usart.h"
#include "usartx_setbaud.h"

/******************************************************************************
 * void usartx_setbaud (u32 usartx, u32 pclk_freq, u32 u32BaudRate);
 * @brief	: set baudrate register (BRR)
 * @param	: u32BaudRate is the baud rate.
******************************************************************************/
void usartx_setbaud (u32 usartx, u32 pclk_freq, u32 u32BaudRate)
{
	/* Configure the USART Baud Rate -------------------------------------------*/
	u32 tmpreg = 0x00;
	u32 fractionaldivider = 0x00;
	u32 integerdivider = 0x00;

	/* Determine the integer part */
	integerdivider = ((0x19 * pclk_freq) / (0x04 * u32BaudRate));
	tmpreg = (integerdivider / 0x64) << 0x04;

	/* Determine the fractional part */
	fractionaldivider = integerdivider - (0x64 * (tmpreg >> 0x04));
	tmpreg |= ((((fractionaldivider * 0x10) + 0x32) / 0x64)) & ((u8)0x0F);

	/* Set USART BRR --------------------------------------------------------------- */
	  USART_BRR(usartx) = (u16)tmpreg;
	return;
}

