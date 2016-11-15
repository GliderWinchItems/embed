/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : USART2_rxint.c 
* Hackor             : deh
* Date First Issued  : 10/12/2010 deh
* Description        : USART2 receive with char-by-char interrupts into line buffers
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

#include "../libmiscstm32/clocksetup.h"

#include "../libusartstm32/usartprotoprivate.h" // Subroutine prototypes for internal use
#include "../libusartstm32/commonbitband.h" 	// Bitband macros
#include "../libusartstm32/dmabitband.h" 	// Register defines for bit band use


/* This is the only variable placed in static variable area.  The
control block and the buffers are allocated in the heap area via
mymalloc which is a one-time only allocation, i.e. no 'free' etc. */
struct USARTCBR* pUSARTcbr2;


/*******************************************************************************
 * u16 USART2_rxint_init (u32 BaudRate,u16 RcvCircularSize, u16 GetLineSize);
 * @brief	: Initializes the USART2 to 8N1 and the baudrate for DMA circular
 *                buffering.
 * @param	: u32 BaudRate		- baud rate.				(e.g. 115200)
 * @param	: u16 RcvCircularSize 	- size of receive circular buffer 	(e.g. 128)
 * @param	: u16 GetLineSize 	- size of 'getline' buffer 		(e.g. 80)
 * @return	: 0 = success; 1 = fail malloc; 2 = RcvCircularSize = 0; 3 = GetLineSize = 0; 
 *******************************************************************************/
/* 
	Setup for DMA1 CCR registers (Reference manual 10.4.3 page 209)--
*/
/* NOTE:
The struct pointer usage:
char*	prx_now_i;	// Working:  pointer in line buffer currently being filled (interrupt)
char*	prx_begin_m;	// Working:  pointer to beginning of current line buffer (main)
char*	prx_begin_i;	// Constant: pointer to beginning of current line buffer (interrupt)
char*	prx_begin;	// Constant: pointer to beginning of first line buffer
har*	prx_end;	// Constant: pointer to end of last line buffer
u16*	prx_ctary_now_m;// Working:  pointer to count of chars in corresponding line buffer
u16*	prx_ctary_now_i;// Working:  pointer to count of chars in corresponding line buffer
u16*	prx_ctary_begin;// Constant: pointer to beginning of array of char counts
u16	rx_ln_sz;	// Constant: size of one line buffer
u16	rx_ln_ct;	// Constant: number of line buffers
*/

/* This is the only variable placed in static variable area.  The
control block and the buffers are allocated in the heap area via
mymalloc which is a one-time only allocation, i.e. no 'free' etc. */
struct USARTCBR* pUSARTcbr2;

u16 USART2_rxdma_init (u32 BaudRate,u16 RcvCircularSize, u16 GetLineSize)
{
	u16 temp;
	if ( (temp = usartx_rxdma_allocatebuffers(RcvCircularSize, GetLineSize, &pUSARTcbr2)) != 0) return temp;
		
	/* ---------------------- DMA and USART hardware setup --------------------- */	
	
	/* Enable clock for USART2. */
	RCC_APB1ENR |= RCC_APB1ENR_USART2EN;

	/* DMA1 is used with all three buffered USART routines */
	RCC_AHBENR |= RCC_AHBENR_DMA1EN;

	/* Setup USART baudrate, rx non-interrupting, and connected to DMA */
	usartx_rxdma_usart_init (USART2, BaudRate);

	/* Setup DMA for usart receive, circular buffering, no interrupts */
	DMA1_CNDTR6 = RcvCircularSize;			// Number of incoming chars before wrap-around
	DMA1_CMAR6 = (u32)pUSARTcbr2->prx_begin;	// DMA chan 6 memory rx circular buffer start address
	DMA1_CPAR6 = (u32)&USART2_DR;			// DMA chan 6 periperal usart rx address
	DMA_CCR6(DMA1) = DMA_CCR6_MINC | DMA_CCR6_CIRC | DMA_CCR6_EN;   // Chan 6 Rx Mem increment, Circular, Enable

	return 0;	// Return 0 = success!
}

/*#######################################################################################
 * No ISR routines needed!
 *####################################################################################### */

/* ---------------------- glorious end -------------------------------------------------- */
