/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : USART1_rxint_getlineboth.c 
* Hackor             : deh
* Date First Issued  : 10/12/2010 deh
* Description        : USARTx rx char-by-char interrupting, get a line
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
/*******************************************************************************
* struct USARTLB USART1_rxint_getlineboth(void);
* @brief	: Get pointer to one 0x0d (END_OF_LINE) zero terminated line buffer
* @return	: 0 == line not complete, != 0 is ptr to completed line buffer & char count
*******************************************************************************/
struct USARTLB USART1_rxint_getlineboth(void)
{
/* NOTE:
The order of getting the count and pointer is important.  The call to advance the pointer
returns with the pointers advanced to the next line buffer, but returns the pointer
before the advance is done.  Therefore, the counter must be retrieved before the
pointer.
*/
	struct USARTLB lb = {0,0};	// zero count and null pointer

	if (pUSARTcbr1->prx_begin_m == pUSARTcbr1->prx_begin_i)
	{	
		return lb;			// Return zero, meaning nothing to do.
	}
	lb.ct = *pUSARTcbr1->prx_ctary_now_m;	// Return count in current buffer
	lb.p = usartx_rxint_rxmainadvptr (pUSARTcbr1); // Return pointer and step to next buffer
	return lb; 
}

