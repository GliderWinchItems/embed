/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : USART2_rxmin.c
* Hackor             : deh
* Date First Issued  : 10/03/2010 deh
* Description        : USART2 rx minimal; interrupt to store single char
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
   will cause a conflict if some other USART2 receive routine
   is called */
//struct USARTCBR* pUSARTcbr2;	// Receive

/* The all important flag and char */
volatile char USART2_rcvchar;	// Char received
volatile char USART2_rcvflag;	// Char ready flag

/*******************************************************************************
 * void USART2_rxmin_init(u32 BaudRate);
 * @brief	: Initializes the USARTx to 8N1 and the baudrate 
 * @param	: u32 Baudrate		- baudrate 		(e.g. 115200)
 * @return	: none
 *******************************************************************************/
void USART2_rxmin_init(u32 BaudRate)
{	
	// Enable clock for USART2.
	RCC_APB1ENR |= RCC_APB1ENR_USART2EN;

	/* Set up usart and baudrate */
	usartx_rxmin_init (USART2,BaudRate);

	/* Set and enable interrupt controller for USART2 */
	NVICIPR(NVIC_USART2_IRQ, USART_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_USART2_IRQ);			// Enable interrupt
	

	return;
}
/*#######################################################################################
 * ISR routine
 *####################################################################################### */
void USART2_IRQHandler(void)
{
	/* Receive */
	if (USART_SR(USART2) & USART_SR_RXNE)	// Receive register loaded?
	{  // Here, receive interrupt. 
		USART2_rcvchar = USART2_DR;	// Read and store char
		USART2_rcvflag += 1;		// Set flag to show main char is ready
	}
	return;
}

