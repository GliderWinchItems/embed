/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : UART5_rxmin.c
* Hackor             : deh
* Date First Issued  : 03/05/2013 deh
* Description        : UART5 rx minimal; interrupt to store single char
*******************************************************************************/
/* Strategy--
USART receive interrupts and the char is stored in a single byte buffer and a
flag is incremented.  The mainline tests the flag to see if a char is ready and
removes the char and sets the flag to zero.  The flag is increment upon an 
interrupt so if the mainline misses a char the flag will be greater than one
when it is tested.
*/


#include "../libopenstm32/usart.h"
#include "../libopenstm32/memorymap.h"
#include "../libopenstm32/rcc.h"
#include "../libopenstm32/gpio.h"
#include "../libopenstm32/rcc.h"
#include "../libopenstm32/nvic.h"

#include "../libusartstm32/usartprotoprivate.h"
#include "../libusartstm32/usartall.h"
#include "../libusartstm32/nvicdirect.h" 	// Register macros
/* The following is not used, but is placed here so that it
   will cause a conflict if some other USART1 receive routine
   is called */
//struct USARTCBR* pUSARTcbr1;	// Receive

/* The all important flag and char */
volatile char UART5_rcvchar;	// Char received
volatile char UART5_rcvflag;	// Char ready flag

/*******************************************************************************
 * void UART5_rxmin_init(u32 BaudRate);
 * @brief	: Initializes the USARTx to 8N1 and the baudrate 
 * @param	: u32 Baudrate		- baudrate 		(e.g. 115200)
 * @return	: none
 *******************************************************************************/
void UART5_rxmin_init(u32 BaudRate)
{	
	// Enable clock for UART5.
	RCC_APB1ENR |= RCC_APB1ENR_UART5EN;

	/* Set up usart and baudrate */
	usartx_rxmin_init (UART5,BaudRate);

	/* Set and enable interrupt controller for USART1 */
	NVICIPR(NVIC_UART5_IRQ, USART_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_UART5_IRQ);			// Enable interrupt
	

	return;
}
/*#######################################################################################
 * ISR routine
 *####################################################################################### */
void UART5_IRQHandler(void)
{
	/* Receive */
	if (USART_SR(UART5) & USART_SR_RXNE)	// Receive register loaded?
	{  // Here, receive interrupt. 
		UART5_rcvchar = UART5_DR;	// Read and store char
		UART5_rcvflag += 1;		// Set flag to show main char is ready
	}
	return;
}

