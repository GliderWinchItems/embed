/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : USART1_rxcirtxcir.c
* Hackor             : deh
* Date First Issued  : 10/15/2010 deh
* Description        : USART1 char-by-char interrupting; rx line buffers, tx circular buffer
*******************************************************************************/

/*
10/13/2010 deh
07/23/2013 deh hacked USART1_rxinttxcir.c to cir & cir
*/

/* Strategy--

In this routine both Rx and Tx interrupt on a char-by-char basis so the
isr handler entry has to deal with both since they share a common 
interrupt vector.  When either Rx or Tx uses the DMA then there is 
no issue with Rx and Tx sharing a common interrupt vector.

For receive a line buffer is filled with incoming chars until an END_OF_LINE char
is received.  When EOL is received the char count is stored in an array
and the next line buffer is selected.

The mainline program checks for data with 'getcount', 'getline', or 'getlineboth'.  
If the count is not zero then the program gets a pointer to the line buffer (see
the Utest2 program for the different ways of handling this).

For transmit chars are added to a circular buffer, and enables the transmit interrupt.
Characters are sent until the transmit pointer catches up to the mainline pointer.
There is no advantage to sending out of line buffers with this scheme.

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
//struct USARTCBR* pUSARTcbr1;	// Receive
//struct USARTCBT* pUSARTcbt1;	// Transmit
/* NOTE -- Tx control block usage
The struct pointer usage:
char*	prx_now_i;	// Working:  pointer in line buffer currently being filled (interrupt)
char*	prx_begin_m;	// Working:  pointer to beginning of current line buffer (main)
char*	prx_begin_i;	// Constant: pointer to beginning of current line buffer (interrupt)
char*	prx_begin;	// Constant: pointer to beginning of first line buffer
har*	prx_end;	// Constant: pointer to end of circular buffer
u16*	prx_ctary_now_m;// Working:  pointer to count of chars in corresponding line buffer
u16*	prx_ctary_now_i;// Working:  pointer to count of chars in corresponding line buffer
u16*	prx_ctary_begin;// Constant: pointer to beginning of array of char counts
u16	rx_ln_sz;	// Constant: size of one line buffer
u16	rx_ln_ct;	// Constant: number of line buffers
*/
/*******************************************************************************
 * u16 USART1_rxinttxcir_init (u32 BaudRate,u16 RcvCircularSize, u16 XmtCircularSize);
 * @brief	: Initializes the USARTx to 8N1 and the baudrate for interrupting receive into line buffers
 * @param	: u32 Baudrate		- baudrate 				(e.g. 115200)
 * @param	: u16 RcvLineSize	- size of each line buffer 		(e.g. 32)
 * @param	: u16 XmtCircularSize	- size of xmt circular buffer 		(e.g. 128)
 * @return	: 0 = success; 1 = fail malloc; 2 = RcvLineSize zero;  4 = XmitCircularSize = 0 
 *******************************************************************************/
u16 USART1_rxcirtxcir_init (u32 BaudRate,u16 RcvCircularSize, u16 NumberRcvLines, u16 XmtCircularSize)
{
	u16 temp;
	/* Allocate memory for buffers and setup pointers in control blocks for rcv and xmt */
	if ( (temp = usartx_rxint_allocatebuffers(RcvCircularSize,         1,&pUSARTcbr1)) != 0) return temp+2;
	if ( (temp = usartx_txcir_allocatebuffers(XmtCircularSize,           &pUSARTcbt1)) != 0) return temp+3;

	/* Setup GPIO pin for GPIO_USART1 tx (PA9) (See Ref manual, page 158) */
	GPIO_CRH(GPIOA) &= ~((0x000f ) << (4*1));	// Clear CNF reset bit 01 = Floating input (reset state)
	GPIO_CRH(GPIOA) |=  (( (GPIO_CNF_OUTPUT_ALTFN_PUSHPULL<<2) | (GPIO_MODE_OUTPUT_50_MHZ) ) << (4*1));
	
	// Enable clock for USART1.
	RCC_APB2ENR |= RCC_APB2ENR_USART1EN;

	/* Set up usart and baudrate */
	usartx_txcir_usart_init (USART1,BaudRate);	// Tx enable
	usartx_rxint_usart_init (USART1,BaudRate);	// Rx enable and enable interrupt
	
	/* Set and enable interrupt controller USART */
	NVICIPR(NVIC_USART1_IRQ, USART_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_USART1_IRQ);			// Enable interrupt

	return 0 ;	// Success	
}
/*#######################################################################################
 * ISR routine
 *####################################################################################### */
void USART1_IRQHandler(void)
{
	/* Receive */
	if ( MEM_ADDR(BITBAND(USART1SR,RXFLAG)) != 0)	// Receive register loaded?
	{  // Here, receive interrupt flag is on. 
		*pUSARTcbr1->prx_now_i = USART_DR(USART1);	// Read and store char

		/* Advance pointers to line buffers and array of counts and reset when end reached */	
		usartx_rxint_rxisrptradv_cir(pUSARTcbr1);	// Advance pointers common routine

		pUSARTcbr1->prx_now_i += 1;	
		if (pUSARTcbr1->prx_now_i >= pUSARTcbrl->prx_end)
		{ // Here, end of circular buffer, wrap around to beginning
			pUSARTcbr1->prx_now_i = pUSARTcbr1->prx_begin_i;
		}
	}

	/* Transmit */
	if (MEM_ADDR(BITBAND(USART1CR1,TXFLAG)) != 0)	// Are Tx interrupts enabled?
	{  // Here, yes.  Transmit interrupts are enabled so check if a tx interrupt
		if (MEM_ADDR(BITBAND(USART1SR,TXFLAG)) != 0)	// Transmit register empty?
		{ // Here, yes.
			USART_DR(USART1) = *pUSARTcbt1->ptx_now_d++;	// Send next char, step pointer
			/* At end of buffer? */
			if (pUSARTcbt1->ptx_now_d >= pUSARTcbt1->ptx_end)
			{ // Here yes.  Reset pointer to beginning of buffer
				pUSARTcbt1->ptx_now_d = pUSARTcbt1->ptx_begin;
			}
			/* Are we caught up, i.e. did this last pointer increment match where chars are to be added?  */	
			if (pUSARTcbt1->ptx_now_d == pUSARTcbt1->ptx_now_m) 	// Interrupt versus Mainline pointers
			{ // Here, we are caught up with the main that has been adding chars to buffers
				MEM_ADDR(BITBAND(USART1CR1,TXFLAG)) = 0x00;	// Disable Tx interrupts
			}
		}
	}

	return;
}

