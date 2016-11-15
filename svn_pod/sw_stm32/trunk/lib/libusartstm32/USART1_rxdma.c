/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : USART1_rxdma.c 
* Hackor             : deh
* Date First Issued  : 10/11/2010 deh
* Description        : USART1 receive with no interrupts, using DMA and circular buffer
*******************************************************************************/ 

/* Progression
09/29/2010 deh Pulled out common code, allocate buffers in heap using 'mymalloc'
*/

/* Strategy--

Receive
The usart is setup to receive without interrupt servicing.  The DMA is setup for circular
buffer mode.  It is given the start of the ecircular buffer and the size count.
When incoming chars are received they store in the circular buffer without any
interrupt servicing.

For retrieving chars there is a pointer into the circular buffer.  When this pointer equals
the position of the DMA storing (computed by reading the DMA count down register and
subtracting that from the buffer end address) is the same, all the chars have been retrieved.
A negative difference of the pointers means there is a wrap-around situation, and the
difference is adjusted by the size of the buffer.

To retrieve the chars the mainline program calls 'getcount' to determine if any chars are
buffered.  'getchar' returns a single char.  If 'getcount' is not called before 'getchar'
and there are no chars ready 'getchar' hangs in a loop waiting for chars.

Calls to 'getline' are used to retrieve lines; a line designated by a terminator byte (e.g.
0x0d, defined as END_OF_LINE in the .h file).  'getline' returns zero when the line has 
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
struct USARTCBR* pUSARTcbr1;


/*******************************************************************************
 * u16 USART1_rxdma_init (u32 BaudRate,u16 RcvCircularSize, u16 GetLineSize);
 * @brief	: Initializes the USART1 to 8N1 and the baudrate for DMA circular
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
char*	prx_now_i;	// Working:  pointer in circular rx buffer
char*	prx_begin_m;	// Working:  pointer to getline buffer
char*	prx_begin_i;	// Constant: pointer to getline buffer begin
char*	prx_begin;	// Constant: pointer to beginning of circular line buffer
har*	prx_end;	// Constant: pointer to end of rx circular buffer
u16*	prx_ctary_now_m;// Working:  --not used--
(char*)	prx_ctary_now_i;// Constant: pointer to getline buffer end 
u16*	prx_ctary_begin;// Constant: --not used--
u16	rx_ln_sz;	// Constant: size of circular line buffer  
u16	rx_ln_ct;	// Constant: size of getline buffer
*/

/* This is the only variable placed in static variable area.  The
control block and the buffers are allocated in the heap area via
mymalloc which is a one-time only allocation, i.e. no 'free' etc. */
struct USARTCBR* pUSARTcbr1;

u16 USART1_rxdma_init (u32 BaudRate,u16 RcvCircularSize, u16 GetLineSize)
{
	u16 temp;
	if ( (temp = usartx_rxdma_allocatebuffers(RcvCircularSize, GetLineSize, &pUSARTcbr1)) != 0) return temp;
		
	/* ---------------------- DMA and USART hardware setup --------------------- */	
	
	/* Enable clock for USART1. */
	RCC_APB2ENR |= RCC_APB2ENR_USART1EN;

	/* DMA1 is used with all three buffered USART routines */
	RCC_AHBENR |= RCC_AHBENR_DMA1EN;

	/* Setup USART baudrate, rx non-interrupting, and connected to DMA */
	usartx_rxdma_usart_init (USART1, BaudRate);

	/* Setup DMA for usart receive, circular buffering, no interrupts */
	DMA1_CNDTR5 = RcvCircularSize;			// Number of incoming chars before wrap-around
	DMA1_CMAR5 = (u32)pUSARTcbr1->prx_begin;	// DMA chan 5 memory rx circular buffer start address
	DMA1_CPAR5 = (u32)&USART1_DR;			// DMA chan 5 periperal usart rx address
	DMA_CCR5(DMA1) = DMA_CCR5_MINC | DMA_CCR5_CIRC | DMA_CCR5_EN;   // Chan 5 Rx Mem increment, Circular, Enable

	return 0;	// Return 0 = success!
}

/*#######################################################################################
 * No ISR routines needed!
 *####################################################################################### */

/* ---------------------- glorious end -------------------------------------------------- */
