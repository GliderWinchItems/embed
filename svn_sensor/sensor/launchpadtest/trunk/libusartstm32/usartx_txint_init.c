/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : usartx_txint.c 
* Hackor             : deh
* Date First Issued  : 10/15/2010 deh
* Description        : USARTx tx using dma common routine
*******************************************************************************/ 
#include "../libopenstm32/usart.h"
#include "../libusartstm32/usartprotoprivate.h"
//#include "../libusartstm32/mymalloc.h"
#include <stdlib.h>
/******************************************************************************
 *u16 usartx_txint_allocatebuffers (u16 xmtlinesize,u16 numberxmtlines, struct USARTCBT** pUSARTcbtx);
 * @brief	: Allocate buffers with 'mymalloc' for receive and setup control block pointers
 * @param	: xmtlinesize is size of each line buffer (e.g. 48)
 * @param	: numberxmtlines is the number of receive line buffers (e.g. 4)
 * @param	: pUSARTcbtx points to pointer in static memory that points to control block
 * @return	: 0 = success; 1 = fail malloc; 2 = rcvlinesize zero; 3 = numberrcvlines = 0; 
******************************************************************************/
u16 usartx_txint_allocatebuffers (u16 xmtlinesize,u16 numberxmtlines, struct USARTCBT** pUSARTcbtx)
{
	u32* ptr;
	struct USARTCBT* pUSARTcbtl; // Local pointer to control block setup on heap

	/* Bomb out just in case buffer size would be zero */
	if (xmtlinesize    == 0) return 2;
	if (numberxmtlines == 0) return 3;

	/* Allocate space and set pointer for the struct USARTcbtx */
	ptr = (u32*)malloc(sizeof (struct USARTCBT));
	if  ( ptr  == 0)  return 1;
	*pUSARTcbtx = (struct USARTCBT*)ptr;	// Initialize static variable used by others
	 pUSARTcbtl = (struct USARTCBT*)ptr;	

	/* ---------------------------- transmit (dma) buffers and pointers -------------------------- */

	/* Allocate space and set pointer in USARTcbx for transmit line buffer(s) */
	if ( (pUSARTcbtl->ptx_begin = (char*)malloc(xmtlinesize * numberxmtlines)) == 0 ) return 1;

	/* These tx pointers start out on the same line buffer */	
	pUSARTcbtl->ptx_begin_m = pUSARTcbtl->ptx_begin;	// Beginning of line being filled (main)
	pUSARTcbtl->ptx_begin_d = pUSARTcbtl->ptx_begin;	// Beginning of line being sent (int)
	pUSARTcbtl->ptx_now_m   = pUSARTcbtl->ptx_begin;	// Pointer in line being filled (main)
	pUSARTcbtl->ptx_now_d   = pUSARTcbtl->ptx_begin;	// Pointer in line being filled (int)

	/* Use this for testing for end of line buffers wraparound */
	pUSARTcbtl->ptx_end     = pUSARTcbtl->ptx_begin + (xmtlinesize * numberxmtlines); 

	/* Save tx line buffer size */
	pUSARTcbtl->tx_ln_sz = xmtlinesize;

	/* Save number of tx line buffers */
	pUSARTcbtl->tx_ln_ct = numberxmtlines;

	/* Setup an array with the line buffer transfer count for each tx line buffer */
	if ( (pUSARTcbtl->ptx_ctary_begin = malloc(sizeof(u16) * numberxmtlines)) == 0 ) return 1;
	pUSARTcbtl->ptx_ctary_now_m = pUSARTcbtl->ptx_ctary_begin;	// Set working pointer, (main)
	pUSARTcbtl->ptx_ctary_now_d = pUSARTcbtl->ptx_ctary_begin;	// Set working pointer, (int)

	return 0;
}
/******************************************************************************
 * void usartx_txint_usart_init (u32 USARTx, u32 BaudRate);
 * @brief	: Setup USART baudrate, Tx for dma
******************************************************************************/
void usartx_txint_usart_init (u32 USARTx, u32 BaudRate)
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

