/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : USART1_txmin.c
* Hackor             : deh
* Date First Issued  : 10/03/2010 deh
* Description        : USART1 tx minimal; single char loads into USART register
*******************************************************************************/

/*
10/02/2010 deh
*/

/* Strategy--
A line buffer is filled with incoming chars until an END_OF_LINE char
is received.  When EOL is received the char count is stored in an array
and the next line buffer is selected.

The mainline program checks for data with 'getcount'.  If the count is 
not zero then the program gets a pointer to the line with 'getline'.
When the program is finished with using the line it calls 'getnextline'
to step to the next available buffer (additional calls to 'getnextline'
when main's pointer has caught up with the interrupt pointer results in
no action).
*/


#include "../libopenstm32/usart.h"
#include "../libopenstm32/memorymap.h"
#include "../libopenstm32/gpio.h"
#include "../libopenstm32/rcc.h"

#include "../libusartstm32/usartprotoprivate.h"

/* The following is not used, but is placed here so that it
   will cause a conflict if some other USART1 receive routine
   is called */
struct USARTCBT* pUSARTcbt1;	// Receive


/*******************************************************************************
 * void USART1_txmin_init (u32 BaudRate);
 * @brief	: Initializes the USARTx to 8N1 and the baudrate for interrupting receive into line buffers
 * @param	: u32 Baudrate		- baudrate 				(e.g. 115200)
 *******************************************************************************/
void USART1_txmin_init (u32 BaudRate)
{

	/* Setup GPIO pin for GPIO_USART1 tx (PA9) (See Ref manual, page 158) */
	GPIO_CRH(GPIOA) &= ~((0x000f ) << (4*1));	// Clear CNF reset bit 01 = Floating input (reset state)
	GPIO_CRH(GPIOA) |=  (( (GPIO_CNF_OUTPUT_ALTFN_PUSHPULL<<2) | (GPIO_MODE_OUTPUT_50_MHZ) ) << (4*1));
	
	// Enable clock for USART1.
	RCC_APB2ENR |= RCC_APB2ENR_USART1EN;

	/* Set up usart and baudrate */
	usartx_txmin_init (USART1,BaudRate);
	
	return;
}

