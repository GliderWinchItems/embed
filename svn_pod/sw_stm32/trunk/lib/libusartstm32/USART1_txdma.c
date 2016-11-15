/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : USART1_txdma.c
* Hackor             : deh
* Date First Issued  : 10/07/2010 deh
* Description        : USART1 tx using DMA with line buffers
*******************************************************************************/

/* Strategy--
There is a circular array of line buffers that the mainline 
fills while the DMA sends to the USART.  The only interrupt is
from the DMA is when it has transfered an entire line buffer.

The main program fills a line buffer.  When the buffer is ready, or 
full 'puts' calls 'send' and the DMA is setup and enabled, 
unless it is already active.

The DMA sends chars to the usart tx from a line buffer.  When
the line has been transfered the DMA interrupt routines steps to
the next line buffer.  It that buffer is ready to be sent it
begins sending that buffer.  If it is not ready then the DMA
remains disabled, until the main program executes a 'send'.
*/


#include "../libopenstm32/usart.h"
#include "../libopenstm32/memorymap.h"
#include "../libopenstm32/rcc.h"
#include "../libopenstm32/nvic.h"
#include "../libopenstm32/gpio.h"
#include "../libopenstm32/rcc.h"
#include "../libopenstm32/dma.h"

#include "../libmiscstm32/clocksetup.h"

#include "../libusartstm32/usartprotoprivate.h" // Subroutine prototypes for internal use
#include "../libusartstm32/commonbitband.h" 	// Bitband macros
#include "../libusartstm32/dmabitband.h" 	// Register defines for bit band use
#include "../libusartstm32/nvicdirect.h" 	// Register macros

/* This is the only variable placed in static variable area.  The
control block and the buffers are allocated in the heap area via
mymalloc which is a one-time only allocation, i.e. no 'free' etc. */
struct USARTCBT* pUSARTcbt1;

/*******************************************************************************
 * u16 USART1_txdma_init (u32 BaudRate,u16 XmtLineSize, u16 NumberXmtLines);
 * @brief	: Initializes the USARTx to 8N1 and the baudrate for interrupting 
 * @param	: Baudrate 		- baudrate 			(e.g. 115200)
 * @param	: u16 XmtLineSize	- xmit line size, 		(e.g. 80)
 * @param	: u16 NumberXmtLines	- number of xmit line buffers, 	(e.g. 4)
 * @return	: 0 = success; 1 = fail malloc; 2 = XmtLineSize zero; 3 = NumberXmtLines < 2; 
 *******************************************************************************/
u16 USART1_txdma_init (u32 BaudRate,u16 XmtLineSize, u16 NumberXmtLines)
{
	u16 temp;
	/* Allocate memory for buffers and setup pointers in a control block for xmt */
	if ( (temp = usartx_txdma_allocatebuffers(XmtLineSize,NumberXmtLines,&pUSARTcbt1)) != 0) return temp;

	/* Setup GPIO pin for GPIO_USART1 tx (PA9) (See Ref manual, page 158) */
	GPIO_CRH(GPIOA) &= ~((0x000f ) << (4*1));	// Clear CNF reset bit 01 = Floating input (reset state)
	GPIO_CRH(GPIOA) |=  (( (GPIO_CNF_OUTPUT_ALTFN_PUSHPULL<<2) | (GPIO_MODE_OUTPUT_50_MHZ) ) << (4*1));
	
	// Enable clock for USART1.
	RCC_APB2ENR |= RCC_APB2ENR_USART1EN;

	/* DMA1 is used with all three buffered USART routines */
	RCC_AHBENR |= RCC_AHBENR_DMA1EN;

	/* Set up usart registers */
	usartx_txdma_usart_init (USART1,BaudRate);
		
	/* Set usart tx address into DMA channel periperhal address register  */
	DMA1_CPAR4 = (u32)&USART1_DR;			// DMA chan 4 periperal tx address

	/* The following bits remain fixed.  The ENable bit is turned off and on */
	DMA_CCR4(DMA1) = DMA_CCR4_MINC | DMA_CCR4_DIR  | DMA_CCR4_TCIE; // Ch 4 Tx Mem inc | Read from memory | Xfr complete interrupt

	/* Set and enable interrupt controller DMA */
//	nvic_set_priority(NVIC_DMA1_CHANNEL4_IRQ, DMA1_TX_PRIORITY);// Set interrupt priority 
//	nvic_enable_irq  (NVIC_DMA1_CHANNEL4_IRQ);	// Enable DMA1 chan 4 (tx) interrupt

	NVICIPR (NVIC_DMA1_CHANNEL4_IRQ, DMA1_TX_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_DMA1_CHANNEL4_IRQ);			// Enable interrupt

	return 0;
}

/*#######################################################################################
 * ISR routine
 *####################################################################################### */
/*******************************************************************************
* void DMA1CH4_IRQHandler(void) // USART1 Tx
* @brief	: Interrupt when line has been transfered to USART Tx.
* @param	: none
* @return	: none
*******************************************************************************/
void DMA1CH4_IRQHandler(void)
{
	/* Reset interrupt flag */
	DMA1_IFCR = DMA_ISR_GIF4;				// Reset interrupt flags, global, for this channel

	/* Advance pointers to line buffers and array of counts and reset when end reached */	
	usartx_txisr_advlnptr(pUSARTcbt1);

	/* Disable DMA so that it can be reloaded; exit if no more line buffers to send */
	MEM_ADDR(BITBAND(DMA1CCR4,0)) = 0x00;


	/* Are we caught up with the main line? */
	if (pUSARTcbt1->ptx_begin_d != pUSARTcbt1->ptx_begin_m) // main and dma pointing to same line buffer?
	{ // Here there are more buffers to send.  Set up DMA and enable.
		DMA1_CMAR4  =(u32)pUSARTcbt1->ptx_begin_d; 	// Load DMA with addr of line buffer
		DMA1_CNDTR4 =    *pUSARTcbt1->ptx_ctary_now_d;	// Load DMA transfer count
		MEM_ADDR(BITBAND(DMA1CCR4,0)) = 0x01;

	}
	return;	
}

