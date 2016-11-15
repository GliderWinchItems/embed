/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : USART1_rxmin.c
* Hackor             : deh
* Date First Issued  : 10/03/2010 deh
* Description        : USART1 rx minimal; interrupt to store single char
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
volatile char USART1_rcvchar;	// Char received
volatile char USART1_rcvflag;	// Char ready flag

/*******************************************************************************
 * void USART1_rxmin_init(u32 BaudRate);
 * @brief	: Initializes the USARTx to 8N1 and the baudrate 
 * @param	: u32 Baudrate		- baudrate 		(e.g. 115200)
 * @return	: none
 *******************************************************************************/
void USART1_rxmin_init(u32 BaudRate)
{	
	// Enable clock for USART1.
	RCC_APB2ENR |= RCC_APB2ENR_USART1EN;

	/* Set up usart and baudrate */
	usartx_rxmin_init (USART1,BaudRate);

	/* Set and enable interrupt controller for USART1 */
	NVICIPR(NVIC_USART1_IRQ, USART_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_USART1_IRQ);			// Enable interrupt
	

	return;
}
/*#######################################################################################
 * ISR routine
 *####################################################################################### */
void USART1_IRQHandler(void)
{
	/* Receive */
	if (USART_SR(USART1) & USART_SR_RXNE)	// Receive register loaded?
	{  // Here, receive interrupt. 
		USART1_rcvchar = USART1_DR;	// Read and store char
		USART1_rcvflag += 1;		// Set flag to show main char is ready
	}
	return;
}

