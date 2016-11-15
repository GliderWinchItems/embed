/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : USART1_PC_gateway.c
* Author             : deh
* Date First Issued  : 07/23/2013
* Board              : STM32 board
* Description        : PC<->gateway using USART1
*******************************************************************************/
/*
10-04-2013 - revised so that 'PC_gateway_comm.[ch]' has routines common to PC and stm32,
   and this routine is USART1 specific.
*/
#include "USART1_PC_gateway.h"
#include "PC_gateway_comm.h"
#include "usartallproto.h"


/* **************************************************************************************
 * int USART1_PC_msg_get(struct PCTOGATEWAY* ptr);
 * @brief	: Build message from PC
 * @param	: ptr = Pointer to msg buffer (see common_can.h)
 * @return	:  1 = completed; ptr->ct hold byte count
 *              :  0 = msg not ready; 
 *              : -1 = completed, but bad checksum
 *  		: -2 = completed, but too few bytes to be a valid CAN msg
 * ************************************************************************************** */
/* Note: It is up to the caller to have the struct initialized, initially and after the 
message has been "consumed." */
int USART1_PC_msg_get(struct PCTOGATEWAY* ptr)
{
	u8 c;
	int x;

	/* Take chars from input buffer and build the message. */
	while (USART1_rxdma_getcount() > 0) // Input chars available?
	{ // Here, yes.
		c = USART1_rxdma_getchar();	// Fetch one char

		/* Build msg */
		if ( (x=PC_msg_get(ptr,c)) != 0)	// Did this complete a msg?
		{ // Yes.
			return x;  // Completed msg.  Return code shows if victorious glory awaits.
		}
	}
	return 0; // No more bytes to work with, and msg is not complete.
}
/* **************************************************************************************
 * void USART1_toPC_msg(u8* pin, int ct);
 * @brief	: Send msg to PC in binary with framing and byte stuffing 
 * @param	: pin = Pointer to bytes to send to PC
 * @param	: ct = byte count to send (does not include frame bytes, no stuffing bytes)
 * ************************************************************************************** */
void USART1_toPC_msg(u8* pin, int ct)
{
	int i;
	u8 b[PCTOGATEWAYSIZE];	// Sufficiently large output buffer

	/* Prepare msg for sending.  Add byte stuffing, framing, and checksum to input bytes. */
	int sz = PC_msg_prep(&b[0], PCTOGATEWAYSIZE, pin, ct);

	/* Copy to output */
	for (i = 0; i < sz; i++)
		USART1_txdma_putc(b[i]);
	
	USART1_txdma_send();	// Send USART buffer

	return;
}

