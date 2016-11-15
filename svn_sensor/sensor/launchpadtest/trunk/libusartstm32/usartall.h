/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : usartall.h
* Hackor             : deh
* Date First Issued  : 10/05/2010
* Description        : For "all" usart routines
*******************************************************************************/
/*
07-14-2011 - update USART_PRIORITY for POD board use
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USARTALL_H
#define __USARTALL_H

/* Interrupt priority for DMA and USART tx------------------------------------*/
// Note: The lower four bits of the priority byte are not used.
// Note: The higher the priority number, the lower the priority
#define DMA1_TX_PRIORITY	0xa0	// Priority for DMA interrupt
#define USART_PRIORITY		0x40	// Priority for USART interrupt

/* Byte that 'getline' uses for detecting end-of-line (receive)---------------*/
#define END_OF_LINE	0x0d		

/* Includes ------------------------------------------------------------------*/
#include "../libopenstm32/common.h"	// Has things like 'u16' defined

/* Control blocks -------------------------------------------------------------*/

/* Control block for receive */
/* NOTE: the usage of the pointers may vary so look at the comments in the subroutines */
struct USARTCBR	
{
	char*	prx_now_i;	// Working:  pointer within current rx line buffer, interrupt or dma
	char*	prx_begin_m;	// Working:  beginning in current rx line buffer, main
	char*	prx_begin_i;	// Working:  beginning in current rx line buffer, interrupt or dma
	char*	prx_begin;	// Constant: pointer to beginning of all rx line buffers
	char*	prx_end;	// Constant: pointer to end of all rx line buffers
	u16*	prx_ctary_now_m;// Working: pointer to array of counts, main | pointer to getline buffer
	u16*	prx_ctary_now_i;// Working: pointer to array of counts, interrupt | pointer to getline buffer end 
	u16*	prx_ctary_begin;// Constant: pointer to array of counts | pointer to getline buffer begin
	u16	rx_ln_sz;	// Constant: size of one rx line buffer 
	u16	rx_ln_ct;	// Constant: number of line buffers | size of getline buffer
};

/* Control block for transmit */
struct USARTCBT
{
	char*	ptx_now_m;	// Working:  pointer within current tx line buffer, main
	char*	ptx_begin_m;	// Working:  beginning in current tx line buffer, main
	char*	ptx_now_d;	// Working:  pointer within current tx line buffer, dma or int
	char*	ptx_begin_d;	// Working:  beginning in current tx line buffer, dma or int
	char*	ptx_begin;	// Constant: pointer to beginning of all tx line buffers
	char*	ptx_end;	// Constant: pointer to end of all tx line buffers
	u16*	ptx_ctary_now_m;// Working: pointer to array of counts, main
	u16*	ptx_ctary_now_d;// Working: pointer to array of counts, dma or int
	u16*	ptx_ctary_begin;// Constant: pointer to array of counts 
	u16	tx_ln_sz;	// Constant: size of one tx line buffer
	u16	tx_ln_ct;	// Constant: number of line buffers
};



/* --- USART register offsets ----------------------------------------------------- */
#define SR	0x00	/* Status register (USARTx_SR) */
#define DR	0x04	/* Data register (USARTx_DR) */
#define BRR	0x08	/* Baud rate register (USARTx_BRR) */
#define CR1	0x0c	/* Control register 1 (USARTx_CR1) */
#define CR2	0x10	/* Control register 2 (USARTx_CR2) */
#define CR3	0x14	/* Control register 3 (USARTx_CR3) */


/* USART CR1 ------------------------------------------------------------------------------------------------------------------------------------------------------------ */
#define USART_UE			((u16)0b0010000000000000) // Bit 13 UE: USART enable (0=USART prescaler and outputs disabled; 1=USART enabled)
#define USART_WORD_LENGTH		((u16)0b0001000000000000) // Bit 12 M: Word length (0= 1 Start bit, 8 Data bits, n Stop bit;1= 1 Start bit, 9 Data bits, n Stop bit)
#define USART_WAKEUP_METHOD		((u16)0b0000100000000000) // Bit 11 WAKE: Wakeup method (0: Idle Line, 1: Address Mark)
#define USART_PARITY_SELECT		((u16)0b0000010000000000) // Bit 10 PCE: Parity control enable (selects the hardware parity control)
#define USART_PARITY_ODD		((u16)0b0000001000000000) // Bit 9 PS: Parity ODD (0 = EVEN)
#define USART_PE_IMTERRUPT_ENABLE	((u16)0b0000000100000000) // Bit 8 PEIE: PE interrupt enable (whenever PE=1 in the USART_SR register)
#define USART_TX_INTERRUPT_ENABLE	((u16)0b0000000010000000) // Bit 7 TXEIE: TXE interrupt enable (whenever TXE=1 in the USART_SR register)
#define USART_TX_COMPLETE_ENABLE	((u16)0b0000000001000000) // Bit 6 TCIE: Transmission complete interrupt enable (whenever TC=1 in the USART_SR register)
#define USART_RXNE_INTERRUPT_ENABLE	((u16)0b0000000000100000) // Bit 5 RXNEIE: RXNE interrupt enable (whenever ORE=1 or RXNE=1 in the USART_SR)
#define USART_IDLE_INTERRUPT_ENABLE	((u16)0b0000000000010000) // Bit 4 IDLEIE: IDLE interrupt enable (whenever IDLE=1 in the USART_SR register)
#define USART_TX_ENABLE			((u16)0b0000000000001000) // Bit 3 TE: Transmitter enable (Transmitter is enabled)
#define USART_RX_ENABLE			((u16)0b0000000000000100) // Bit 2 RE: Receiver enable (Receiver is enabled and begins searching for a start bit)
#define USART_RCVR_WAKEUP		((u16)0b0000000000000010) // Bit 1 RWU: Receiver wakeup (set/clear by software; can be cleared by hardware from wakeup sequence)
#define USART_SEND_BREAK		((u16)0b0000000000000001) // Bit 0 SBK: Send break (set by software, and will be reset by hardware during the stop bit of break)

/* USART CR3 ------------------------------------------------------------------------------------------------------------------------------------------------------------ */
#define USART_DMAT			((u16)0b0000000010000000) // Bit 7 DMAT: DMA enable transmitter
#define USART_DMAR			((u16)0b0000000001000000) // Bit 6 DMAR: DMA enable receiver


/* USART Flags ---------------------------------------------------------------*/
#define USART_FLAG_TXE                  ((u16)0b0000000010000000) // Bit 7 TXE flag
#define USART_FLAG_TC                   ((u16)0b0000000001000000) // Bit 6 TC flag
#define USART_FLAG_RXNE                 ((u16)0b0000000000100000) // Bit 5 RXNE flag



#endif 


