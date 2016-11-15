/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : xb_at_cmd.c
* Hackee             : deh
* Date First Issued  : 03/24/2012
* Board              : STM32F103VxT6_pod_mm with XBee
* Description        : Send AT command
*******************************************************************************/

#include "libusartstm32/usartallproto.h"
#include "libopenstm32/usart.h"
#include "libmiscstm32/systick_delay.h"

/* ************************************************************
 * void xb_send_AT_cmd(char * p);
 * @brief	: Send command(s) to XB
 * @param	: p = pointer to string to send
***************************************************************/
void xb_send_AT_cmd(char * p)
{
	USART1_txint_puts(p);		// Echo back the line just received
	USART1_txint_puts ("\n");	// Add line feed to make things look nice
	USART1_txint_send();		// Send to USART1

	/* Send command out USART2* */
	USART2_txdma_puts("+++");	// Sequence to enter command mode 
	delay_systick(1100);		// No-character guard delay following "+++"
	USART2_txdma_puts(p);		// Send command to XBee
	USART2_txdma_puts("\r");	// Add <CR> (that triggers response?)

	return;
}


