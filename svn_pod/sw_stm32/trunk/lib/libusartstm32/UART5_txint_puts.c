/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : UART5_txint_puts.c
* Hackor             : deh
* Date First Issued  : 07/14/2011
* Description        : Add a zero terminated string to the current line buffer
*******************************************************************************/
#include "../libusartstm32/usartprotoprivate.h"
#include "../libusartstm32/usartall.h"


/*******************************************************************************
* void UART5_txint_puts(char* p);
* @brief	: p: Pointer to zero terminated string
* @return	: 1 = all line buffers filled; 0 = free line buffer(s)
*******************************************************************************/
void UART5_txint_puts(char* p)
{
// The call to the assembly putsS.S does the following--
//	while (*p != 0) 		// Store until zero terminator is reached
//	{
//
//	 	*pUSARTcbt5->ptx_now++ = *p++;	// Store a char
//		if (pUSARTcbt5->ptx_now >= (pUSARTcbt5->ptx_begin_m + UART5_BUFFER_TX_LINE_SIZE) ) 
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
		/* Be sure we don't try to load into a buffer that is being sent, or to be sent */
		while (*(volatile char*)pUSARTcbt5->ptx_ctary_now_m != 0);	// Loop until a buffer is free

		/* Move the string into the buffer quickly with this assembly program */
		p = usartx_putsS(p,&pUSARTcbt5->ptx_now_m,(pUSARTcbt5->ptx_begin_m + pUSARTcbt5->tx_ln_sz) );

		/* Step to the next buffer, and if buffer end was hit, start Tx if not already sending */
		if ( p != 0)
			UART5_txint_send();

	/* Repeat loading buffer if the end of the buffer was reached when loading the string */	
	} while (p != 0);

	return ;

}
