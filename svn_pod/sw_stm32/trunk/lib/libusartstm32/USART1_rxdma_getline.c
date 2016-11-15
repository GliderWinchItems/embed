/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : usarrx_rxdma_getline.c 
* Hackor             : deh
* Date First Issued  : 10/11/2010 deh
* Description        : USARTx rx dma, get a line
*******************************************************************************/ 
#include "../libopenstm32/usart.h"
#include "../libopenstm32/dma.h"

#include "../libusartstm32/usartprotoprivate.h"
#include "../libusartstm32/usartall.h"

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
/*******************************************************************************
* char* USART1_rxdma_getline(void);
* @brief	: Assemble one 0x0d (END_OF_LINE) zero terminated line from the input/read buffer.
* @return	: 0 == line not complete, != 0 is ptr to completed line buffer
*******************************************************************************/
char* USART1_rxdma_getline(void)
{
	char c;					// Temp char
	u16 Count = USART1_rxdma_getcount();	// Count of chars ready in buffer
	if ( Count == 0 ) 			// Any chars in buffer?
		return 0;			// Return zero, meaning nothing to do.
	while (Count-- > 0) 			// Loop until buffer exhausted (or end of line)
	{ // Here, there are 1 or more chars in buffer
		c = USART1_rxdma_getchar();	// Save the char
		*pUSARTcbr1->prx_begin_m++ = c;	// Store the char in the line buffer
		if (pUSARTcbr1->prx_begin_m >= (char*)pUSARTcbr1->prx_ctary_now_i) // At end of buffer?
			pUSARTcbr1->prx_begin_m--;	// Prevent line buffer overflow
		if (c == END_OF_LINE)		// Test the char: Are we at an end of line?
		{ // Here, yes
			*pUSARTcbr1->prx_begin_m = 0; 	// Zero terminate the line
		         pUSARTcbr1->prx_begin_m = pUSARTcbr1->prx_begin_i; // Reset pointer to beginning
			return pUSARTcbr1->prx_begin_m;	// Return pointer to beginning of line (non-zero return)
		}
	}
	return 0;	// Return, still not end of line
}
