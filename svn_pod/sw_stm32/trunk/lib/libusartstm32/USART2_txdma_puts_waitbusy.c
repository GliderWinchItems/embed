/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : USART2_txdma_puts_waitbusy.c 
* Hackor             : deh
* Date First Issued  : 10/05/2010 deh
* Description        : Check and loop busy before putting a string to the buffer
*******************************************************************************/ 
#include "../libusartstm32/usartprotoprivate.h"
#include "../libusartstm32/usartall.h"

/*******************************************************************************
* char* USART2_txdma_puts_waitbusy(char* p);
* @brief	: Wait if busy, then add string
* @return	: 0 = OK; otherwise,buffer full and 'p' pointing to char not stored
*******************************************************************************/
char* USART2_txdma_puts_waitbusy(char* p)
{
//	while (USART2_txdma_busy() == 1); // 10/08/2010 no longer needed!	
	return USART2_txdma_puts(p);
}

