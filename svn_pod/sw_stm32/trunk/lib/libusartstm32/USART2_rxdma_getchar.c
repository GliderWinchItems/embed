/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : USART2_rxdma_getchar.c
* Hackor             : deh
* Date First Issued  : 10/10/2010
* Description        : Get char from the circular buffer
*******************************************************************************/
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
* char USART2_rxdma_getchar(void);
* @brief	: Get one char from the input (rx) circular buffer.
* @return	: a nice round char
*******************************************************************************/
char USART2_rxdma_getchar(void)
{
	char	c;
	/* If the bozo mainline program didn't check for chars we have to loop! */
//	while (USART2_rxdma_getcount() == 0);	// Loop until something comes available
	/* Now we can get a char */
	c = *pUSARTcbr2->prx_now_i++;		// Store char, advance pointer	
	if (pUSARTcbr2->prx_now_i >= pUSARTcbr2->prx_end)	// Wrap around?
		pUSARTcbr2->prx_now_i = pUSARTcbr2->prx_begin;	// Reset pointer to beginning
	return c;
}

