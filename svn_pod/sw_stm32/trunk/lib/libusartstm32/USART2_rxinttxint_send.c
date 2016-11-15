#include "../libopenstm32/usart.h"
#include "../libfaststm32/usartdma.h"
#include "../libopenstm32/dma.h"
#include "../libfaststm32/usartdmabuff.h"
extern struct USARTCBT* pUSARTcbt2;		// Pointer to control block for this USART
/*******************************************************************************
* void USART2_rxinttxint_send(void);
* @brief	: Step to next line tx line buffer; if DMA not sending, start it now.
* @return	: none
*******************************************************************************/
void USART2_rxinttxint_send(void)
{
	/* Common to all three USARTS */
	usartx_txint_send(pUSARTcbt2);
					
	return;	
}

