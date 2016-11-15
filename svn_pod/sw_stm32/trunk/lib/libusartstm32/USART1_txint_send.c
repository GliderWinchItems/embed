/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : USART1_txint_send.c
* Hackor             : deh
* Date First Issued  : 10/16/2010
* Description        : Step to the next line buffer, thus making this line buffer ready
*******************************************************************************/
#include "../libusartstm32/usartprotoprivate.h" // Subroutine prototypes for internal use
#include "../libusartstm32/usartbitband.h" 	// Register defines for bit band use
/*******************************************************************************
* void USART1_txint_send(void);
* @brief	: Step to next line tx line buffer; if DMA not sending, start it now.
* @return	: none
*******************************************************************************/
void USART1_txint_send(void)
{
	/* Common to all three USARTS */
	if (usartx_txint_send(pUSARTcbt1) != 0) return;
			
	MEM_ADDR(BITBAND(USART1CR1,TXFLAG)) = 0x01;	// Enable Tx interrupts

	return;	
}

