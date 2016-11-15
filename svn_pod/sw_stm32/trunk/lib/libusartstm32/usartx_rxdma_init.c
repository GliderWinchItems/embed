/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : usartx_rxdma_init.c 
* Hackor             : deh
* Date First Issued  : 10/09/2010 deh
* Description        : USARTx rx using dma common routine
*******************************************************************************/ 
#include "../libopenstm32/usart.h"

#include "../libusartstm32/usartprotoprivate.h"

#include "../libusartstm32/mymalloc.h"

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

/******************************************************************************
 * u16 usartx_rxdma_allocatebuffers (u16 rcvcircularsize, u16 getlinesize, struct USARTCBR** pUSARTcbrx);
 * @brief	: Allocate buffers with 'mymalloc' for transmit and setup control block pointers
 * @param	: getlinesize is size of each line buffer (e.g. 48)
 * @param	: numberrcvlines is the number of receive line buffers (e.g. 4)
 * @param	: pUSARTcbrx points to pointer in static memory that points to control block
 * @return	: 0 = success;1 = fail mymallocl; 2 = zero getlinesize zero; 3 = numbergetlinesize zero
******************************************************************************/
u16 usartx_rxdma_allocatebuffers (u16 rcvcircularsize, u16 getlinesize, struct USARTCBR** pUSARTcbrx)
{
	u32* ptr;
	struct USARTCBR* pUSARTcbrl; // Local pointer to control block setup on heap

	/* Zero buffer size would cause trouble */
	if (getlinesize     == 0 ) return 2;
	if (rcvcircularsize == 0 ) return 3;

	/* Allocate space and set pointer for the struct USARTcbtx */
	ptr = (u32*)mymalloc(sizeof (struct USARTCBR));
	if  ( ptr  == 0)  return 1;
	*pUSARTcbrx = (struct USARTCBR*)ptr;	// Initialize static variable used by others
	 pUSARTcbrl = (struct USARTCBR*)ptr;	

	/* ---------------------------- receive (dma) buffers and pointers -------------------------- */
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
	/* Allocate space and set pointer in USARTcbrx for receive circular buffer */
	if ( (pUSARTcbrl->prx_begin = (char*)mymalloc(rcvcircularsize)) == 0 ) return 1;

	/* These tx pointers start out on the same line buffer */	
	pUSARTcbrl->prx_now_i = pUSARTcbrl->prx_begin;	// main's working pointer in circular rx buffer

	/* Use this for testing for end of line buffers wraparound */
	pUSARTcbrl->prx_end     = pUSARTcbrl->prx_begin + rcvcircularsize;	// pointer to end 

	/* Save rx circular buffer size */
	pUSARTcbrl->rx_ln_sz = rcvcircularsize;

	/* Save rx line buffer size */
	pUSARTcbrl->rx_ln_ct = getlinesize;

	/* Setup getline line buffer */
	if ( (pUSARTcbrl->prx_begin_i = (char*)(mymalloc(getlinesize))) == 0 ) return 1;
	pUSARTcbrl->prx_begin_m = pUSARTcbrl->prx_begin_i;	// Set working pointer: main
	pUSARTcbrl->prx_ctary_now_i = (u16*)(pUSARTcbrl->prx_begin_i + getlinesize);	// Set constant: pointer end

	return 0;
}
/******************************************************************************
 * void usartx_rxdma_usart_init (u32 USARTx, u32 BaudRate);
 * @brief	: Setup USART baudrate, rx for dma
 * @return	: none
******************************************************************************/
void usartx_rxdma_usart_init (u32 USARTx, u32 BaudRate)
{
	/* Set baud rate */
	usartx_setbaud(USARTx,BaudRate);	// Compute divider settings and load BRR

 	/* Setup CR2 ------------------------------------------------------------------- */
	/* After reset CR2 is 0x0000 and this is just fine */

	/* Set up CR1 (page 771) ------------------------------------------------------- */
	USART_CR1(USARTx) |= USART_UE | USART_RX_ENABLE;// Set Usart enable, receive enable
	
	/* Hook up usart rx to dma channel */
	USART_CR3(USARTx) |= USART_DMAR;			// CR3 setup

	return;	
}

