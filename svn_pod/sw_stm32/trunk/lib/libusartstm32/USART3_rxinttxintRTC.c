/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : USART3_rxinttxintRTC.c
* Hackor             : deh
* Date First Issued  : 10/17/2010 deh
* Description        : USART3 rx & tx char-by-char interrupting using line buffers
*******************************************************************************/
/* RTC suffix--(07-15-2011)
This is the same routine as 'UART4_rxinttxint.c' but with the isr call for 
receive chars to 'usartx_rxint_rxisrptradvRTC' which appends the RTC system
counter at the end of the line (i.e. four bytes appended following the EOL and
zero terminator bytes.

This allows associating a GPS NMEA line with the time to the RTC tick count without
a variable delay.
*/

/* Strategy--

In this routine both Rx and Tx interrupt on a char-by-char basis so the
isr handler entry has to deal with both since they share a common 
interrupt vector.  When either Rx or Tx uses the DMA then there is 
no issue with Rx and Tx sharing a common interrupt vector.

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
#include "../libopenstm32/rcc.h"
#include "../libopenstm32/nvic.h"
#include "../libopenstm32/gpio.h"
#include "../libopenstm32/rcc.h"
#include "../libopenstm32/dma.h"
#include "../libmiscstm32/clocksetup.h"

#include "../libusartstm32/usartprotoprivate.h" // Subroutine prototypes for internal use
#include "../libusartstm32/usartbitband.h" 	// Register defines for bit band use
#include "../libusartstm32/nvicdirect.h" 	// Register macros


/* This is the only variable placed in static variable area.  The
control block and the buffers are allocated in the heap area via
mymalloc which is a one-time only allocation, i.e. no 'free' etc. */

//struct USARTCBR* pUSARTcbr3;	// Receive
//struct USARTCBT* pUSARTcbt3;	// Transmit

/*******************************************************************************
 * u16 USART3_rxinttxint_initRTC (u32 BaudRate,u16 RcvLineSize, u16 NumberRcvLines, u16 XmtLineSize, u16 NumberXmtLines);
 * @brief	: Initializes the USARTx to 8N1 and the baudrate for interrupting receive into line buffers
 * @param	: u32 Baudrate		- baudrate 				(e.g. 115200)
 * @param	: u16 RcvLineSize	- size of each line buffer 		(e.g. 32)
 * @param	: u16 NumberRcvLines	- number of receive line buffers	(e.g. 3)
 * @param	: u16 XmtLineSize	- size of each line buffer 		(e.g. 80)
 * @param	: u16 NumberXmtLines	- number of xmt line buffers		(e.g. 2)
 * @return	: 0 = success; 1 = fail malloc; 2 = RcvLineSize zero; 3 = NumberRcvLines = 0; 
 *******************************************************************************/
u16 USART3_rxinttxint_initRTC (u32 BaudRate,u16 RcvLineSize, u16 NumberRcvLines, u16 XmtLineSize, u16 NumberXmtLines)
{
	u16 temp;

	/* Setup GPIO pin for GPIO_USART3 tx (PB10) (See Ref manual, page 158) */
	GPIO_CRH(GPIOB) &= ~((0x000f ) << (4*2));	// Clear CNF reset bit 01 = Floating input (reset state)
	GPIO_CRH(GPIOB) |=  (( (GPIO_CNF_OUTPUT_ALTFN_PUSHPULL<<2) | (GPIO_MODE_OUTPUT_50_MHZ) ) << (4*2));
	
	// Enable clock for USART3.
	RCC_APB1ENR |= RCC_APB1ENR_USART3EN;

	/* Allocate memory for buffers and setup pointers in control blocks for rcv and xmt */
	/* 'rxint' is the same as for 'rxinttxint'  Just the interrupt routine is different */
	if ( (temp = usartx_rxint_allocatebuffers(RcvLineSize,NumberRcvLines,&pUSARTcbr3)) != 0) return temp;
	/* 'txdma' uses the same tx buffer setup as 'txint' */
	if ( (temp = usartx_txdma_allocatebuffers(XmtLineSize,NumberXmtLines,&pUSARTcbt3)) != 0) return temp+2;

	/* Set up usart and baudrate */
	/* 'txcir' sets up the usart Tx the same as needed here */
	usartx_txint_usart_init (USART3,BaudRate);// Tx enable
	/* 'rxint' of course is the same except for the interrup routine */
	usartx_rxint_usart_init (USART3,BaudRate);// Rx enable and enable interrupt

	/* Set and enable interrupt controller USART */
	NVICIPR(NVIC_USART3_IRQ, USART_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_USART3_IRQ);			// Enable interrupt	
	return 0 ;	// Success	
}
/*#######################################################################################
 * ISR routine
 *####################################################################################### */
void USART3_IRQHandler(void)
{
	/* Receive */
	if ( MEM_ADDR(BITBAND(USART3SR,RXFLAG)) != 0)	// Receive register loaded?
	{  // Here, receive interrupt flag is on. 
		*pUSARTcbr3->prx_now_i = USART_DR(USART3);	// Read and store char

		/* Advance pointers to line buffers and array of counts and reset when end reached */	
		usartx_rxint_rxisrptradvRTC(pUSARTcbr3);	// Advance pointers common routine
	}

	/* Transmit */
	if (MEM_ADDR(BITBAND(USART3CR1,TXFLAG)) != 0)	// Are Tx interrupts enabled?
	{  // Here, yes.  Transmit interrupts are enabled so check if a tx interrupt
		if (MEM_ADDR(BITBAND(USART3SR,TXFLAG)) != 0)	// Transmit register empty?
		{ // Here, yes.
			USART_DR(USART3) = *pUSARTcbt3->ptx_now_d++;	// Send next char, step pointer
			*pUSARTcbt3->ptx_ctary_now_d -= 1;	// Count down number to send in the line buffer
			if (*pUSARTcbt3->ptx_ctary_now_d == 0) 
			{ // Here, last char to be sent from this line buffer has been loaded into the tx register
				/* Advance pointers to line buffers and array of counts and reset to beginning when end reached */	
				usartx_txisr_advlnptr(pUSARTcbt3);	// Advance pointer for interrupt pointer ('_d)
				/* Are we caught up?  Check beginning of line buffer pointers, mainline versus interrupt  */	
//				if (pUSARTcbt3->ptx_begin_d == pUSARTcbt3->ptx_begin_m) // Interrupt versus Mainline pointers
				if (*pUSARTcbt3->ptx_ctary_now_d == 0)	// Is new line buffer ready?
				{ // Here, we are caught up with the main (where chars are being added)
					MEM_ADDR(BITBAND(USART3CR1,TXFLAG)) = 0x00;	// Disable Tx interrupts
				}
				// else   pointers have been advanced, so the next interrupt will send a char from
				//         from the new line buffer.
			}
		}
	}

	return;
}

