/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : USART1_rxint_getcount.c 
* Hackor             : deh
* Date First Issued  : 10/12/2010 deh
* Description        : Get char for USART1 receive with no interrupts, using DMA and circular buffer
*******************************************************************************/ 
#include "../libopenstm32/usart.h"
#include "../libopenstm32/dma.h"
#include "../libusartstm32/usartprotoprivate.h"
#include "../libusartstm32/usartall.h"

/* NOTE:
The struct pointer usage:
char*	prx_now_i;	// Working:  pointer in line buffer currently being filled (interrupt)
char*	prx_begin_m;	// Working:  pointer to beginning of current line buffer (main)
char*	prx_begin_i;	// Constant: pointer to beginning of current line buffer (interrupt)
char*	prx_begin;	// Constant: pointer to beginning of first line buffer
har*	prx_end;	// Constant: pointer to end of last line buffer
u16*	prx_ctary_now_m;// Working:  pointer to count of chars in corresponding line buffer
u16*	prx_ctary_now_i;// Working:  pointer to count of chars in corresponding line buffer
u16*	prx_ctary_begin;// Constant: pointer to beginning of array of char counts
u16	rx_ln_sz;	// Constant: size of one line buffer
u16	rx_ln_ct;	// Constant: number of line buffers
*/

/* NOTE:
The order of getting the count and pointer is important.  A call to 'getline' advances
the pointer and returns with the pointers in the struct advanced to the next line buffer, 
but subroutine return is the pointer before the advance is done.  Therefore, the count
must be retrieved before the pointer.
*/
/*******************************************************************************
* u16 USART1_rxint_getcount(void);
* @brief	: Get the number of chars in the input/read buffer.
* @return	: number of chars in currently buffered.
*******************************************************************************/
u16 USART1_rxint_getcount(void)
{
	return (*pUSARTcbr1->prx_ctary_now_m);  // Return char count
}

