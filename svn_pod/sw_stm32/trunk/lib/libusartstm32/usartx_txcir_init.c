/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : usartx_txcir.c 
* Hackor             : deh
* Date First Issued  : 10/13/2010 deh
* Description        : USARTx tx char-by-char interrupt, circular buffer
*******************************************************************************/ 
#include "../libopenstm32/usart.h"
#include "../libopenstm32/memorymap.h"


#include "../libusartstm32/mymalloc.h"
#include "../libusartstm32/usartprotoprivate.h" // Subroutine prototypes for internal use

/* NOTE -- tx control block usage
char*	ptx_now_m;	// Working:  pointer in circular buffer, main
char*	ptx_begin_m;	// Working:  --not used--
char*	ptx_now_d;	// Working:  pointer in circular buffer, interrupt
char*	ptx_begin_d;	// Working:  --not used--
char*	ptx_begin;	// Constant: pointer to beginning of tx circular buffer
char*	ptx_end;	// Constant: pointer to end of tx circular buffer
u16*	ptx_ctary_now_m;// Working:  --not used--
u16*	ptx_ctary_now_d;// Working:  --not used--
u16*	ptx_ctary_begin;// Constant: --not used--
u16	tx_ln_sz;	// Constant: Size of circular buffer
u16	tx_ln_ct;	// Constant: --not used--
*/
/******************************************************************************
 *u16 usartx_txcir_allocatebuffers (u16 xmtcircularsize, struct USARTCBT** pUSARTcbtx);
 * @brief	: Allocate buffers with 'mymalloc' for receive and setup control block pointers
 * @param	: xmtcircularsize is size of circular buffer (e.g. 128)
 * @param	: pUSARTcbtx points to pointer in static memory that points to control block
 * @return	: 0 = success; 1 = fail malloc; 2 = rcvcircularsize zero; 
******************************************************************************/
u16 usartx_txcir_allocatebuffers (u16 xmtcircularsize, struct USARTCBT** pUSARTcbtx)
{
	u32* ptr;
	struct USARTCBT* pUSARTcbtl; // Local pointer to control block setup on heap

	/* Bomb out just in case buffer size would be zero */
	if (xmtcircularsize    == 0) return 2;

	/* Allocate space and set pointer for the struct USARTcbtx */
	ptr = (u32*)mymalloc(sizeof (struct USARTCBT));
	if  ( ptr  == 0)  return 1;
	*pUSARTcbtx = (struct USARTCBT*)ptr;	// Initialize static variable used by others
	 pUSARTcbtl = (struct USARTCBT*)ptr;	

	/* ---------------------------- transmit (dma) buffers and pointers -------------------------- */

	/* Allocate space and set pointer in USARTcbx for transmit line buffer(s) */
	if ( (pUSARTcbtl->ptx_begin = mymalloc(xmtcircularsize)) == 0 ) return 1;

	/* These tx pointers start out on the same line buffer */	
	pUSARTcbtl->ptx_now_m   = pUSARTcbtl->ptx_begin;	// Pointer in circular buffer for (main)
	pUSARTcbtl->ptx_now_d   = pUSARTcbtl->ptx_begin;	// Pointer in circular buffer for (interrupt)

	/* Use this for testing for end of line buffers wraparound */
	pUSARTcbtl->ptx_end     = pUSARTcbtl->ptx_begin + xmtcircularsize; 

	/* Save size */
	pUSARTcbtl->tx_ln_sz = xmtcircularsize;

	return 0;
}
/******************************************************************************
 * void usartx_txcir_usart_init (u32 USARTx, u32 BaudRate);
 * @brief	: Setup USART baudrate, Tx for dma
******************************************************************************/
void usartx_txcir_usart_init (u32 USARTx, u32 BaudRate)
{
	/* Set baud rate */
	usartx_setbaud (USARTx,BaudRate);	// Compute divider settings and load BRR

	 	/* Setup CR2 ------------------------------------------------------------------- */
		/* After reset CR2 is 0x0000 and this is just fine */
		/* Set up CR1 (page 771) ------------------------------------------------------- */
		USART_CR1(USARTx) |= USART_UE | USART_TX_ENABLE;// Set UE, TE
		// Note Tx interrupt enable is set when there is data ready to be sent		

	return;	
}

