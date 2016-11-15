/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : usartx_txint_send.c 
* Hackor             : deh
* Date First Issued  : 10/16/2010 deh
* Description        : USARTx send for tx char-by-char interrupts using line buffers
*******************************************************************************/ 
#include "../libusartstm32/usartprotoprivate.h" // Subroutine prototypes for internal use
#include "../libusartstm32/usartbitband.h" 	// Register defines for bit band use
/*******************************************************************************
* char usartx_txint_send(struct USARTCBT* pUSARTcbtx);
* @brief	: Step to next line tx line buffer; if USART not sending, start it now.
* @param	: pUSARTcbtx	Pointer to struct--pUSARTcbid1,2,3
** @return	: 0 = OK; 1 = NG
*******************************************************************************/
char usartx_txint_send(struct USARTCBT* pUSARTcbtx)
{

	/* Compute number of chars to send */ 
 	u16 temp = (pUSARTcbtx->ptx_now_m - pUSARTcbtx->ptx_begin_m);

	/* A zero count would cause 65536 chars to sent, i.e. a full wrap around */
	/* So, zero would mean a bogus 'send' call was issued */
	if ( temp == 0 ) return 1; // Show no chars to send
			
	/* Place count in array that holds the counts */
	*pUSARTcbtx->ptx_ctary_now_m = temp;	

	/* Advance to next line buffer for 'main' pointer */
	usartx_txmain_advlnptr (pUSARTcbtx);

	return 0; // Show chars were sent
}

