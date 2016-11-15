/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : usart2_txint.c
* Hackor             : deh
* Date First Issued  : 10/03/2010 deh
* Description        : USART2 tx interrupting using line buffers
*******************************************************************************/

/*
10/02/2010 deh
*/

/* Strategy--
The only buffering is the USART transmit register.  A test for
the register not empty can be made.  putc checks the register
and loops if it is not empty.
*/


#include "../libopenstm32/usart.h"
#include "../libopenstm32/memorymap.h"
#include "../libopenstm32/rcc.h"
#include "../libopenstm32/gpio.h"
#include "../libopenstm32/rcc.h"

#include "../libusartstm32/usartproto.h"
#include "../libusartstm32/usartprivate.h"
#include "../libusartstm32/usartall.h"


/* The following is not used, but is place here so that it
   will cause a conflict if some other USART2 transmit routine
   is called */
struct USARTCBT* pUSARTcbt2;	// Receive

/*******************************************************************************
 * void USART2_txmin_init (u32 BaudRate);
 * @brief	: Initializes the USARTx to 8N1 and the baudrate for interrupting receive into line buffers
 * @param	: u32 Baudrate		- baudrate 				(e.g. 115200)
 *******************************************************************************/
void USART2_txmin_init (u32 BaudRate)
{

	/* Setup GPIO pin for GPIO_USART2 tx (PA2) (See Ref manual, page 157) */
	GPIO_CRL(GPIOA) &= ~((0x000f ) << (4*2));	// Clear CNF reset bit 01 = Floating input (reset state)
	GPIO_CRL(GPIOA) |=  (( (GPIO_CNF_OUTPUT_ALTFN_PUSHPULL<<2) | (GPIO_MODE_OUTPUT_50_MHZ) ) << (4*2));
	
	// Enable clock for USART2.
	RCC_APB1ENR |= RCC_APB1ENR_USART2EN;

	/* Set up usart and baudrate */
	usartx_txmin_init (usartx,BaudRate);
	
	return;
}

