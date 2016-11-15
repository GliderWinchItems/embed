/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : usartx_rxint.c 
* Hackor             : deh
* Date First Issued  : 10/12/2010 deh
* Description        : USARTx rx char-by-char interrupts using line buffers
*******************************************************************************/ 



#include "../libopenstm32/usart.h"
#include "../libopenstm32/memorymap.h"

//#include "../libusartstm32/mymalloc.h"
#include <stdlib.h>
#include "../libusartstm32/usartprotoprivate.h" // Subroutine prototypes for internal use


/* NOTE:
The struct pointer usage:
char*	prx_now_i;	// Working:  pointer in line buffer currently being filled (interrupt)
char*	prx_begin_m;	// Working:  pointer to beginning of current line buffer (main)
char*	prx_begin_i;	// Constant: pointer to beginning of current line buffer (interrupt)
char*	prx_begin;	// Constant: pointer to beginning of first line buffer
har*	prx_end;	// Constant: pointer to end of last line buffer
u16*	prx_ctary_now_m;// Working:  pointer to count of chars in corresponding line buffer
u16*	prx_ctary_now_i;// Working:  pointer to count of chars in corresponding line buffer
u16*	prx_ctary_begin;// Constant: pointer to beginning of array of char counts
u16	rx_ln_sz;	// Constant: size of one line buffer
u16	rx_ln_ct;	// Constant: number of line buffers
*/

/******************************************************************************
 *u16 usartx_rxint_allocatebuffers (u16 rcvlinesize, u16 numberrcvlines, struct USARTCBR** pUSARTcbrx);
 * @brief	: Allocate buffers with 'mymalloc' for receive and setup control block pointers
 * @param	: rcvlinesize is size of each line buffer (e.g. 48)
 * @param	: numberrcvlines is the number of receive line buffers (e.g. 4)
 * @param	: pUSARTcbrx points to pointer in static memory that points to control block
 * @return	: 0 = success; 1 = fail malloc; 2 = rcvlinesize zero; 3 = numberrcvlines = 0; 
******************************************************************************/
u16 usartx_rxint_allocatebuffers (u16 rcvlinesize, u16 numberrcvlines, struct USARTCBR** pUSARTcbrx)
{
	u32* ptr;
	struct USARTCBR* pUSARTcbrl; // Local pointer to control block setup on heap

	/* Bomb out just in case buffer size would be zero */
	if (rcvlinesize    == 0) return 2;
	if (numberrcvlines == 0) return 3;

	/* Allocate space and set pointer for the struct USARTcbrx */
	ptr = (u32*)malloc(sizeof (struct USARTCBR));
	if  ( ptr  == 0)  return 1;
	*pUSARTcbrx = (struct USARTCBR*)ptr;	// Initialize static variable used by others
	 pUSARTcbrl = (struct USARTCBR*)ptr;	// Local pointer

	/* ---------------------------- receive buffers and pointers ------------------------------- */
	/* Allocate space and set pointer in USARTcbrx for receive line buffer(s) */
	if ( (pUSARTcbrl->prx_begin = (char*)malloc(rcvlinesize * numberrcvlines)) == 0 ) return 1;

	/* These rx pointers to start out on the same line buffer */	
	pUSARTcbrl->prx_begin_m = pUSARTcbrl->prx_begin;	// Beginning of line (main)
	pUSARTcbrl->prx_begin_i = pUSARTcbrl->prx_begin;	// Beginning of line being filled (interrupt)
	pUSARTcbrl->prx_now_i   = pUSARTcbrl->prx_begin;	// Pointer within line being filled (interrupt)

	/* Use this for testing for end of line buffers wraparound */
	pUSARTcbrl->prx_end     = pUSARTcbrl->prx_begin + (rcvlinesize * numberrcvlines); 

	/* Save rx line buffer size */
	pUSARTcbrl->rx_ln_sz = rcvlinesize;

	/* Save number of rx line buffers */
	pUSARTcbrl->rx_ln_ct = numberrcvlines;

	/* Allocate space and set pointer in USARTcbrx for char count array */
	if ( (pUSARTcbrl->prx_ctary_begin = malloc(sizeof(u16) * numberrcvlines)) == 0 ) return 1;
	 pUSARTcbrl->prx_ctary_now_m = pUSARTcbrl->prx_ctary_begin; 	// main pointer
	 pUSARTcbrl->prx_ctary_now_i = pUSARTcbrl->prx_ctary_begin; 	// isr pointer
	*pUSARTcbrl->prx_ctary_now_m = 0; 				// Show no chars in initial line buffer
	return 0;
}
/******************************************************************************
 * void usartx_rxint_usart_init (u32 USARTx, u32 BaudRate);
 * @brief	: Setup USART baudrate, Rx interrupting with line buffers
 * @param	: USARTx = USART1, USART2, or USART3 base address
 * @param	: u32 BaudRate is the baud rate.
******************************************************************************/
void usartx_rxint_usart_init (u32 USARTx, u32 BaudRate)
{
	/* Set baud rate */
	usartx_setbaud (USARTx,BaudRate);	// Compute divider settings and load BRR

 	/* Setup CR2 ------------------------------------------------------------------- */
	/* After reset CR2 is 0x0000 and this is just fine */
	/* Set up CR1 (page 771) ------------------------------------------------------- */
	USART_CR1(USARTx) |= USART_UE | USART_RXNE_INTERRUPT_ENABLE | USART_RX_ENABLE;// Set UE, RNEIE, RE 

	return;	
}
/* NOTE:
The struct pointer usage:
char*	prx_now_i;	// Working:  pointer in line buffer currently being filled (interrupt)
char*	prx_begin_m;	// Working:  pointer to beginning of current line buffer (main)
char*	prx_begin_i;	// Constant: pointer to beginning of current line buffer (interrupt)
char*	prx_begin;	// Constant: pointer to beginning of first line buffer
har*	prx_end;	// Constant: pointer to end of last line buffer
u16*	prx_ctary_now_m;// Working:  pointer to count of chars in corresponding line buffer
u16*	prx_ctary_now_i;// Working:  pointer to count of chars in corresponding line buffer
u16*	prx_ctary_begin;// Constant: pointer to beginning of array of char counts
u16	rx_ln_sz;	// Constant: size of one line buffer
u16	rx_ln_ct;	// Constant: number of line buffers
*/
/******************************************************************************
 * char* usartx_rxint_rxmainadvptr (struct USARTCBR* pUSARTcbrx);
 * @brief	: Advance pointers for mainline
 * @param	: Receive control block
 * @return	: 0 = caught up, not zero = pointer to completed line buffer
******************************************************************************/
char* usartx_rxint_rxmainadvptr (struct USARTCBR* pUSARTcbrx)
{
	char* p;	// Temp 

	/* If no chars, then we are caught up with incoming lines */
	if (*pUSARTcbrx->prx_ctary_now_m == 0) return 0;
	
	p = pUSARTcbrx->prx_begin_m;		// Save pointer for later return

	/* Advance to next line buffer and companion array of char cts */
	pUSARTcbrx->prx_begin_m     += pUSARTcbrx->rx_ln_sz;	// Line buffer
	pUSARTcbrx->prx_ctary_now_m += 1;	// Array of char cts

	/* Check if at the end of all the line buffers */
	if (pUSARTcbrx->prx_begin_m >= pUSARTcbrx->prx_end)
	{ // Here, end of line buffers, reset pointers to beginning
		pUSARTcbrx->prx_begin_m     = pUSARTcbrx->prx_begin;	// Line buffer
		pUSARTcbrx->prx_ctary_now_m = pUSARTcbrx->prx_ctary_begin;// Array of char cts
	}
	return p;
}
/******************************************************************************
 * void usartx_rxint_rxisrptradv2 (struct USARTCBR* pUSARTcbrx);
 * @brief	: Step pointers to next line buffer for rx isr routine
******************************************************************************/
void usartx_rxint_rxisrptradv2 (struct USARTCBR* pUSARTcbrx)
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
}
/******************************************************************************
 * void usartx_rxint_rxisrptradv (struct USARTCBR* pUSARTcbrx);
 * @brief	: Check for EOL, advance pointers for rx isr routine
******************************************************************************/
void usartx_rxint_rxisrptradv (struct USARTCBR* pUSARTcbrx)
{	
	/* Was the char just stored an END_OF_LINE? */
	if ( *pUSARTcbrx->prx_now_i++ == END_OF_LINE)	// Check for end of line, step to next location
	{ // Here, EOL, so move to next line buffer.
		/* Store number of chars in line just completed */
		*pUSARTcbrx->prx_ctary_now_i = (pUSARTcbrx->prx_now_i - pUSARTcbrx->prx_begin_i);
		/* Check if space to store zero string terminator */
		if (pUSARTcbrx->prx_now_i < (pUSARTcbrx->prx_begin_i + pUSARTcbrx->rx_ln_sz) )
		{ // Here, there is room for the zero
			*pUSARTcbrx->prx_now_i = 0; 	// Add zero for the string terminator			
		}
		/* Step to next line buffer */
		usartx_rxint_rxisrptradv2 (pUSARTcbrx);
	}
	else
	{ // Here, not EOL, so check if we are at end of the current line buffer.
		if (pUSARTcbrx->prx_now_i >= (pUSARTcbrx->prx_begin_i + pUSARTcbrx->rx_ln_sz) )
		{ // Here, we at end, so step to next line buffer
			/* Store number of chars in line just completed */
			*pUSARTcbrx->prx_ctary_now_i = (pUSARTcbrx->prx_now_i - pUSARTcbrx->prx_begin_i);
			usartx_rxint_rxisrptradv2 (pUSARTcbrx);
		}
	}
	return;
}

