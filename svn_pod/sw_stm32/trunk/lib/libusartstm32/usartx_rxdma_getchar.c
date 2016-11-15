/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : ussartx_rxdma_getchar.c
* Hackor             : deh
* Date First Issued  : 10/10/2010
* Description        : Get char from the circular buffer
*******************************************************************************/
#include "../libusartstm32/usartprotoprivate.h"
#include "../libusartstm32/usartall.h"
/* NOTE:
The struct pointer usage:
char*	ptx_now_i;	// Working:  --not used--
char*	prx_begin_m;	// Working:  pointer in circular rx buffer
char*	prx_begin_i;	// Working:  --not used--
char*	prx_begin;	// Constant: pointer to beginning of circular line buffer
char*	prx_end;	// Constant: pointer to end of rx circular buffer
(char*)	prx_ctary_now_m;// Working:  pointer to getline buffer
(char*)	prx_ctary_now_i;// Constant: pointer to getline buffer end 
(char*)	prx_ctary_begin;// Constant: pointer to getline buffer begin
u16	rx_ln_sz;	// Constant: size of circular line buffer 
u16	rx_ln_ct;	// Constant: size of getline buffer
*/

/*******************************************************************************
* char usartx_rxdma_getchar(struct USARTCBR* USARTcbrx);
* @brief	: Get one char from the input (rx) circular buffer.
* @return	: a nice round char
*******************************************************************************/
char usartx_rxdma_getchar(struct USARTCBR* pUSARTcbrx)
{
	char	c;
	/* If the bozo mainline program didn't check for chars we have to loop! */
	while (usart_rxdma_getcount(pUSARTcbrx) == 0);	// Loop until something comes available
	/* Now we can get a char */
	c = *pUSARTcbrx->prx_begin_m++;			// Store char, advance pointer	
	if (pUSARTcbrx->prx_begin_m >= pUSARTcbrx->prx_end)	// Wrap around?
		pUSARTcbrx->prx_begin_m = pUSARTcbrx->prx_begin;// Reset pointer to beginning
	return c;
}

