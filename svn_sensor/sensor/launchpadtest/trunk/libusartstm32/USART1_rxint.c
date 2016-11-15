/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : USART1_rxint.c 
* Hackor             : deh
* Date First Issued  : 10/12/2010 deh
* Description        : USART1 receive with char-by-char interrupts into line buffers
*******************************************************************************/ 

/* Progression
09/29/2010 deh Pulled out common code, allocate buffers in myheap using 'mymalloc'
*/

/* Strategy--

Receive
The usart is setup to receive with char-by-char interrupting into an array of line buffers.
A line buffer is filled with incoming chars until a END_OF_LINE char is received (END_OF_LINE 
is defined in the usartall.h file).

Calls to 'getline' are used to retrieve lines; a line designated by a terminator byte (e.g.
0x0d, .  'getline' returns zero when the line has 
not been completed.  A return of non-zero means a line is in the line buffer, and the 
return is a pointer to the beginning of that line.

*/

#include "../libopenstm32/usart.h"
#include "../libopenstm32/memorymap.h"
#include "../libopenstm32/rcc.h"
#include "../libopenstm32/nvic.h"
#include "../libopenstm32/gpio.h"
#include "../libopenstm32/rcc.h"
#include "../libopenstm32/dma.h"

//#include "../libmiscstm32/clocksetup.h"

#include "../libusartstm32/usartprotoprivate.h" // Subroutine prototypes for internal use
#include "../libusartstm32/commonbitband.h" 	// Bitband macros
//#include "../libusartstm32/dmabitband.h" 	// Register defines for bit band use
#include "../libusartstm32/nvicdirect.h" 	// Register macros


/* This is the only variable placed in static variable area.  The
control block and the buffers are allocated in the heap area via
mymalloc which is a one-time only allocation, i.e. no 'free' etc. */
struct USARTCBR* pUSARTcbr1;

/*******************************************************************************
 * u16 USART1_rxint_init (u32 BaudRate,u16 RcvLineBufferSize, u16 RcvNumberLineBuffers);
 * @brief	: Initializes the USART1 to 8N1 and the baudrate for DMA circular
 *                buffering.
 * @param	: u32 BaudRate		- baud rate.				(e.g. 115200)
 * @param	: u16 RcvLineBufferSize 	- size of one line buffer 	(e.g. 48)
 * @param	: u16 RcvNumberLineBuffers	- number of line buffers 	(e.g. 4)
 * @return	: 0 = success; 1 = fail malloc; 2 = RcvLineBufferSize = 0; 3 = RcvNumberLineBuffers = 0; 
 *******************************************************************************/
/* 
	Setup for DMA1 CCR registers (Reference manual 10.4.3 page 209)--
*/


/* This is the only variable placed in static variable area.  The
control block and the buffers are allocated in the heap area via
mymalloc which is a one-time only allocation, i.e. no 'free' etc. */
struct USARTCBR* pUSARTcbr1;

u16 USART1_rxint_init (u32 BaudRate,u16 RcvLineBufferSize, u16 RcvNumberLineBuffers)
{
	u16 temp;
	if ( (temp = usartx_rxint_allocatebuffers(RcvLineBufferSize, RcvNumberLineBuffers, &pUSARTcbr1)) != 0) return temp;
		
	/* ---------------------- DMA and USART hardware setup --------------------- */	
	
	/* Enable clock for USART1. */
	RCC_APB2ENR |= RCC_APB2ENR_USART1EN;

	/* Setup USART baudrate, rx non-interrupting, and connected to DMA */
	usartx_rxint_usart_init (USART1, BaudRate);

	/* Set and enable interrupt controller USART */
	NVICIPR(NVIC_USART1_IRQ, USART_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_USART1_IRQ);			// Enable interrupt

	return 0;	// Return 0 = success!
}

/*#######################################################################################
 * ISR routine
 *####################################################################################### */
void USART1_IRQHandler(void)
{
	/* Receive */
	if (USART_SR(USART1) & USART_FLAG_RXNE)	// Receive register loaded?
	{  // Here, receive interrupt. 
		*pUSARTcbr1->prx_now_i = USART_DR(USART1);	// Read and store char

		/* Advance pointers to line buffers and array of counts and reset when end reached */	
		usartx_rxint_rxisrptradv(pUSARTcbr1);	// Advance pointers common routine
	}

	return;
}

/* ---------------------- glorious end -------------------------------------------------- */
