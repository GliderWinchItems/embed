/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : usartx_rxcir_ready.c
* Hackor             : deh
* Date First Issued  : 07/23/2013
* Description        : Get number of chars in rx circular buffer
*******************************************************************************/
#include "../libusartstm32/usartprotoprivate.h"
#include "../libusartstm32/usartall.h"
/* NOTE:
The struct pointer usage:
char*	prx_now_i;	// Working:  pointer in line buffer currently being filled (interrupt)
char*	prx_begin_m;	// Working:  pointer to beginning of current line buffer (main)
char*	prx_begin_i;	// Constant: pointer to beginning of current line buffer (interrupt)
char*	prx_begin;	// Constant: pointer to beginning of first line buffer
har*	prx_end;	// Constant: pointer to end of circular buffer
u16*	prx_ctary_now_m;// Working:  pointer to count of chars in corresponding line buffer
u16*	prx_ctary_now_i;// Working:  pointer to count of chars in corresponding line buffer
u16*	prx_ctary_begin;// Constant: pointer to beginning of array of char counts
u16	rx_ln_sz;	// Constant: size of one line buffer
u16	rx_ln_ct;	// Constant: number of line buffers
*/

/*******************************************************************************
* u16 usartx_rxcir_ready(struct USARTCBR* USARTcbrx);
* @brief	: Check number of chars in rx circular buffer
* @return	: Number of chars
*******************************************************************************/
char usartx_rxdma_getchar(struct USARTCBR* pUSARTcbrx)
{
	s32	x;
	x =  pUSARTcbt1->ptx_end - pUSARTcbt1->ptx_now_d;
	if (x < 0) x += rx_ln_sz;
	return x;
}

