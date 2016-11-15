/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : USART3_txdma_puts.c
* Hackor             : deh
* Date First Issued  : 10/06/2010
* Description        : Add a zero terminated string to the current line buffer
*******************************************************************************/
#include "../libusartstm32/usartprotoprivate.h"
#include "../libusartstm32/usartall.h"

/*******************************************************************************
* void USART3_txdma_puts(char* p);
* @brief	: Add a zero terminated string to the output buffer.
* @param	: Pointer to zero terminated string
* @return	: none
*******************************************************************************/
void USART3_txdma_puts(char* p)
{
// The call to the assembly putsS.S does the following--
//	while (*p != 0) 		// Store until zero terminator is reached
//	{
//
//	 	*pUSARTcbt3->ptx_now++ = *p++;	// Store a char
//		if (pUSARTcbt3->ptx_now >= (pUSARTcbt3->ptx_begin_m + USART3_BUFFER_TX_LINE_SIZE) ) 
//		{
//			return p; 	// Here we hit the end of the buffer!
//		}
//	}
//	return 0;	
// Supply putsS with: pointer to string, current buffer pointer (now), pointer to end of buffer
// If the putsS hits the end of the line buffer, then it returns a non-zero pointer that points
// that points the char not loaded into the line buffer.  When this happens, 'send' the full line 
// buffer and step to the next line buffer.  If that buffer is the one currently being sent by 
// the DMA then we return the pointer to the mainline and the bozo writing that program can 
// wrestle with the problem of what to do.
	do 
	{
		while (USART3_txdma_busy() == 1);
		p = usartx_putsS(p,&pUSARTcbt3->ptx_now_m,(pUSARTcbt3->ptx_begin_m + pUSARTcbt3->tx_ln_sz) );
		USART3_txdma_send();
	} while (p != 0);

	return ;

}
