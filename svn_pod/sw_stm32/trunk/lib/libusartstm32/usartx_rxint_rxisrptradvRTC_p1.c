/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : usartx_rxintRTC_p1.c 
* Hackor             : deh
* Date First Issued  : 07/14/2011 deh
* Description        : USARTx rx char-by-char interrupts using line buffers adding RTC count
*******************************************************************************/ 
/* 
This routine is a copy of the following--
'usartx_rxint_rxisrptradv.c' (located in usartx_rxint.c)
with the addition of appending the RTC system time counter after the EOL and zero line
terminator.  This results in--
...chars...EOL, 0, 32 bit int

This allows a GPS NMEA line that has just been recevied to be associated with the RTC CNT 
register time with a minimum of delay.
*/

#include "../libopenstm32/usart.h"
#include "../libopenstm32/memorymap.h"

#include "../libusartstm32/mymalloc.h"
#include "../libusartstm32/usartprotoprivate.h" // Subroutine prototypes for internal use

/* Holds the RTC CNT register count that is maintained in memory during power up */
extern volatile long long	t64RTCsystemcounterTim2IC;	// Save RTC count upon each Tim2 IC

/******************************************************************************
 * void usartx_rxint_rxisrptradv2 (struct USARTCBR* pUSARTcbrx);
 * @brief	: Step pointers to next line buffer for rx isr routine
******************************************************************************/
void usartx_rxint_rxisrptradv2R (struct USARTCBR* pUSARTcbrx)
{
	pUSARTcbrx->prx_ctary_now_i += 1;			// Step to next char ct arrary position
	pUSARTcbrx->prx_begin_i     += pUSARTcbrx->rx_ln_sz;	// Step to next line buffer beginning
	if (pUSARTcbrx->prx_begin_i >= pUSARTcbrx->prx_end)	// Are we at end of the line buffers?
	{ // Here, yes.  Reset pointers to beginning
		pUSARTcbrx->prx_ctary_now_i  = pUSARTcbrx->prx_ctary_begin;
		pUSARTcbrx->prx_begin_i      = pUSARTcbrx->prx_begin;			
	}
	pUSARTcbrx->prx_now_i = pUSARTcbrx->prx_begin_i;	// Set working pointer.
	return;
}/******************************************************************************
 * void usartx_rxint_rxisrptradvRTC_p1 (struct USARTCBR* pUSARTcbrx);
 * @brief	: Check for EOL, advance pointers for rx isr routine; Add RTC at end of line
******************************************************************************/
void usartx_rxint_rxisrptradvRTC_p1 (struct USARTCBR* pUSARTcbrx)
{	
	/* Was the char just stored an END_OF_LINE? */
	if ( *pUSARTcbrx->prx_now_i++ == END_OF_LINE)	// Check for end of line, step to next location
	{ // Here, EOL, so move to next line buffer.
		/* Store number of chars in line just completed */
		*pUSARTcbrx->prx_ctary_now_i = (pUSARTcbrx->prx_now_i - pUSARTcbrx->prx_begin_i);
		/* Check if space to store zero string terminator + 4 byte RTC counter */
		if (pUSARTcbrx->prx_now_i < (pUSARTcbrx->prx_begin_i + pUSARTcbrx->rx_ln_sz + 9) )
		{ // Here, there is room for the zero
			*pUSARTcbrx->prx_now_i++ = 0; 	// Add zero for the string terminator			
			/*  Read high order word and OR with low order word of counter (p 455) */
			*(unsigned long long *)(pUSARTcbrx->prx_now_i) = t64RTCsystemcounterTim2IC;	// Add 32 bit counter
		}
		else
		{
			*(pUSARTcbrx->prx_now_i - 1) = 0xff;	// Ran out of space error
		}
		/* Step to next line buffer */
		usartx_rxint_rxisrptradv2R (pUSARTcbrx);
	}
	else
	{ // Here, not EOL, so check if we are at end of the current line buffer.
		if (pUSARTcbrx->prx_now_i >= (pUSARTcbrx->prx_begin_i + pUSARTcbrx->rx_ln_sz) )
		{ // Here, we at end, so step to next line buffer
			/* Store number of chars in line just completed */
			*pUSARTcbrx->prx_ctary_now_i = (pUSARTcbrx->prx_now_i - pUSARTcbrx->prx_begin_i);
			usartx_rxint_rxisrptradv2R (pUSARTcbrx);
		}
	}
	return;
}


