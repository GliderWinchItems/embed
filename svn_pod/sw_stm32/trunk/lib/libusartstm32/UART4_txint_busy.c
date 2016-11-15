/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : UART4_txint_busy.c 
* Hackor             : deh
* Date First Issued  : 07/14/2011
* Description        : Check that line buffer is available
*******************************************************************************/ 
#include "../libusartstm32/usartprotoprivate.h" // Subroutine prototypes for internal use
#include "../libusartstm32/usartbitband.h" 	// Register defines for bit band use
/*******************************************************************************
* u16 UART4_txint_busy(void);
* @brief	: Check that line buffer is available
* @return	: 1 = all line buffers filled; 0 = free line buffer(s)
*******************************************************************************/
u16 UART4_txint_busy(void)
{	
	/* 
	   IF tx interrupts are enabled (i.e. transmitting) AND main pointer as wrapped around and matched the
	   interrupt pointer, then main has filled all the line buffers, so none are free.  If the pointers are
	   the same, but we are not transmitting, then the all the line buffers are free, e.g. the initial
           conditions. 
	*/
//	if (  (MEM_ADDR(BITBAND(UART4CR1,TXFLAG)) == 1) && (pUSARTcbt4->ptx_begin_d == pUSARTcbt4->ptx_begin_m) )
	if (*pUSARTcbt4->ptx_ctary_now_m != 0)
		return 1;	// Show that no line buffers are free
	return 0;		// Show we are good to go.
}

