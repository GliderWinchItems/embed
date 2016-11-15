/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : usartx_rxinttxcir_irqhandler.c
* Hackor             : deh
* Date First Issued  : 10/13/2010 deh
* Description        : Tx char-by-char interrupt, circular buffer irq handling
*******************************************************************************/ 
#include "../libopenstm32/usart.h"
#include "../libopenstm32/memorymap.h"


#include "../mymalloc.h"
#include "../libusartstm32/usartprotoprivate.h" // Subroutine prototypes for internal use

/* NOTE -- control block usage
char*	ptx_now_m;	// Working:  pointer in circular buffer, main
char*	ptx_begin_m;	// Working:  --not used--
char*	ptx_now_d;	// Working:  pointer in circular buffer, interrupt
char*	ptx_begin_d;	// Working:  --not used--
char*	ptx_begin;	// Constant: pointer to beginning of tx circular buffer
char*	ptx_end;	// Constant: pointer to end of tx circular buffer
u16*	ptx_ctary_now_m;// Working:  --not used--
u16*	ptx_ctary_now_d;// Working:  --not used--
u16*	ptx_ctary_begin;// Constant: --not used--
u16	tx_ln_sz;	// Constant: --not used--
u16	tx_ln_ct;	// Constant: --not used--
*/
/******************************************************************************
 * void usartx_rxinttxcir_irqhandler(u32 USARTx, struct USARTCBR* pUSARTcbrx, struct USARTCBT* pUSARTcbtx)
 * @brief	: Do USART rxint and txcir interrupt handling
 * @param	: pUSARTcbtx	Pointer to struct--pUSARTcbt1,2,3
 * @return	: none
****************************************************************************/
void usartx_rxinttxcir_irqhandler(u32 USARTx, struct USARTCBR* pUSARTcbrx, struct USARTCBT* pUSARTcbtx);
{
	/* Receive */
	if ( *(u16*)(USARTx + SR) & USART_FLAG_RXNE)	// Receive register loaded?
	{  // Here, receive interrupt. 
		*pUSARTcbrx->prx_now_i = *(u16*)(USARTx + DR);	// Read and store char

		/* Advance pointers to line buffers and array of counts and reset when end reached */	
		usartx_rxint_rxisrptradv(pUSARTcbrx);	// Advance pointers common routine
	}

	/* Transmit */
	if (*(u16*)(USARTx + SR) & USART_FLAG_TXE)	// Transmit register empty?
	{  // Here, transmit interrupt. 
		*(u16*)(USARTx + DR) = *pUSARTcbtx->prx_now_d++;	// Send next char, step pointer

		/* At end of buffer? */
		if (pUSARTcbtx->prx_now_d >= pUSARTcbtx->prx_end)
		{ // Here yes.  Reset to beginning
			pUSARTcbtx->prx_now_d = pUSARTcbtx->prx_begin;
		}

		/* Are we caught up  */	
		if (pUSARTcbtx->ptx_now_d == pUSARTcbtx->ptx_now_m)
		{ // We are caught up with the main that has been adding chars to buffers
			*(u16*)(USARTx + CR1) &= ~USART_TX_INTERRUPT_ENABLE;// Disable Tx interrupts
		}
	}

}

