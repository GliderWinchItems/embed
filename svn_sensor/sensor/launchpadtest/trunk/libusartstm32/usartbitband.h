/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : usartbitband.h
* Hackor             : deh
* Date First Issued  : 10/13/2010
* Description        : usart addresses for bit banding operations
*******************************************************************************/
/*
07-14-2011 - Add UART4,5
*/


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USARTBITBAND_H
#define __USARTBITBAND_H

#include "../libopenstm32/usart.h"		// Use the libopenstm32 base defines
#include "../libusartstm32/commonbitband.h" 	// Bitband macros

/* USARTx is picked up from libopenstm32/usart.h (which refers to memorymap.h) */
#define USART1SR  (USART1 + 0x00)	// Status register
#define USART2SR  (USART2 + 0x00)	// Status register
#define USART3SR  (USART3 + 0x00)	// Status register
#define  UART4SR  ( UART4 + 0x00)	// Status register
#define  UART5SR  ( UART5 + 0x00)	// Status register

#define USART1DR  (USART1 + 0x04)	// Data register
#define USART2DR  (USART2 + 0x04)	// Data register
#define USART3DR  (USART3 + 0x04)	// Data register
#define  UART4DR  ( UART4 + 0x04)	// Data register
#define  UART5DR  ( UART5 + 0x04)	// Data register

#define USART1CR1 (USART1 + 0x0c)	// Control register 1
#define USART2CR1 (USART2 + 0x0c)	// Control register 1
#define USART3CR1 (USART3 + 0x0c)	// Control register 1
#define  UART4CR1 ( UART4 + 0x0c)	// Control register 1
#define  UART5CR1 ( UART5 + 0x0c)	// Control register 1
	
#define RXFLAG	5
#define TXFLAG	7


#endif

