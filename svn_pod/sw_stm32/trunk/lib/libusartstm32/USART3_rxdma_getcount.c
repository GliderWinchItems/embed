/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : USART3_rxdma_getcount.c 
* Hackor             : deh
* Date First Issued  : 10/11/2010 deh
* Description        : Get char for USART3 receive with no interrupts, using DMA and circular buffer
*******************************************************************************/ 
#include "../libopenstm32/usart.h"
#include "../libopenstm32/dma.h"
#include "../libusartstm32/usartprotoprivate.h"
#include "../libusartstm32/usartall.h"

/* NOTE:
The struct pointer usage:
char*	ptx_now_i;	// Working:  pointer in circular rx buffer
char*	prx_begin_m;	// Working:  pointer to getline buffer
char*	prx_begin_i;	// Working:  pointer to getline buffer begin
char*	prx_begin;	// Constant: pointer to beginning of circular line buffer
char*	prx_end;	// Constant: pointer to end of rx circular buffer
u16*	prx_ctary_now_m;// Working:  --not used--
(char*)	prx_ctary_now_i;// Working:  pointer to getline buffer end 
u16*	prx_ctary_begin;// Constant: --not used--
u16	rx_ln_sz;	// Constant: size of circular line buffer 
u16	rx_ln_ct;	// Constant: size of getline buffer
*/

/*******************************************************************************
* u16 USART3_rxdma_getcount(void);
* @brief	: Get the number of chars in the input/read buffer.
* @return	: number of chars in currently buffered.
*******************************************************************************/
u16 USART3_rxdma_getcount(void)
{
	/* Difference between where we are taking out chars, and where DMA is storing */
	s16 Diff = pUSARTcbr3->prx_end - (u32)DMA1_CNDTR3 - pUSARTcbr3->prx_now_i;
	if (Diff >= 0) 				// Is there a wrap-around?
		return Diff;			// Here, no wrap-around, or zero chars
	// Here there is a wraparound
	return (Diff + pUSARTcbr3->rx_ln_sz);  // Adjust for wrap
}
