/******************************************************************************
* File Name          : bsp_uart.h
* Date First Issued  : 11/06/2013
* Board              : Discovery F4 (F405 or F407)
* Description        : Board Specific Support for uart routines
*******************************************************************************/
#ifndef __BSP_UART
#define __BSP_UART


#include "libopencm3/stm32/memorymap.h"
#include "libopencm3/cm3/common.h"

struct CB_UART
{
	volatile u32	idma;	// DMA base address
	volatile u32	iuart;	// uart base address
	u32		rxdma_stream; // Stream number (0 - 7)
	u32		txdma_stream; // Stream number (0 - 7)
	u32		flag;	// 0 = not initialized; 1 = interrupt; 2 = dma
	u8		nstop;	/* Number of Stop bits: 
		USART_CR2_STOPBITS_1		(0x00 << 12)      1 stop bit 
		USART_CR2_STOPBITS_0_5		(0x01 << 12)      0.5 stop bits 
		USART_CR2_STOPBITS_2		(0x02 << 12)      2 stop bits 
		USART_CR2_STOPBITS_1_5		(0x03 << 12)      1.5 stop bits */

	/* TX buffering */
	u32	txbuff_size;	// Buffer size
	u8*	txbuff_base;	// Low end of buffer
	u8*	txbuff_end;	// Low end + size
	u8*	txbuff_in;	// Pointer for adding to buffer
	u8*	txbuff_out;	// Pointer for removing from buffer
	u8*	txbuff_dmanext;	// 'out = 'dmanext when dma tx completes (interrupts)
	u8	txblock;	// Buff overflow: 0 = blocking; 1 = discard chars

	/* Rx buffering */
	u8*	rxbuff_base;
	u8*	rxbuff_end;
	u32	rxbuff_size;
	u8*	rxbuff_in;
	u8*	rxbuff_out;	
};


/******************************************************************************/
struct CB_UART* bsp_uart_open(const char *name);
/* @brief	: Translate name string to file descriptor for 'open'
 * @return	: control block pointer
 ******************************************************************************/
int bsp_uart_dma_init_number(u32 uartnumber, u32 baud,u32 rxbuffsize, u32 txbuffsize, u32 dmastreamrx, u32 dmastreamtx, u32 dma_tx_int_priority, u8 block, u8 nstop);
/* @brief	: A wrapper for 'int bsp_uart_dma_init' with uart number (1-6) rather than uart base address as the first argument
 * @return	: Same as bsp_uart_dma_init
*******************************************************************************/
int bsp_uart_int_init_number(u32 uartnumber, u32 baud, u32 rxbuffsize, u32 txbuffsize,  u32 uart_int_priority, u8 block, u8 nstop);
/* @brief	: A wrapper for 'int bsp_uart_int_init' with uart number (1-6) rather than uart base address as the first argument
 * @return	: Same as bsp_uart_int_init
*******************************************************************************/
int bsp_uart_int_init(u32 iuart, u32 baud, u32 rxbuffsize, u32 txbuffsize,  u32 uart_int_priority, u8 block, u8 nstop);
/* @brief	: Initialize USART/UART for DMA transfers
 * @param	: iuart: pointer to UART base, e.g. 'USART1'
 * @param	: baud: name says it all, e.g. '921600'
 * @param	: rxbuffsize: number of bytes in a circular rx buffer
 * @param	: txbuffsize" number of bytes in a circular tx buffer
 * @param	: uart_int_priority: interrupt priority, (0x00 - 0xf0) e.g. 0xc0, low 4 bits zero
 * @param	: block: 0 = loop when buffer full; non-zero = discard char when buff full
 * @param	: nstop: 0 = 1.0; 1 = 0.5; 2 = 2; 3 = 1.5 bits
 * @return	: 0 = success; fail traps to 'panic_leds'
*******************************************************************************/
int bsp_uart_dma_init(u32 iuart, u32 baud, u32 rxbuffsize, u32 txbuffsize, u32 dmastreamrx, u32 dmastreamtx, u32 dma_tx_int_priority, u8 block, u8 nstop);
/* @brief	: Initialize USART/UART for DMA transfers
 * @param	: iuart: pointer to UART base, e.g. 'USART1'
 * @param	: baud: name says it all, e.g. '921600'
 * @param	: rxbuffsize: number of bytes in a circular tx buffer
 * @param	: txbuffsize" number of bytes in a circular rx buffer
 * @param	: dmastreamrx: DMA stream number for RX (0 - 7)
 * @param	: dmastreamtx: DMA stream number for TX (0 - 7)
 * @param	: dma_tx_int_priority: interrupt priority, (0x00 - 0xf0) e.g. 0xc0, low 4 bits zero
 * @param	: block: 0 = loop when buffer full; non-zero = discard char when buff full
 * @param	: nstop: 0 = 1.0; 1 = 0.5; 2 = 2; 3 = 1.5 bits
 * @return	: 0 = success; fail traps to 'panic_leds'
*******************************************************************************/
int bsp_uart_getcount_ptr    (struct CB_UART* pctl);	// Select: control blk ptr
int bsp_uart_getcount_uartnum(int uartnum);	// Select: uart number
int bsp_uart_getcount_fd(int fd); 		// Select: file descriptor
/* @brief	: Get the number of bytes buffered
 * @param	: pctl = pointer uart control block
 * @return	: number of chars in currently buffered.
*******************************************************************************/
char bsp_uart_getc_ptr    (struct CB_UART* pctl);	// Select: control blk ptr
char bsp_uart_getc_uartnum(int uartnum);		// Select: uart number
char bsp_uart_getc_fd(int fd); 			// Select: file descriptor
/* @brief	: Get one char
 * @param	: pctl = pointer uart control block
 * @return	: char from buffer
*******************************************************************************/
int bsp_uart_getn_ptr(struct CB_UART* pctl, char *pchr, int len);	// Select: control blk ptr
int bsp_uart_getn_uartnum(int uartnum, char *pchr, int len);		// Select: uart number
int bsp_uart_getn_fd(int fd, char *pchr, int len); 			// Select: file descriptor
/* @brief	: Attempt to get 'len' number of chars
 * @param	: pctl = pointer uart control block
 * @param	: pchr = pointer to output char buffer
 * @param	: len = number of bytes requested
 * @return	: number of chars actually transferrederred
*******************************************************************************/
void bsp_uart_start_dma(struct CB_UART* pctl);
/* @brief	: If DMA driven uart, start DMA sending if not already sending
 * @param	: pctl: control block poiner
 * @return	:
*******************************************************************************/
void bsp_uart_send_int(struct CB_UART* pctl);
/* @brief	: If uart tx is idle, and not dma driven, start tx
 * @param	: pctl = pointer uart control block
 * @return	: 
*******************************************************************************/
int bsp_uart_putc_ptr(struct CB_UART* pctl, char c); // Select: control blk ptr
int bsp_uart_putc_uartnum(int uartnum, char c);	// Select: uart number
int bsp_uart_putc_fd(int fd, char c);		// Select: file descriptor
/* @brief	: Put char.  Add a char to output buffer
 * @param	: pctl = pointer uart control block
 *       	: uartnum = uart number 1 - n
 *       	: fd = file descriptor
 * @param	: Char to be sent
 * @return	: 1 = one char add; 0 = no chars added;
 * NOTE: this does not start an idle DMA, but starts an idle interrupt driven uart
*******************************************************************************/
int bsp_uart_puts_ptr(struct CB_UART* pctl, char* p); // Select: control blk ptr
int bsp_uart_puts_uartnum(int uartnum,char* p);	// Select: uart number
int bsp_uart_puts_fd(int fd, char* p);		// Select: file descriptor
/* @brief	: Put String:  Add a zero terminated string to output buffer
 * @param	: pctl = pointer uart control block
 *       	: uartnum = uart number 1 - n
 *       	: fd = file descriptor
 * @param	: p = pointer to buffer with bytes to be added
 * @return	: count of bytes added to buffer
 * NOTE: this starts either DMA or interrupt driven 
*******************************************************************************/
int bsp_uart_putn_ptr(struct CB_UART* pctl, char* p, int len); // Select: control blk ptr
int bsp_uart_putn_uartnum(int uartnum, char* p, int len);	// Select: uart number
int bsp_uart_putn_fd(int fd, char* p, int len);		// Select: file descriptor
/* @brief	: Put char wit count.  Add 'n' chars to output buffer
 * @param	: pctl = pointer uart control block
 *       	: uartnum = uart number 1 - n
 *       	: fd = file descriptor
 * @param	: p = input char pointer
 * @param	: count = number of bytes to be added
 * @return	: count of bytes added to buffer
 * NOTE: this starts either DMA or interrupt driven 
*******************************************************************************/



/*#######################################################################################
 * ISR DMA TX routine: Entered from nvic_dma_mgr.c which dispatches the DMAx_STREAMy interrupts */
void DMA_UART_IRQHandler(volatile u32* pctl); // USART1 Tx
/* @param	: pctl = pointer to uart control block
 *####################################################################################### */










#endif 

