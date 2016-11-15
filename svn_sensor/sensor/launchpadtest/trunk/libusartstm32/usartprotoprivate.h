/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : usartprotoprivate.h
* Hackor             : deh
* Date First Issued  : 10/04/2010
* Description        : usart subroutine prototypes not for public comsumption
*******************************************************************************/
/*
07-14-2011 - Update to add UART4,5 and UART4 RTC
*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USARTALLPROTOPRIVATE_H
#define __USARTALLPROTOPRIVATE_H

#include "../libusartstm32/usartall.h"
#include "../libusartstm32/usartallproto.h"
/* ################ common to all ########################################### */

/*****************************************************************************/
void usartx_setbaud (u32 usartx, u32 u32BaudRate);
/* @brief	: set baudrate register (BRR)
 * @param	: u32BaudRate is the baud rate.
******************************************************************************/
char* usartx_putsS( char* p, char** pnow, char* pend);
/* @brief	: Fast inner loop assembly routine for USART1_puts,USART2_puts,USART3_puts
* @param	: Pointer to zero terminated string
* @return	: == 0 for buffer did not overflow; != pointer p for next char not stored
*******************************************************************************/

/* ############### dma and int line buffer ################################# */
/*****************************************************************************/
void usartx_txmain_advptr (struct USARTCBT* pUSARTcbtx);
/* @brief	: Advance line buffer pointers for tx main
 * @return	: none
******************************************************************************/
void usartx_rxisr_advptr (struct USARTCBT* pUSARTcbtx);
/* @brief	: Advance line buffer pointers for tx isr routine
 * @return	: none
******************************************************************************/

/* ############### tx cir ###################################################### */
/*******************************************************************************/
void usartx_txcir_putsS(char* p, char** pnow,  char* pbegin, char* pend);
/* @brief	: Add a zero terminated string to the output buffer.
* @param	: Pointer to zero terminated string
* @param	: Address of pointer to where chars are being stored (now)
* @param	: Pointer to beginning if circular buffer
* @param	: Pointer to end of circular buffer
* @return	: none
*******************************************************************************/
void usartx_txcir_putc(char c, struct USARTCBT* pUSARTcbtx);
/* @brief	: Put char.  Add a char to output buffer
* @param	: Char to be sent
* @param	: Pointer to struct control block for transmit
* @return	: none
*******************************************************************************/
void usartx_txcir_puts(char* p, struct USARTCBT* pUSARTcbtx);
/* @brief	: Add a zero terminated string to the output buffer, common routine
* @param	: p 		Pointer to zero terminated string
* @param	: pUSARTcbx	Pointer to struct--pUSARTcb1,2,3
* @return	: none
*******************************************************************************/
u16 usartx_txcir_allocatebuffers (u16 xmtcircularsize, struct USARTCBT** pUSARTcbtx);
/* @brief	: Allocate buffers with 'mymalloc' for receive and setup control block pointers
 * @param	: xmtcircularsize is size of circular buffer (e.g. 128)
 * @param	: pUSARTcbtx points to pointer in static memory that points to control block
 * @return	: 0 = success; 1 = fail malloc; 2 = rcvcircularsize zero; 
******************************************************************************/
void usartx_txcir_usart_init (u32 USARTx, u32 BaudRate);
/* @brief	: Setup USART baudrate, Tx for dma
******************************************************************************/
u16 usartx_txcir_putcount(u32* usartx_txflag_bitband, struct USARTCBT* pUSARTcbtx);
/* @brief	: Get number of chars available in circular buffer
* @param	: u32* usartx_txflag_bitband: 	Bit-band address for usart tx flag
* @param	: pUSARTcbx:	Pointer to struct--pUSARTcb1,2,3
* @return	: 0 = no chars available, + = count of chars free
*******************************************************************************/
/* ############### tx int ###################################################### */
/******************************************************************************/
u16 usartx_txint_allocatebuffers (u16 xmtlinesize,u16 numberxmtlines, struct USARTCBT** pUSARTcbtx);
/* @brief	: Allocate buffers with 'mymalloc' for receive and setup control block pointers
 * @param	: xmtlinesize is size of each line buffer (e.g. 48)
 * @param	: numberxmtlines is the number of receive line buffers (e.g. 4)
 * @param	: pUSARTcbtx points to pointer in static memory that points to control block
 * @return	: 0 = success; 1 = fail malloc; 2 = rcvlinesize zero; 3 = numberrcvlines = 0; 
******************************************************************************/
void usartx_txint_usart_init (u32 USARTx, u32 BaudRate);
/* @brief	: Setup USART baudrate, Tx for dma
******************************************************************************/
void usartx_txint_isrptradv(struct USARTCBT* pUSARTcbtx);
/* @brief	: Advance usart control block pointers for txint isr routine
 * @param	: pUSARTcbtx	Pointer to struct--pUSARTcbt1,2,3
 * @return	: none
******************************************************************************/
void usartx_txint_puts(char* p, struct USARTCBT* pUSARTcbtx);
/* @brief	: Add a zero terminated string to the output buffer, common routine
* @param	: p 		Pointer to zero terminated string
* @param	: pUSARTcbx	Pointer to struct--pUSARTcb1,2,3
* @return	: none
*******************************************************************************/
char usartx_txint_send(struct USARTCBT* pUSARTcbtx);
/* @brief	: Step to next line tx line buffer; if USART not sending, start it now.
* @param	: pUSARTcbtx	Pointer to struct--pUSARTcbid1,2,3
* @return	: 0 = OK; 1 = NG
*******************************************************************************/
/* ############### tx dma ###################################################### */
/******************************************************************************/
char usartx_txdma_putc(char c, struct USARTCBT* pUSARTcbtx);
/* @brief	: Put char.  Add a char to output buffer
* @param	: Char to be sent
* @return	: == 0 for buffer did not overflow; == 1 overflow, chars lost
*******************************************************************************/
u16 usartx_txdma_allocatebuffers (u16 xmtlinesize,u16 numberxmtlines, struct USARTCBT** pUSARTcbtx);
/* @brief	: Allocate buffers with 'mymalloc' for transmit and setup control block pointers
 * @param	: xmtlinesize is size of each line buffer (e.g. 48)
 * @param	: numberxmtlines is the number of receive line buffers (e.g. 4)
 * @param	: pUSARTcbtx points to pointer in static memory that points to control block
 * @return	: 0 = success;1 = fail mymallocl; 2 = zero xmtlinesize zero; 3 = numberxmtlinesize less than 2
******************************************************************************/
 void usartx_txdma_usart_init (u32 USARTx, u32 BaudRate);
/* @brief	: Setup USART baudrate, Tx for dma
 * @return	: none
******************************************************************************/
 void usartx_txmain_advlnptr (struct USARTCBT* pUSARTcbtx);
/* @brief	: Advance line buffer pointers for main
******************************************************************************/
void usartx_txisr_advlnptr (struct USARTCBT* pUSARTcbtx);
/* @brief	: Advance line buffer pointers for isr
******************************************************************************/
/* ############### rx dma ###################################################### */
/******************************************************************************/
u16 usartx_rxdma_allocatebuffers (u16 rcvcircularsize, u16 getlinesize, struct USARTCBR** pUSARTcbrx);
/* @brief	: Allocate buffers with 'mymalloc' for transmit and setup control block pointers
 * @param	: getlinesize is size of each line buffer (e.g. 48)
 * @param	: numberrcvlines is the number of receive line buffers (e.g. 4)
 * @param	: pUSARTcbrx points to pointer in static memory that points to control block
 * @return	: 0 = success;1 = fail mymallocl; 2 = zero getlinesize zero; 3 = numbergetlinesize zero
******************************************************************************/
void usartx_rxdma_usart_init (u32 USARTx, u32 BaudRate);
/* @brief	: Setup USART baudrate, rx for dma
 * @return	: none
******************************************************************************/
/* ############### rx int ###################################################### */
/******************************************************************************/
u16 usartx_rxint_allocatebuffers (u16 rcvlinesize, u16 numberrcvlines, struct USARTCBR** pUSARTcbrx);
/* @brief	: Allocate buffers with 'mymalloc' for receive and setup control block pointers
 * @param	: rcvlinesize is size of each line buffer (e.g. 48)
 * @param	: numberrcvlines is the number of receive line buffers (e.g. 4)
 * @param	: pUSARTcbrx points to pointer in static memory that points to control block
 * @return	: 0 = success; 1 = fail malloc; 2 = rcvlinesize zero; 3 = numberrcvlines = 0; 
******************************************************************************/
char* usartx_rxint_rxmainadvptr (struct USARTCBR* pUSARTcbrx);
/* @brief	: Check for EOL, advance pointers for rx isr routine
******************************************************************************/
void usartx_rxint_rxisrptradv (struct USARTCBR* pUSARTcbrx);
/* @brief	: Check for EOL, advance pointers for rx isr routine
******************************************************************************/
void usartx_rxint_rxisrptradvRTC (struct USARTCBR* pUSARTcbrx);
/* @brief	: Check for EOL, advance pointers for rx isr routine; Add RTC at end of line
******************************************************************************/
void usartx_rxint_rxisrptradvRTC_p1 (struct USARTCBR* pUSARTcbrx);
/* @brief	: Check for EOL, advance pointers for rx isr routine; Add RTC at end of line for pod_v1
******************************************************************************/
void usartx_rxint_usart_init (u32 USARTx, u32 BaudRate);
/* @brief	: Setup USART baudrate, Rx interrupting with line buffers
 * @param	: USARTx = USART1, USART2, or USART3 base address
 * @param	: u32 BaudRate is the baud rate.
******************************************************************************/
char* usartx_rxint_getline(struct USARTCBR* pUSARTcbrx);
/* @brief	: Get pointer to one 0x0d (END_OF_LINE) zero terminated line buffer
* @return	: 0 == line not complete, != 0 is ptr to completed line buffer
*******************************************************************************/

/* ########################## txmin ########################################### */
/******************************************************************************/
void usartx_rxmin_init (u32 USARTx, u32 BaudRate);
/* @brief	: Setup USART baudrate, Rx interrupting with line buffers
 * @param	: USARTx = USART1, USART2, or USART3 base address
* @param	: u32 Baudrate		- baudrate 		(e.g. 115200)
******************************************************************************/
void usartx_txmin_init (u32 USARTx, u32 BaudRate);
/* @brief	: Setup USART baudrate, Tx for dma
 * @param	: USARTx = USART1, USART2, or USART3 base address
 * @param	: u32 Baudrate		- baudrate 		(e.g. 115200)
******************************************************************************/
u16 usartx_txdma_allocatebuffers (u16 xmtlinesize,u16 numberxmtlines, struct USARTCBT** pUSARTcbtx);
/* @brief	: Allocate buffers with 'mymalloc' for transmit and setup control block pointers
 * @param	: xmtlinesize is size of each line buffer (e.g. 48)
 * @param	: numberxmtlines is the number of receive line buffers (e.g. 4)
 * @param	: pUSARTcbtx points to pointer in static memory that points to control block
 * @return	: 0 = success;1 = fail mymallocl; 2 = zero xmtlinesize zero; 3 = numberxmtlinesize zero
******************************************************************************/
void usartx_txdma_send(struct USARTCBT* pUSARTcbtx);
/* @brief	: Step to next line tx line buffer; if DMA not sending, start it now.
* @param	: pUSARTcbtx	Pointer to struct--pUSARTcb1,2,3
* @return	: none
*******************************************************************************/

/* ########################## rxcir ########################################### */
/*******************************************************************************/
u16 usartx_rxcir_ready(struct USARTCBR* USARTcbrx);
/* @brief	: Check number of chars in rx circular buffer
* @return	: Number of chars
*******************************************************************************/



/* ########################################################################## */

/* Control block pointers */
extern struct USARTCBT* pUSARTcbt1;
extern struct USARTCBT* pUSARTcbt2;
extern struct USARTCBT* pUSARTcbt3;
extern struct USARTCBT* pUSARTcbt4;
extern struct USARTCBT* pUSARTcbt5;

extern struct USARTCBR* pUSARTcbr1;
extern struct USARTCBR* pUSARTcbr2;
extern struct USARTCBR* pUSARTcbr3;
extern struct USARTCBR* pUSARTcbr4;
extern struct USARTCBR* pUSARTcbr5;

#endif 

