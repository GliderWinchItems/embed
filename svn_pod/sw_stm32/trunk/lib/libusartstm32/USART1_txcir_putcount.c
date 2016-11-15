/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : USART1_txcir_putcount.c 
* Hackor             : deh
* Date First Issued  : 10/14/2010 deh
* Description        : Get free tx buffer count: USART1 tx char-by-char interrupt with circular buffer
*******************************************************************************/ 
#include "../libusartstm32/usartprotoprivate.h" // Subroutine prototypes for internal use
#include "../libusartstm32/usartbitband.h" 	// Register defines for bit band use
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
* u16 USART1_txcir_putcount(void);
* @brief	: Get number of chars available in circular buffer
* @param	: Pointer to zero terminated string
* @return	: 0 = no chars available, + = count of chars free
*******************************************************************************/
u16 USART1_txcir_putcount(void)
{	
	return usartx_txcir_putcount((u32*)BITBAND(USART1SR,TXFLAG),pUSARTcbt1);
}


