/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : usartx_txcir_puts.c 
* Hackor             : deh
* Date First Issued  : 10/14/2010 deh
* Description        : Put string: USARTx tx char-by-char interrupts using line buffers
*******************************************************************************/ 
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
u16	tx_ln_sz;	// Constant: Size of circular buffer
u16	tx_ln_ct;	// Constant: --not used--
*/
/*******************************************************************************
* void usartx_txcir_puts(u32* usartx_txflag_bitband, struct USARTCBT* pUSARTcbtx, char* p);
* @brief	: Add a zero terminated string to the output buffer, common routine
* @param	: p 		Pointer to zero terminated string
* @param	: pUSARTcbx	Pointer to struct--pUSARTcb1,2,3
* @return	: none
*******************************************************************************/
void usartx_txcir_puts(char* p, struct USARTCBT* pUSARTcbtx)
{
	
	



	// Set up the particular pointers for the assembly routine since it doesn't have a good
	// way to get the struct offsets.
	// ARGS: Input char pointer, address of 'ptx_now pointer, pointer to beginning of buffer, pointer to end of buffer
	usartx_txcir_putsS(p,(&pUSARTcbtx->ptx_now_m),(pUSARTcbtx->ptx_begin),(pUSARTcbtx->ptx_end) );
	return;
}

