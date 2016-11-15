/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : usartx_setbaud.c
* Hackor             : deh
* Date First Issued  : 10/04/2010 deh
* Description        : Compute and set BRR register, given baud rate
*******************************************************************************/
#include "../libopenstm32/usart.h"
#include "../libopenstm32/rcc.h"

#include "usartallproto.h"
#include "usartprotoprivate.h"

//#include "../clocksetup.h"

extern unsigned int	pclk1_freq;	// Freq (Hz) of APB1 bus (see ../lib/libmiscstm32/clockspecifysetup.c)
extern unsigned int	pclk2_freq;	// Freq (Hz) of APB2 bus (see ../lib/libmiscstm32/clockspecifysetup.c)

/******************************************************************************
 * void usartx_setbaud (u32 usartx, u32 u32BaudRate);
 * @brief	: set baudrate register (BRR)
 * @param	: u32BaudRate is the baud rate.
******************************************************************************/
void usartx_setbaud (u32 usartx, u32 u32BaudRate)
{
	/* Configure the USART Baud Rate -------------------------------------------*/
	u32 apbclock = 0x00;
	u32 tmpreg = 0x00;
	u32 fractionaldivider = 0x00;
	u32 integerdivider = 0x00;

	/* USART1 is driven from a different clock, than USART2,3 */
	if ( usartx == USART1_BASE)	// Are doing USART1?
	{  // Here, yes.
		apbclock = pclk2_freq;
	}
	else
	{ // Here, no.  It is presumed to be USART2, USART3, UART4, UART5
		apbclock = pclk1_freq;	
	}

	/* Determine the integer part */
	integerdivider = ((0x19 * apbclock) / (0x04 * u32BaudRate));
	tmpreg = (integerdivider / 0x64) << 0x04;

	/* Determine the fractional part */
	fractionaldivider = integerdivider - (0x64 * (tmpreg >> 0x04));
	tmpreg |= ((((fractionaldivider * 0x10) + 0x32) / 0x64)) & ((u8)0x0F);

	/* Set USART BRR --------------------------------------------------------------- */
	  USART_BRR(usartx) = (u16)tmpreg;
	return;
}

