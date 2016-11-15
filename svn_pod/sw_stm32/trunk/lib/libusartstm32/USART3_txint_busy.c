/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : USART3_txint_busy.c 
* Hackor             : deh
* Date First Issued  : 10/16/2010 deh
* Description        : Check that line buffer is available
*******************************************************************************/ 
#include "../libusartstm32/usartprotoprivate.h" // Subroutine prototypes for internal use
#include "../libusartstm32/usartbitband.h" 	// Register defines for bit band use
/*******************************************************************************
* u16 USART3_txint_busy(void);
* @brief	: Check that line buffer is available
* @return	: 1 = all line buffers filled; 0 = free line buffer(s)
*******************************************************************************/
u16 USART3_txint_busy(void)
{	
	/* 
	   IF tx interrupts are enabled (i.e. transmitting) AND main pointer as wrapped around and matched the
	   interrupt pointer, then main has filled all the line buffers, so none are free.  If the pointers are
	   the same, but we are not transmitting, then the all the line buffers are free, e.g. the initial
           conditions. 
	*/
//	if (  (MEM_ADDR(BITBAND(USART3CR1,TXFLAG)) == 1) && (pUSARTcbt3->ptx_begin_d == pUSARTcbt3->ptx_begin_m) )
	if (*pUSARTcbt3->ptx_ctary_now_m != 0)
		return 1;	// Show that no line buffers are free
	return 0;		// Show we are good to go.
}

