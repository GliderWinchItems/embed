/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : usartproto.h
* Hackor             : deh
* Date First Issued  : 10/01/2010
* Description        : usart subroutine prototypes
*******************************************************************************/
/*
07-14-2011 Added USARTremap routine and UART4,5 rxinttxint routines

*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USARTALLPROTO_H
#define __USARTALLPROTO_H
#include "../libopenstm32/common.h"


/******************************************************************************/
u16 USART1_dma_init (u32 BaudRate,u16 rcvcircularsize, u16 rcvlinesize, u16 xmtlinesz, u16 numberxmtlines);
u16 USART2_dma_init (u32 BaudRate,u16 rcvcircularsize, u16 rcvlinesize, u16 xmtlinesz, u16 numberxmtlines);
u16 USART3_dma_init (u32 BaudRate,u16 rcvcircularsize, u16 rcvlinesize, u16 xmtlinesz, u16 numberxmtlines);
/* @brief	: Initializes the USARTx to 8N1 and the baudrate for interrupting 
 * @param	: u32 BaudRate is the baud rate.
 * @param	: u16 rcvlinesize 	- size of each receive line buffer (e.g. 32)
 * @param	: u16 numberrcvlines	- number of receive line buffers, (e.g. 4)
 * @param	: u16 xmtlnsz		- xmit line size, (e.g. 80)
 * @param	: u16 numberxmtlines	- number of xmit line buffers, (e.g. 4)
 * @return	: u16 0 = success; 1 = failed memory allocation
******************************************************************************/
u16 USART1_dma_getcount(void);
u16 USART2_dma_getcount(void);
u16 USART3_dma_getcount(void);
/* @brief	: Get the number of chars in the input/read buffer.
* @return	: number of chars in currently buffered.
*******************************************************************************/
char USART1_dma_getchar(void);
char USART2_dma_getchar(void);
char USART3_dma_getchar(void);
/*@brief	: Get one char from the input (rx) circular buffer.
* @return	: a nice round char
*******************************************************************************/
char* USART1_dma_getline(void);
char* USART2_dma_getline(void);
char* USART3_dma_getline(void);
/* @brief	: Assemble one 0x0d (END_OF_LINE) terminated line from the input/read buffer.
* @return	: 0 == line not complete, != 0 is ptr to completed line buffer
*******************************************************************************/
char USART1_dma_putc(char c);
char USART2_dma_putc(char c);
char USART3_dma_putc(char c);
/* @brief	: Put char.  Add a char to output buffer
* @param	: Char to be sent
* @return	: == 0 for buffer did not overflow; == 1 overflow, chars lost
*******************************************************************************/
char* USART1_dma_puts(char* p);
char* USART2_dma_puts(char* p);
char* USART3_dma_puts(char* p);
/* @brief	: Add a zero terminated string to the output buffer.
* @param	: Pointer to zero terminated string
* @return	: == 0 for buffer did not overflow; != pointer p for next char not stored
*******************************************************************************/
u16 USART1_dma_busy(void);
u16 USART2_dma_busy(void);
u16 USART3_dma_busy(void);
/* @brief	: Check for buffer overrun.  
* @return	: == 1 all line buffers filled; == 0 free line buffer(s)
*******************************************************************************/
void USART1_dma_send(void);
void USART2_dma_send(void);
void USART3_dma_send(void);
/* @brief	: Start, or mark, current usart tx line buffer for sending via DMA
* @return	: none
*******************************************************************************/
char* USART1_dma_puts_waitbusy(char* p);
char* USART2_dma_puts_waitbusy(char* p);
char* USART3_dma_puts_waitbusy(char* p);
/*@brief	: Wait if busy, then execute send
* @return	: 0 = OK; otherwise,buffer full and 'p' pointing to char not stored
*******************************************************************************/




/******************************************************************************/
u16 USART1_rxinttxdma_init (u32 BaudRate,u16 rcvlinesize, u16 numberrcvlines, u16 xmtlinesize, u16 numberxmtlines);
u16 USART2_rxinttxdma_init (u32 BaudRate,u16 rcvlinesize, u16 numberrcvlines, u16 xmtlinesize, u16 numberxmtlines);
u16 USART3_rxinttxdma_init (u32 BaudRate,u16 rcvlinesize, u16 numberrcvlines, u16 xmtlinesize, u16 numberxmtlines);
/* @brief	: Allocate buffers with 'mymalloc' for receive and setup control block pointers
 * @param	: Baudrate is baudrate (e.g. 115200)
 * @param	: rcvlinesize is size of each line buffer (e.g. 48)
 * @param	: numberrcvlines is the number of receive line buffers (e.g. 4)
 * @param	: u16 xmtlinesize		- xmit line size, (e.g. 80)
 * @param	: u16 numberxmtlines	- number of xmit line buffers, (e.g. 4)
 * @param	: pUSARTcbrx points to pointer in static memory that points to control block
 * @return	: 1 = fail; 0 = success
*******************************************************************************/
u16 USART1_rxinttxdma_busy(void);
u16 USART2_rxinttxdma_busy(void);
u16 USART3_rxinttxdma_busy(void);
/* @brief	: Check for buffer overrun.  
* @return	: == 1 all line buffers filled; == 0 free line buffer(s)
*******************************************************************************/
u16 USART1_rxinttxdma_getcount(void);
u16 USART2_rxinttxdma_getcount(void);
u16 USART3_rxinttxdma_getcount(void);
/* @brief	: Get the number of chars in the input/read buffer.
* @return	: number of chars in currently buffered.  Zero == buffer not ready
*******************************************************************************/
char* USART1_rxinttxdma_getlineptr(void);
char* USART2_rxinttxdma_getlineptr(void);
char* USART3_rxinttxdma_getlineptr(void);
/* @brief	: Get the pointer to the current line buffer 
* @return	: point to buffer; zero = buffer not ready
*******************************************************************************/
void USART1_rxinttxdma_done(void);
void USART2_rxinttxdma_done(void);
void USART3_rxinttxdma_done(void);
/* @brief	: Done with current buffer, step to next 
* @return	: none
*******************************************************************************/
char USART1_rxinttxdma_putc(char c);
char USART2_rxinttxdma_putc(char c);
char USART3_rxinttxdma_putc(char c);
/* @brief	: Put char.  Add a char to output buffer
* @param	: Char to be sent
* @return	: == 0 for buffer did not overflow; == 1 overflow, chars lost
*******************************************************************************/
char* USART1_rxinttxdma_puts(char* p);
char* USART2_rxinttxdma_puts(char* p);
char* USART3_rxinttxdma_puts(char* p);
/* @brief	: Add a zero terminated string to the output buffer.
* @param	: Pointer to zero terminated string
* @return	: == 0 for buffer did not overflow; != pointer p for next char not stored
*******************************************************************************/
char* USART1_rxinttxdma_puts_waitbusy(char* p);
char* USART2_rxinttxdma_puts_waitbusy(char* p);
char* USART3_rxinttxdma_puts_waitbusy(char* p);
/* @brief	: Wait if busy, then execute send
* @return	: 0 = OK; otherwise,buffer full and 'p' pointing to char not stored
*******************************************************************************/
void USART1_rxinttxdma_send(void);
void USART2_rxinttxdma_send(void);
void USART3_rxinttxdma_send(void);
/* @brief	: Step to next line tx line buffer; if DMA not sending, start it now.
* @return	: none
*******************************************************************************/
/* ######################################################################################## */
/* ########################## txmin ####################################################### */
/* ######################################################################################## */
/*******************************************************************************/
void USART1_txmin_init (u32 BaudRate);
void USART2_txmin_init (u32 BaudRate);
void USART3_txmin_init (u32 BaudRate);
void UART5_txmin_init (u32 BaudRate);
/* @brief	: Initializes the USARTx to 8N1 and the baudrate for interrupting receive into line buffers
 * @param	: u32 Baudrate		- baudrate 				(e.g. 115200)
*******************************************************************************/
u16 USART1_txmin_txbusy(void);
u16 USART2_txmin_txbusy(void);
u16 USART3_txmin_txbusy(void);
u16 UART5_txmin_txbusy(void);
/* @brief	: Check for USART transmit register empty
 * @return	: 0 = busy, transmit register NOT emtpy; non-zero = emtpy, ready for char
******************************************************************************/
void USART1_txmin_putc(char c);
void USART2_txmin_putc(char c);
void USART3_txmin_putc(char c);
void UART5_txmin_putc(char c);
/* @brief	: Put char.  Send a char
* @param	: Char to be sent
* @return	: none
*******************************************************************************/
void USART1_txmin_puts(char* p);
void USART2_txmin_puts(char* p);
void USART3_txmin_puts(char* p);
void UART5_txmin_puts(char* p);
/* @brief	: Send a zero terminated string
* @param	: Pointer to zero terminated string
* @return	: none
*******************************************************************************/
/* ######################################################################################## */
/* ########################## rxmin ####################################################### */
/* ######################################################################################## */
/******************************************************************************/
void USART1_rxmin_init (u32 BaudRate);
void USART2_rxmin_init (u32 BaudRate);
void USART3_rxmin_init (u32 BaudRate);
void UART5_rxmin_init (u32 BaudRate);
/* @brief	: Initializes the USARTx to 8N1 and the baudrate for interrupting receive into line buffers
 * @param	: u32 Baudrate		- baudrate 	(e.g. 115200)
*******************************************************************************/
char USART1_rxmin_rxready(void);
char USART2_rxmin_rxready(void);
char USART3_rxmin_rxready(void);
char UART5_rxmin_rxready(void);
/* @brief	: Check for USART transmit register empty
 * @return	: 0 = no char, 1 = char waiting, 2 or more = chars missed
******************************************************************************/
char USART1_rxmin_getchar(void);
char USART2_rxmin_getchar(void);
char USART3_rxmin_getchar(void);
char UART5_rxmin_getchar(void);
/* @brief	: Get a char from the single USART char buffer
 * @return	: Char received
******************************************************************************/
/* ######################################################################################## */
/* ########################## txdma ####################################################### */
/* ######################################################################################## */
/*******************************************************************************/
u16 USART1_txdma_init (u32 BaudRate,u16 XmtLineSize, u16 NumberXmtLines);
u16 USART2_txdma_init (u32 BaudRate,u16 XmtLineSize, u16 NumberXmtLines);
u16 USART3_txdma_init (u32 BaudRate,u16 XmtLineSize, u16 NumberXmtLines);
/* @brief	: Initializes the USARTx to 8N1 and the baudrate for interrupting 
 * @param	: Baudrate 		- baudrate 			(e.g. 115200)
 * @param	: u16 XmtLineSize	- xmit line size, 		(e.g. 80)
 * @param	: u16 NumberXmtLines	- number of xmit line buffers, 	(e.g. 4)
 * @return	: 0 = success; 1 = fail malloc; 2 = XmtLineSize zero; 3 = NumberXmtLines < 2; 
 *******************************************************************************/
char USART1_txdma_putc(char c);
char USART2_txdma_putc(char c);
char USART3_txdma_putc(char c);
/* @brief	: Put char.  Add a char to output buffer
* @param	: Char to be sent
* @return	: == 0 for buffer did not overflow; == 1 overflow, chars lost
*******************************************************************************/
void USART1_txdma_puts(char* p);
void USART2_txdma_puts(char* p);
void USART3_txdma_puts(char* p);
/* @brief	: Add a zero terminated string to the output buffer.
* @param	: Pointer to zero terminated string
* @return	: == 0 for buffer did not overflow; != pointer p for next char not stored
*******************************************************************************/
char* USART1_txdma_puts_waitbusy(char* p);
char* USART2_txdma_puts_waitbusy(char* p);
char* USART3_txdma_puts_waitbusy(char* p);
/* @brief	: Wait if busy, then add string
* @return	: 0 = OK; otherwise,buffer full and 'p' pointing to char not stored
*******************************************************************************/
u16 USART1_txdma_busy(void);
u16 USART2_txdma_busy(void);
u16 USART3_txdma_busy(void);
/* @brief	: Check that line buffer is available
* @return	: 1 = all line buffers filled; 0 = free line buffer(s)
*******************************************************************************/
void USART1_txdma_send(void);
void USART2_txdma_send(void);
void USART3_txdma_send(void);
/* @brief	: Step to next line tx line buffer; if DMA not sending, start it now.
* @return	: none
*******************************************************************************/
char USART1_txdma_putc(char c);
char USART2_txdma_putc(char c);
char USART3_txdma_putc(char c);
/* @brief	: Put char.  Add a char to output buffer
* @param	: Char to be sent
* @return	: 0 = buffer did not overflow; 1 = overflow, chars could be lost
*******************************************************************************/


/* ######################################################################################## */
/* ########################## rxdma ####################################################### */
/* ######################################################################################## */
/*******************************************************************************/
u16 USART1_rxdma_init (u32 BaudRate,u16 RcvCircularSize, u16 GetLineSize);
u16 USART2_rxdma_init (u32 BaudRate,u16 RcvCircularSize, u16 GetLineSize);
u16 USART3_rxdma_init (u32 BaudRate,u16 RcvCircularSize, u16 GetLineSize);
/* @brief	: Initializes the USART2 to 8N1 and the baudrate for DMA circular
 *                buffering.
 * @param	: u32 BaudRate		- baud rate.				(e.g. 115200)
 * @param	: u16 RcvCircularSize 	- size of receive circular buffer 	(e.g. 128)
 * @param	: u16 GetLineSize 	- size of 'getline' buffer 		(e.g. 80)
 * @return	: 0 = success; 1 = fail malloc; 2 = RcvCircularSize = 0; 3 = GetLineSize = 0; 
 *******************************************************************************/
char USART1_rxdma_getchar(void);
char USART2_rxdma_getchar(void);
char USART3_rxdma_getchar(void);
/* @brief	: Get one char from the input (rx) circular buffer.
* @return	: a nice round char
*******************************************************************************/
u16 USART1_rxdma_getcount(void);
u16 USART2_rxdma_getcount(void);
u16 USART3_rxdma_getcount(void);
/* @brief	: Get the number of chars in the input/read buffer.
* @return	: number of chars in currently buffered.
*******************************************************************************/
char* USART1_rxdma_getline(void);
char* USART2_rxdma_getline(void);
char* USART3_rxdma_getline(void);
/* @brief	: Assemble one 0x0d (END_OF_LINE) zero terminated line from the input/read buffer.
* @return	: 0 == line not complete, != 0 is ptr to completed line buffer
*******************************************************************************/
/* ######################################################################################## */
/* ########################## rxint ####################################################### */
/* ######################################################################################## */
/*******************************************************************************/
u16 USART1_rxint_init (u32 BaudRate,u16 RcvLineBufferSize, u16 RcvNumberLineBuffers);
u16 USART2_rxint_init (u32 BaudRate,u16 RcvLineBufferSize, u16 RcvNumberLineBuffers);
u16 USART3_rxint_init (u32 BaudRate,u16 RcvLineBufferSize, u16 RcvNumberLineBuffers);
/* @brief	: Initializes the USART2 to 8N1 and the baudrate for DMA circular buffering.
 * @param	: u32 BaudRate		- baud rate.				(e.g. 115200)
 * @param	: u16 RcvLineBufferSize 	- size of one line buffer 	(e.g. 48)
 * @param	: u16 RcvNumberLineBuffers	- number of line buffers 	(e.g. 4)
 * @return	: 0 = success; 1 = fail malloc; 2 = RcvLineBufferSize = 0; 3 = RcvNumberLineBuffers = 0; 
 *******************************************************************************/
u16 USART1_rxint_getcount(void);
u16 USART2_rxint_getcount(void);
u16 USART3_rxint_getcount(void);
/* @brief	: Get the number of chars in the input/read buffer.
* @return	: number of chars in currently buffered.
*******************************************************************************/
char* USART1_rxint_getline(void);
char* USART2_rxint_getline(void);
char* USART3_rxint_getline(void);
/* @brief	: Get pointer to one 0x0d (END_OF_LINE) zero terminated line buffer
* @return	: 0 == line not complete, != 0 is ptr to completed line buffer
*******************************************************************************/
struct USARTLB USART1_rxint_getlineboth(void);
struct USARTLB USART2_rxint_getlineboth(void);
struct USARTLB USART3_rxint_getlineboth(void);
struct USARTLB  UART4_rxint_getlineboth(void);
struct USARTLB  UART5_rxint_getlineboth(void);
/* @brief	: Get pointer to one 0x0d (END_OF_LINE) zero terminated line buffer
* @return	: 0 == line not complete, != 0 is ptr to completed line buffer & char count
*******************************************************************************/
/* ######################################################################################## */
/* ########################## txint ####################################################### */
/* ######################################################################################## */
/*******************************************************************************/
void USART1_txint_send(void);
void USART2_txint_send(void);
void USART3_txint_send(void);
void  UART4_txint_send(void);
void  UART5_txint_send(void);
/* @brief	: Step to next line tx line buffer; if DMA not sending, start it now.
* @return	: none
*******************************************************************************/
u16 USART1_txint_busy(void);
u16 USART2_txint_busy(void);
u16 USART3_txint_busy(void);
u16  UART4_txint_busy(void);
u16  UART5_txint_busy(void);
/* @brief	: Check that line buffer is available
* @return	: 1 = all line buffers filled; 0 = free line buffer(s)
*******************************************************************************/
void USART1_txint_puts(char* p);
void USART2_txint_puts(char* p);
void USART3_txint_puts(char* p);
void  UART4_txint_puts(char* p);
void  UART5_txint_puts(char* p);
/* @brief	: p: Pointer to zero terminated string
* @return	: 1 = all line buffers filled; 0 = free line buffer(s)
*******************************************************************************/
char USART1_txint_putc(char c);
char USART2_txint_putc(char c);
char USART3_txint_putc(char c);
char  UART4_txint_putc(char c);
char  UART5_txint_putc(char c);
/* @brief	: Put char.  Add a char to output buffer
* @param	: Char to be sent
* @return	: 0 = buffer did not overflow; 1 = overflow, chars could be lost
*******************************************************************************/
/* ######################################################################################## */
/* ########################## txcir ####################################################### */
/* ######################################################################################## */
/*******************************************************************************/
void USART1_txcir_puts(char* p);
void USART2_txcir_puts(char* p);
void USART3_txcir_puts(char* p);
/* @brief	: Add a zero terminated string to the output buffer.
* @param	: Pointer to zero terminated string
* @return	: none
*******************************************************************************/
void USART1_txcir_putc(char c);
void USART2_txcir_putc(char c);
void USART3_txcir_putc(char c);
/* @brief	: Put char.  Add a char to output buffer
* @param	: Char to be sent
* @return	: none
*******************************************************************************/
u16 USART1_txcir_putcount(void);
u16 USART2_txcir_putcount(void);
u16 USART3_txcir_putcount(void);
/* @brief	: Get number of chars available in circular buffer
* @param	: Pointer to zero terminated string
* @return	: 0 = no chars available, + = count of chars free
*******************************************************************************/



/* ######################################################################################## */
/* ########################## rxinttxcir ################################################## */
/* ######################################################################################## */
/*******************************************************************************/
u16 USART1_rxinttxcir_init (u32 BaudRate,u16 RcvLineSize, u16 NumberRcvLines, u16 XmtCircularSize);
u16 USART2_rxinttxcir_init (u32 BaudRate,u16 RcvLineSize, u16 NumberRcvLines, u16 XmtCircularSize);
u16 USART3_rxinttxcir_init (u32 BaudRate,u16 RcvLineSize, u16 NumberRcvLines, u16 XmtCircularSize);
/* @brief	: Initializes the USARTx to 8N1 and the baudrate for interrupting receive into line buffers
 * @param	: u32 Baudrate		- baudrate 				(e.g. 115200)
 * @param	: u16 RcvLineSize	- size of each line buffer 		(e.g. 32)
 * @param	: u16 NumberRcvLines	- number of receive line buffers	(e.g. 3)
 * @param	: u16 XmtCircularSize	- size of xmt circular buffer 		(e.g. 128)
 * @return	: 0 = success; 1 = fail malloc; 2 = RcvLineSize zero; 3 = NumberRcvLines = 0; 4 = XmitCircularSize = 0 
 *******************************************************************************/
/* ######################################################################################## */
/* ########################## rxinttxint ################################################## */
/* ######################################################################################## */
/*******************************************************************************/
u16 USART1_rxinttxint_init (u32 BaudRate,u16 RcvLineSize, u16 NumberRcvLines, u16 XmtLineSize, u16 NumberXmtLines);
u16 USART2_rxinttxint_init (u32 BaudRate,u16 RcvLineSize, u16 NumberRcvLines, u16 XmtLineSize, u16 NumberXmtLines);
u16 USART3_rxinttxint_init (u32 BaudRate,u16 RcvLineSize, u16 NumberRcvLines, u16 XmtLineSize, u16 NumberXmtLines);
u16  UART4_rxinttxint_init (u32 BaudRate,u16 RcvLineSize, u16 NumberRcvLines, u16 XmtLineSize, u16 NumberXmtLines);
u16  UART5_rxinttxint_init (u32 BaudRate,u16 RcvLineSize, u16 NumberRcvLines, u16 XmtLineSize, u16 NumberXmtLines);
u16 USART1_rxinttxint_initRTC (u32 BaudRate,u16 RcvLineSize, u16 NumberRcvLines, u16 XmtLineSize, u16 NumberXmtLines);
u16 USART1_rxinttxint_initRTC_p1 (u32 BaudRate,u16 RcvLineSize, u16 NumberRcvLines, u16 XmtLineSize, u16 NumberXmtLines);
u16 USART2_rxinttxint_initRTC (u32 BaudRate,u16 RcvLineSize, u16 NumberRcvLines, u16 XmtLineSize, u16 NumberXmtLines);
u16 USART3_rxinttxint_initRTC (u32 BaudRate,u16 RcvLineSize, u16 NumberRcvLines, u16 XmtLineSize, u16 NumberXmtLines);
u16  UART4_rxinttxint_initRTC (u32 BaudRate,u16 RcvLineSize, u16 NumberRcvLines, u16 XmtLineSize, u16 NumberXmtLines);
u16  UART5_rxinttxint_initRTC (u32 BaudRate,u16 RcvLineSize, u16 NumberRcvLines, u16 XmtLineSize, u16 NumberXmtLines);
/* @brief	: Initializes the USARTx, UARTx, to 8N1 and the baudrate for interrupting receive into line buffers
 * @brief	: 'RTC is a special version that appends the RTC system counter after the EOL & zero terminator bytes
 * @param	: u32 Baudrate		- baudrate 				(e.g. 115200)
 * @param	: u16 RcvLineSize	- size of each line buffer 		(e.g. 32)
 * @param	: u16 NumberRcvLines	- number of receive line buffers	(e.g. 3)
 * @param	: u16 XmtLineSize	- size of each line buffer 		(e.g. 80)
 * @param	: u16 NumberXmtLines	- number of xmt line buffers		(e.g. 2)
 * @return	: 0 = success; 1 = fail malloc; 2 = RcvLineSize zero; 3 = NumberRcvLines = 0; 
 *******************************************************************************/

/* ######################################################################################## */
/* ########################## rxcirtxcir ################################################## */
/* ######################################################################################## */
/*******************************************************************************/
u16 USART1_rxcirtxcir_init (u32 BaudRate,u16 RcvCircularSize, u16 XmtCircularSize);
/* @brief	: Initializes the USARTx to 8N1 and the baudrate for interrupting receive into line buffers
 * @param	: u32 Baudrate		- baudrate 				(e.g. 115200)
 * @param	: u16 RcvLineSize	- size of each line buffer 		(e.g. 32)
 * @param	: u16 XmtCircularSize	- size of xmt circular buffer 		(e.g. 128)
 * @return	: 0 = success; 1 = fail malloc; 2 = RcvLineSize zero;  4 = XmitCircularSize = 0 
 *******************************************************************************/

/*******************************************************************************/
u16 USARTremap (u32 usartx, u8 u8Code);
/* @brief	: Remaps the USART (see p 166 Ref Manual)
 * @param	: u32 USART base address, e.g. 'USART1' (see libopenstm32/usart.h)
 * @param	: u8  Code for remapping: USART1,2: 0 or 1 USART3: 0,1,or 3; UART4,5 not remapped
 * @return	: 0 == OK, not 0: 1 = bad USART3 code, 2 = UART4,5 don't remap, 3 = bad USART1,2 remap, 4 = bad base address
 *******************************************************************************/

/*******************************************************************************/
void USART2_setbaud(u32 Baudrate);
/* @brief	: Set baud rate
*******************************************************************************/

/* struct for returning a pointer to a line buffer along with the char count stored */
struct USARTLB
{
	u16	ct;	// Number of chars stored in the buffer (not counting zero terminator)
	char*	p;	// Pointer to beginning of line buffer
};

/* These are used with the 'min' version routines */
extern volatile char USART1_rcvchar;	// Char received
extern volatile char USART2_rcvchar;	// Char received
extern volatile char USART3_rcvchar;	// Char received
extern volatile char USART1_rcvflag;	// Char ready flag
extern volatile char USART2_rcvflag;	// Char ready flag
extern volatile char USART3_rcvflag;	// Char ready flag

#endif 


