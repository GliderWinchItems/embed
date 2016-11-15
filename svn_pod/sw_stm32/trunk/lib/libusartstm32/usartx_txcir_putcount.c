/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : usartx_txcir_putcount.c 
* Hackor             : deh
* Date First Issued  : 10/14/2010 deh
* Description        : Get free tx buffer count: USART2 tx char-by-char interrupt with circular buffer
*******************************************************************************/ 
#include "../libusartstm32/usartprotoprivate.h" // Subroutine prototypes for internal use
#include "../libusartstm32/usartbitband.h" 	// Register defines for bit band use
/* NOTE -- control block usage
char*	ptx_now_m;	// Working:  pointer in circular buffer, main
char*	ptx_begin_m;	// Working:  --not used--
char*	ptx_now_d;	// Working:  pointer in circular buffer, interrupt
char*	ptx_begin_d;	// Working:  --not used--
char*	ptx_begin;	// Constant: pointer to beginning of tx circular buffer
char*	ptx_end;	// Constant: pointer to end of tx circular buffer
u16*	ptx_ctary_now_m;// Working:  --not used--
u16*	ptx_ctary_now_d;// Working:  --not used--
u16*	ptx_ctary_begin;// Constant: --not used--
u16	tx_ln_sz;	// Constant: Size of circular buffer
u16	tx_ln_ct;	// Constant: --not used--
*/
/*******************************************************************************
* u16 usartx_txcir_putcount(u32* usartx_txflag_bitband, struct USARTCBT* pUSARTcbtx);
* @brief	: Get number of chars available in circular buffer
* @param	: u32* usartx_txflag_bitband: 	Bit-band address for usart tx flag
* @param	: pUSARTcbx:	Pointer to struct--pUSARTcb1,2,3
* @return	: 0 = no chars available, + = count of chars free
*******************************************************************************/
u16 usartx_txcir_putcount(u32* usartx_txflag_bitband, struct USARTCBT* pUSARTcbtx)
{
	/* First compute difference between where chars are being added and being taken out */
	u16 Diff = (pUSARTcbtx->ptx_now_m - pUSARTcbtx->ptx_now_d);
	/* If they are the same, there can be two situations */
	if (Diff == 0)
	{ // Here the pointers are the same, so see which situation we have.
		if (*usartx_txflag_bitband == 0) 	// Are we already transmitting?
		{
			return 0;			// Yes, then the circular buffer is full (but not for long!).
		}
		else 
		{
			return 	pUSARTcbtx->tx_ln_sz;	// No, the circular buffer is empty
		}
	}
	if (Diff > 0)					// Check for wrap-around
	{
		return Diff;				// Return free space
	}
	return (Diff + pUSARTcbtx->tx_ln_sz);		// Adjust for wraparound
}

