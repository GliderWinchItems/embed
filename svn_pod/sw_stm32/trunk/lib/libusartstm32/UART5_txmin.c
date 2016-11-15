/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : UART5_txmin.c
* Hackor             : deh
* Date First Issued  : 03/05/2013 deh
* Description        : UART5 tx minimal; single char loads into UART register
*******************************************************************************/

#include "../libopenstm32/usart.h"
#include "../libopenstm32/memorymap.h"
#include "../libopenstm32/gpio.h"
#include "../libopenstm32/rcc.h"

#include "../libusartstm32/usartprotoprivate.h"

/* The following is not used, but is placed here so that it
   will cause a conflict if some other USART1 receive routine
   is called */
struct USARTCBT* pUSARTcbt5;	// Receive


/*******************************************************************************
 * void U5ART1_txmin_init (u32 BaudRate);
 * @brief	: Initializes the USARTx to 8N1 and the baudrate for interrupting receive into line buffers
 * @param	: u32 Baudrate		- baudrate 				(e.g. 115200)
 *******************************************************************************/
void UART5_txmin_init (u32 BaudRate)
{

	/* Setup GPIO pin for GPIO_UART5 tx (PC12) (See Ref manual, page 158) */
	GPIO_CRH(GPIOC) &= ~((0x000f ) << (4*4));	// Clear CNF reset bit 01 = Floating input (reset state)
	GPIO_CRH(GPIOC) |=  (( (GPIO_CNF_OUTPUT_ALTFN_PUSHPULL<<2) | (GPIO_MODE_OUTPUT_50_MHZ) ) << (4*4));
	
	// Enable clock for UART5.
	RCC_APB1ENR |= RCC_APB1ENR_UART5EN;

	/* Set up usart and baudrate */
	usartx_txmin_init (UART5,BaudRate);
	
	return;
}

