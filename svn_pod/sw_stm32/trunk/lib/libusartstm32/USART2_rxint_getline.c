/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : USART2_rxint_getline.c 
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
* char* USART2_rxint_getline(void);
* @brief	: Get pointer to one 0x0d (END_OF_LINE) zero terminated line buffer
* @return	: 0 == line not complete, != 0 is ptr to completed line buffer
*******************************************************************************/
char* USART2_rxint_getline(void)
{
	return usartx_rxint_getline(pUSARTcbr2);
}

