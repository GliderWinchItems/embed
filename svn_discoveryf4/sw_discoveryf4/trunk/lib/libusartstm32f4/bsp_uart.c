/******************************************************************************
* File Name          : bsp_uart.c
* Date First Issued  : 11/06/2013
* Board              : Discovery F4 (F405 or F407)
* Description        : Board Specific Support for uart routines
*******************************************************************************/
/* 
05-27-2015 rev-309 Added nstop (number of stop bits) and block (block/not-block when
     buffer full) to calls 
*/
#include <string.h>
#include <stdio.h>
#include <malloc.h>

#include "libopencm3/stm32/memorymap.h"
#include "libopencm3/cm3/common.h"
#include "libopencm3/stm32/usart.h"
#include "libopencm3/stm32/nvic.h"
//#include "libopencm3/stm32/nvic_f4.h"
#include "libopencm3/stm32/f4/usart.h"
#include "libopencm3/stm32/f4/dma_common_f24.h"
#include "libopencm3/stm32/f4/gpio.h"
#include "libopencm3/stm32/f4/rcc.h"
#include "libusartstm32f4/nvicdirect.h"
#include "panic_leds.h"
#include "nvic_dma_mgr.h"
#include "DISCpinconfig.h"
#include "usartx_setbaud.h"
#include "uart_pins.h"
#include "bsp_uart.h"


/* Static routines */
static void common_dma(struct CB_UART* pctl);

/* Error trapping */
int PANIC_x;	// Global variable to look at.
void bsp_panic (int x)
{ PANIC_x = x; panic_leds(7); } // Save code, and flash LEDs 7x

//urt2 = bsp_uart_dma(USART2, 921600, 256, 256, 7, 2); // UARTx, baud, tx buff size, rx buff size, tx dma stream, rx dma stream -- FTDI

#define FDMAX 8	// Number of file descriptors allowed
struct CB_UART cb_uart[8];	// Control blocks for each possible UART
struct CB_UART* cb_map[FDMAX];	// Pointers to control block versus array index (USART1 = 0,...,USART6 = 5)

/* Table to check that stream selection is permitted for this UART and determine channel number. */
// DMA v RX stream for channels 4, 5, 7
static const volatile u32 dma1_rxstreamtbl_4[8] = {UART5,  USART3, UART4,      0,      0, USART2,      0,      0};
static const volatile u32 dma1_rxstreamtbl_5[8] = {    0,       0,     0,      0,      0,      0,      0,      0}; // *** stm32f42x and f43x ***

static const volatile u32 dma2_rxstreamtbl_4[8] = {     0,      0, USART1,     0,      0, USART1,      0,      0};
static const volatile u32 dma2_rxstreamtbl_5[8] = {     0, USART6, USART6,     0,      0,      0,      0,      0};

// DMA v TX stream for channels 4, 5, 7
static const volatile u32 dma1_txstreamtbl_4[8] = {    0,       0,     0, USART3,  UART4,      0, USART2,  UART5};
static const volatile u32 dma1_txstreamtbl_5[8] = {    0,       0,     0,      0,      0,      0, USART6, USART6}; // *** stm32f42x and f43x ***
static const volatile u32 dma1_txstreamtbl_7[8] = {    0,       0,     0,      0, USART3,      0,      0,      0};

static const volatile u32 dma2_txstreamtbl_4[8] = {     0,      0,      0,     0,      0,      0,      0, USART1};
static const volatile u32 dma2_txstreamtbl_5[8] = {     0,      0,      0,     0,      0,      0, USART6, USART6};

/*******************************************************************************
 * static u8* rxbuff_adv (struct CB_UART* pctl, u8* p);
 * @brief	: Advance rx buffer pointer, circularly
 * @param	: pctl = pointer to control block
 * @param	: p = pointer to be advanced
 * @return	: new pointer 'p'
*******************************************************************************/
static u8* rxbuff_adv (struct CB_UART* pctl, u8* p)
{
	p += 1; if (p >= pctl->rxbuff_end) p = pctl->rxbuff_base;
	return p;
}
/*******************************************************************************
 * static u8* txbuff_adv (struct CB_UART* pctl, char* p);
 * @brief	: Advance tx buffer pointer, circularly
 * @param	: pctl = pointer to control block
 * @param	: p = pointer to be advanced
 * @return	: new pointer 'p'
*******************************************************************************/
static u8* txbuff_adv (struct CB_UART* pctl, u8* p)
{
	p += 1; if (p >= pctl->txbuff_end) p = pctl->txbuff_base;
	return p;
}
/******************************************************************************
 * static int getbuff(int idx, u32 rxbuffsize, u32 txbuffsize);
 * @brief	: Obtain some buffer space and initialize buffer pointers.
 * @param	: idx = index into array of control blocks
 * @param	: rxbuffsize = requested size of rx buffer
 * @param	: txbuffsize = requested size of rx buffer
 * @return	: 0 = OK; -1; fail = led trap
******************************************************************************/	
static int getbuff(int idx, u32 rxbuffsize, u32 txbuffsize)
{
	/* Obtain some buffer space and initialize buffer pointers. */
	if ((cb_uart[idx].rxbuff_base = malloc(rxbuffsize)) == NULL) bsp_panic(-13);	// malloc failed to obtain rx buffer
	if ((cb_uart[idx].txbuff_base = malloc(txbuffsize)) == NULL) bsp_panic(-14);	// malloc failed to obtain tx buffer
	cb_uart[idx].rxbuff_end  = cb_uart[idx].rxbuff_base + rxbuffsize;
	cb_uart[idx].txbuff_end  = cb_uart[idx].txbuff_base + txbuffsize;
	cb_uart[idx].rxbuff_size = rxbuffsize;
	cb_uart[idx].txbuff_size = txbuffsize;
	cb_uart[idx].rxbuff_out  = cb_uart[idx].rxbuff_base;
	cb_uart[idx].txbuff_out  = cb_uart[idx].txbuff_base;
	cb_uart[idx].txbuff_dmanext = cb_uart[idx].txbuff_base;
	cb_uart[idx].rxbuff_in   = cb_uart[idx].rxbuff_base;
	cb_uart[idx].txbuff_in   = cb_uart[idx].txbuff_base;
	
	return 0;
}
/******************************************************************************
 * static int mapindex(volatile u32 iuart);
 * @brief	: Edit & Map register base address to control block index & enable APBxENR
 * @param	: p = pointer to register base
 * @return	: 0 - 7 = OK; panic_leds for fail
******************************************************************************/	
static int mapindex(volatile u32 iuart)
{
	int idx=60;

	/* Index v register base */
	switch (iuart)
	{
	case USART1:	idx =  0; break;
	case USART2:	idx =  1; break;
	case USART3:	idx =  2; break;
	case UART4:	idx =  3; break;
	case UART5:	idx =  4; break;
	case USART6:	idx =  5; break;

	default:
		bsp_panic(-61);	// No match for request
	}
	/* Enable USART/UART module on appropriate bus */
	switch (idx)
	{
	case 0:	RCC_AHB2ENR |= (1 << 4); break;
	case 5:	RCC_AHB2ENR |= (1 << 5); break;	

	case 1:	RCC_AHB1ENR |= (1 << 17); break;
	case 2:	RCC_AHB1ENR |= (1 << 18); break;
	case 3:	RCC_AHB1ENR |= (1 << 19); break;
	case 4:	RCC_AHB1ENR |= (1 << 20); break;

	default:
		bsp_panic(-62);	// Wrong
	}

	return idx;
}
/******************************************************************************
 * static int irq_given_datastream(u32 dmabase, int dmastream);
 * @brief	: Map datastream to IRQ number
 * @param	: dmabase = address of DMA1 or DMA2
 * @param	: dmastream (0 - 7)
 * @return	: irq number (various)
******************************************************************************/	
struct IRQNUM	// Pass back two parameters
{
	u32	dma;
	int	num;
};
static struct IRQNUM irq_given_datastream(u32 iuart, int dmastream)
{
	struct IRQNUM irqnum = {0,0}; 
	if (dmastream < 0) bsp_panic(-714);

	if ((iuart == USART1) || (iuart == USART6))
	{
		irqnum.dma = DMA2_BASE;
		if (dmastream > 4)	// Determine IRQ number for the stream 
		{ irqnum.num = (DMASTRM25 + dmastream - 5); return irqnum;}
		else
		{ irqnum.num = (DMASTRM20 + dmastream); return irqnum;}
	}

	irqnum.dma = DMA1_BASE;
	if (dmastream == 7)
		{ irqnum.num = (DMASTRM17); return irqnum;}
	else
		{ irqnum.num = (DMASTRM10 + dmastream); return irqnum;}

	bsp_panic(-715);	// Shouldn't happen
	return irqnum;
}

/******************************************************************************
 * struct CB_UART* bsp_uart_open(const char *name);
 * @brief	: Translate name string to file descriptor for 'open'
 * @return	: control block pointer
******************************************************************************/	
static const char* open_name[8] = {"tty1", "tty2", "tty3", "tty4", "tty5", "tty6", "tty7", "tty8"};

struct CB_UART* bsp_uart_open(const char *name)
{
	int i;
	for (i = 0; i < 8; i++)
	{
		if (strcmp(name, open_name[i]) == 0) break;
	}
	if (i >= 8) return (struct CB_UART*)-1;
	
	return &cb_uart[i];	// Return control block ptr to 'open'
}
/*******************************************************************************
 * static u32 uartnumber_to_baseaddr(u32 uartnumber);
 * @brief	: Map uart number (1-6) to uart module base address
 * @param	: uartnumber = (1 -6), e.g. 2 = USART2
 * @return	: USART/UART base address, e.g. USART2 = 0x40004400
*******************************************************************************/
static u32 uartnumber_to_baseaddr(u32 uartnumber)
{
	u32 iuart = 0;
	/* Convert uart number to uart register base address */
	switch (uartnumber)
	{
	case 1:	iuart = USART1;	break;
	case 2:	iuart = USART2;	break;
	case 3:	iuart = USART3;	break;
	case 4:	iuart =  UART4;	break;
	case 5:	iuart =  UART5;	break;
	case 6:	iuart = USART6;	break;

	default:
		bsp_panic(-66);	// No match for request
	}	
	return iuart;

}
/*******************************************************************************
 * int bsp_uart_dma_init_number(u32 uartnumber, u32 baud,u32 rxbuffsize, u32 txbuffsize, u32 dmastreamrx, u32 dmastreamtx, u32 dma_tx_int_priority, u8 block, u8 nstop);
 * @brief	: A wrapper for 'int bsp_uart_dma_init' with uart number (1-6) rather than uart base address as the first argument
 * @return	: Same as bsp_uart_dma_init
*******************************************************************************/
int bsp_uart_dma_init_number(u32 uartnumber, u32 baud,u32 rxbuffsize, u32 txbuffsize, u32 dmastreamrx, u32 dmastreamtx, u32 dma_tx_int_priority, u8 block, u8 nstop)
{
	u32 iuart = uartnumber_to_baseaddr(uartnumber);
	return  bsp_uart_dma_init(iuart, baud, rxbuffsize, txbuffsize, dmastreamrx, dmastreamtx, dma_tx_int_priority, block, nstop);
}
/*******************************************************************************
 * int bsp_uart_dma_init(u32 iuart, u32 baud, u32 rxbuffsize, u32 txbuffsize, u32 dmastreamrx, u32 dmastreamtx, u32 dma_tx_int_priority, u8 block, u8 nstop);
 * @brief	: Initialize USART/UART for DMA transfers
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
static u32 qqq;
int bsp_uart_dma_init(u32 iuart, u32 baud, u32 rxbuffsize, u32 txbuffsize, u32 dmastreamrx, u32 dmastreamtx, u32 dma_tx_int_priority, u8 block, u8 nstop)
{
	u32 cb_Idx;	// Used "everywhere" to index into the control block array

	int tmp;
	struct UARTPINS uartpins;

	struct IRQNUM dma_irq_number_tx;
	struct IRQNUM dma_irq_number_rx;
	u32 dma_x;

	u32 dma_channel_number_rx = 4;	// Initialize to prevent compiler warning
	u32 dma_channel_number_tx = 5;	


	/* Be sure arguments passed are within range */
	if (dmastreamrx > 7) bsp_panic(-1);
	if (dmastreamtx > 7) bsp_panic(-2);
	if ((dma_tx_int_priority & 0xffffff0f) != 0) bsp_panic(-3); // Bogus priority
	
	cb_Idx = mapindex(iuart);	// Map usart/uart register base to control block index

	/* Convert dma stream to dma base and irq number */
	dma_irq_number_tx = irq_given_datastream(iuart, dmastreamtx); // TX
	dma_irq_number_rx = irq_given_datastream(iuart, dmastreamrx); // RX
	
	/* The DMA determined should be the same...mostly a debugging issue. */
	if (dma_irq_number_tx.dma != dma_irq_number_rx.dma) bsp_panic(-333);
	dma_x = dma_irq_number_tx.dma;	// Lazy way of dealing with it later

	/* Set dma stream interrupt to revector to this routine; check if dma is in use. */
	tmp = nvic_dma_stream_vector_add( (void(*)(u32*))&DMA_UART_IRQHandler, (u32*)&cb_uart[cb_Idx], dma_irq_number_tx.num, dmastreamtx);
	if (tmp != 0) bsp_panic(-30 + tmp);

	/* RX doesn't interrupt, but we need to show that the stream has been taken */
	tmp = nvic_dma_stream_vector_add( (void(*)(u32*))&DMA_UART_IRQHandler, (u32*)&cb_uart[cb_Idx], dma_irq_number_rx.num, dmastreamrx);
	if (tmp != 0) bsp_panic(-130 + tmp);


	/* Load some parameters that might be important. */
	cb_uart[cb_Idx].idma = dma_x;	// Save dma register base address
	cb_uart[cb_Idx].rxdma_stream = dmastreamrx; // Save stream number
	cb_uart[cb_Idx].txdma_stream = dmastreamtx; // Save stream number
	cb_uart[cb_Idx].iuart = iuart;	// Save uart register base address
	cb_uart[cb_Idx].flag = 2;	// Show this setup for dma driven
	cb_uart[cb_Idx].txblock = block;// How to handle buffer full

	/* Find DMA channel numbers for RX and TX, given stream number */
	switch (dma_x)
	{
	case DMA1_BASE:
		if (dma1_rxstreamtbl_4[dmastreamrx & 0x7] == iuart) {dma_channel_number_rx = 4; break;}
		if (dma1_rxstreamtbl_5[dmastreamrx & 0x7] == iuart) {dma_channel_number_rx = 5; break;}
		bsp_panic(-6);	// RX stream specified is not compatible with UART/DMA1

	case DMA2_BASE:
		if (dma2_rxstreamtbl_4[dmastreamrx & 0x7] == iuart) {dma_channel_number_rx = 4; break;}
		if (dma2_rxstreamtbl_5[dmastreamrx & 0x7] == iuart) {dma_channel_number_rx = 5; break;}
		bsp_panic(-7);	// RX stream specified is not compatible with UART/DMA2
	default:
		bsp_panic(-8);	// Something seriously wrong here!

	}

	switch (dma_x)
	{
	case DMA1_BASE:
		if (dma1_txstreamtbl_4[dmastreamtx & 0x7] == iuart) {dma_channel_number_tx = 4; break;}
		if (dma1_txstreamtbl_5[dmastreamtx & 0x7] == iuart) {dma_channel_number_tx = 5; break;}
		if (dma1_txstreamtbl_7[dmastreamtx & 0x7] == iuart) {dma_channel_number_tx = 7; break;}
		bsp_panic(-9);	// TX stream specified is not compatible with UART/DMA1

	case DMA2_BASE:
		if (dma2_txstreamtbl_4[dmastreamtx & 0x7] == iuart) {dma_channel_number_tx = 4; break;}
		if (dma2_txstreamtbl_5[dmastreamtx & 0x7] == iuart) {dma_channel_number_tx = 5; break;}
		bsp_panic(-10);	// TX stream specified is not compatible with UART/DMA2
	default:
		bsp_panic(-11);	// Something seriously wrong here!

	}

	qqq = dma_channel_number_rx;
/* At this point we (should!) have a good DMA channel and stream that associates with the UART. */

	/* Set up UART pins, port & uart clockings.  THIS IS BOARD SPECIFIC */
	if (uart_pins(iuart, &uartpins) != 0) bsp_panic(-15);

	/* Setup baud rate */
	usartx_setbaud (iuart, uartpins.pclk, baud);

	/* Obtain some buffer space */
	getbuff(cb_Idx, rxbuffsize, txbuffsize);

	/* ---------- Set up UART ----------------------------------------------------------------------------------- */

	/* Set up CR1 ---------------------------------------------------- */
	USART_CR1(iuart) |= (1<<13) | (1<<3) | (1<<2);// Set Usart enable, tx enable, rx enable

	/* Hook up usart tx and rx to dma channels */
	USART_CR3(iuart) |= (1<<7) | (1<<6);


	/* Setup CR2 ------------------------------------------------------------------- */
	/* Number of stop bits code. */
	USART_CR2(iuart) |= ((nstop & 0x3) << 12);

	/* --------- Set up the DMA channels ------------------------------------------------------------------------ */

	/* Set peripheral address for RX */
	DMA_SPAR(dma_x,dmastreamrx) = (u32*)(iuart + 0x04); // Address of uart DR register

	/* Set peripheral address for TX */
	DMA_SPAR(dma_x,dmastreamtx) = (u32*)(iuart + 0x04); // Address of uart DR register

	/* DMA stream configuration register--RX p 325 */
	//                                       Channel number     | MINC    | CIRC   | Per->Mem 
	DMA_SCR(dma_x,dmastreamrx) = ( (dma_channel_number_rx<< 25) | (1<<10) | (1<<8) | (0x0<<6) );   

	/* DMA stream configuration register--TX */
	//                                       Channel number     | MINC    | CIRC   | Mem->per | priority
	DMA_SCR(dma_x,dmastreamtx) = ( (dma_channel_number_tx<< 25) | (1<<10) | (0<<8) | (0x1<<6) | (1<<16));

	/* Set RX memory address (stays forever) */
	DMA_SM0AR(dma_x,dmastreamrx) = cb_uart[cb_Idx].rxbuff_base;

	/* Set the number of bytes in the RX buff */
	DMA_SNDTR(dma_x,dmastreamrx) = cb_uart[cb_Idx].rxbuff_size;

	/* DMA for TX interrupts */
	NVICIPR (dma_irq_number_tx.num,dma_tx_int_priority);	// Set dma interrupt priority (tx)
	NVICISER(dma_irq_number_tx.num);			// Enable dma interrupt (tx)

	/* Final enabling of DMA */
	DMA_SCR(dma_x,dmastreamrx) |= (0x1);		// Enable rx stream
	DMA_SCR(dma_x,dmastreamtx) |= ((1<<4));	// TCIE (xfer complete interrupt), not enable stream

	return 0;	// SUCCESS!
}

/*******************************************************************************
 * int bsp_uart_getcount_ptr    (struct CB_UART* pctl);	// Select: control blk ptr
 * int bsp_uart_getcount_uartnum(int uartnum);	// Select: uart number
 * @brief	: Get the number of bytes buffered
 * @param	: pctl = pointer uart control block
 * @return	: number of chars in currently buffered.
*******************************************************************************/
int bsp_uart_getcount_ptr(struct CB_UART* pctl)
{
	int	Diff;

	if (pctl->flag == 1)
	{ // Here, interrupt driven uart
		Diff = (int)(pctl->rxbuff_in - pctl->rxbuff_out);
		if (Diff < 0)
			Diff += pctl->rxbuff_size;  // Adjust for wrap
		return Diff;
	}

	if (pctl->flag == 2)
	{ // Here, DMA driven uart
		/* Difference between where we are taking out chars, and where DMA is/was storing */
		Diff = ( pctl->rxbuff_end - DMA_SNDTR(pctl->idma,pctl->rxdma_stream) - pctl->rxbuff_out );
		if (Diff < 0)
			Diff += pctl->rxbuff_size;  // Adjust for wrap
		return Diff;
	}
	return 0;	// Something wrong if we got here.
}
/* ---------------------------------------------------------------------------- */
int bsp_uart_getcount_uartnum(int uartnum)	// Select by uart number (1 - n)
{
	if ( ((uartnum - 1) < 0) || ((uartnum - 1) > 7) ) return 0;
	return bsp_uart_getcount_ptr(&cb_uart[uartnum - 1]);
}
/*******************************************************************************
 * char bsp_uart_getc_ptr(struct CB_UART* pctl);	// Select: control blk ptr
 * char bsp_uart_getc_uartnum(int uartnum);	// Select: uart number
 * @brief	: Get one char
 * @param	: pctl = pointer uart control block
 * @return	: char from buffer
*******************************************************************************/
char bsp_uart_getc_ptr(struct CB_UART* pctl)
{
	char	c;
	u32 	i = 0;

	/* Hang waiting for incoming bytes if bozo caller didn't check char count. */
	while (( bsp_uart_getcount_ptr(pctl) == 0)  && (i++ < 50000) ) ;
	if (i >= 50000) panic_leds(-50);

	c = *pctl->rxbuff_out;
	pctl->rxbuff_out = rxbuff_adv(pctl, pctl->rxbuff_out);	// Advance pointer common routine
	return c;
}
/* ---------------------------------------------------------------------------- */
char bsp_uart_getc_uartnum(int uartnum)	// Select by uart number (1 - n)
{
	if (((uartnum - 1) < 0) || ((uartnum - 1) > 7)) return 0;
	return bsp_uart_getc_ptr(&cb_uart[uartnum - 1]);
}
/*******************************************************************************
 * int bsp_uart_getn_ptr(struct CB_UART* pctl, char *pchr, int len);	// Select: control blk ptr
 * int bsp_uart_getn_uartnum(int uartnum, char *pchr, int len);		// Select: uart number
 * @brief	: Attempt to get 'len' number of chars
 * @param	: pctl = pointer uart control block
 * @param	: pchr = pointer to output char buffer
 * @param	: len = number of bytes requested
 * @return	: number of chars actually transferred
*******************************************************************************/
int bsp_uart_getn_ptr(struct CB_UART* pctl, char *pchr, int len)
{
	int 	i;
	int 	ct;

	ct = bsp_uart_getcount_ptr(pctl);
	if (ct == 0) return 0;

	if (ct > len) ct = len; // Check for more ready than requested

	for (i = 0; i < ct; i++)
	{
		*pchr++ = *pctl->rxbuff_out;	// Copy from buffer to output
		pctl->rxbuff_out = rxbuff_adv(pctl, pctl->rxbuff_out);	// Advance pointer common routine
	}
	return ct; // Return number transfered.
}
/* ---------------------------------------------------------------------------- */
int bsp_uart_getn_uartnum(int uartnum, char *pchr, int len)	// Select by uart number (1 - n)
{
	if ( ((uartnum - 1) < 0) || ((uartnum - 1) > 7)) return 0;
	return bsp_uart_getn_ptr(&cb_uart[uartnum - 1],  pchr, len);
}
/*******************************************************************************
 * void bsp_uart_start_dma(struct CB_UART* pctl);
 * @brief	: If DMA driven uart, start DMA sending if not already sending
 * @param	: pctl: control block poiner
 * @return	:
*******************************************************************************/
void bsp_uart_start_dma(struct CB_UART* pctl)
{
	if (pctl->flag != 2) return;	// Return, not DMA driven
	
	/* Are we already running? */
	// First look at the enable bit
	if ((DMA_SCR(pctl->idma, pctl->txdma_stream) & 0x1) != 0) return;	// Already running


	// Check if the counter has gone to zero
	if ( DMA_SNDTR(pctl->idma,pctl->txdma_stream) != 0) return;
	
	/* Not running.  If there are any to send, set up the DMA. */
	common_dma(pctl);

	return;
}

/*******************************************************************************
 * void bsp_uart_send_int(struct CB_UART* pctl);
 * @brief	: If uart tx is idle, and not dma driven, start tx
 * @param	: pctl = pointer uart control block
 * @return	: 
*******************************************************************************/
void bsp_uart_send_int(struct CB_UART* pctl)
{
	if (pctl->flag != 1) return;	// Return not interrupt drive & initialized

	if ((USART_CR1(pctl->iuart) & USART_CR1_TXEIE) != 0) // Already enabled?
		return;

	USART_CR1(pctl->iuart) |= USART_CR1_TXEIE ;	// Enable interrupt
	return;
}
/*******************************************************************************
 * int bsp_uart_putc_ptr(struct CB_UART* pctl, char c); // Select: control blk ptr
 * int bsp_uart_putc_uartnum(int uartnum, char c);	// Select: uart number
 * @brief	: Put char.  Add a char to output buffer
 * @param	: pctl = pointer uart control block
 *       	: uartnum = uart number 1 - n
 *       	: fd = file descriptor
 * @param	: Char to be sent
 * @return	: 1 = one char add; 0 = no chars added; -1 = pctl was null
 * NOTE: this does not start an idle DMA, but starts an idle interrupt driven uart
*******************************************************************************/
int bsp_uart_putc_ptr(struct CB_UART* pctl, char c) // Select by control block ptr
{
	u8* p;
	u32 limitctr = 0;

	if (pctl == 0) return -1;	// Bogus control block pointer.

	/* Check if buffer filled.  Return zero if full. */
	p = txbuff_adv(pctl, pctl->txbuff_in); 	// Advance input pointer (circularly)
	while ((p == pctl->txbuff_out) && (pctl->txblock == 0))
	{ // Wait for buffer to empty, but don't hang forever.
		limitctr += 1;
		if (limitctr > 168000000) return 0;
		return 0; 	// Return zero, rather than overwrite buffer
	}

	*pctl->txbuff_in = (u8)c;		// Store char

	/* Update the input buffer pointer to the next position. */
	pctl->txbuff_in = p;

	/* Start sending if non-dma */
	bsp_uart_send_int(pctl);

	/* Start sending: if dma and not idle. */
	bsp_uart_start_dma(pctl); 

 	return 1; // Return byte count stored.
}
/* ---------------------------------------------------------------------------- */
int bsp_uart_putc_uartnum(int uartnum, char c)	// Select by uart number (1 - n)
{
	if (((uartnum - 1) < 0) || ((uartnum - 1) > 7)) return 0;
	return bsp_uart_putc_ptr(&cb_uart[uartnum - 1], c);
}
/*******************************************************************************
 * int bsp_uart_puts_ptr(struct CB_UART* pctl, char* p); // Select: control blk ptr
 * int bsp_uart_puts_uartnum(int uartnum,char* p);	// Select: uart number
 * @brief	: Put String:  Add a zero terminated string to output buffer
 * @param	: pctl = pointer uart control block
 *       	: uartnum = uart number 1 - n
 *       	: fd = file descriptor
 * @param	: p = pointer to buffer with bytes to be added
 * @return	: count of bytes added to buffer
 * NOTE: this starts either DMA or interrupt driven 
*******************************************************************************/
int bsp_uart_puts_ptr(struct CB_UART* pctl, char* p) // Select: control blk ptr
{
	u32 ct = 0;
	int ret;

	if (pctl == 0) return -1;	// Pointer not set
	
	while (*p != 0)
	{
		ret = bsp_uart_putc_ptr(pctl, *p);	// Put char
		if (ret > 0)
		{ // Here, one byte was stored in buffer
			ct += ret;	// Add to count loaded
			p++;
		} // Note: loop hangs when buffer is full
	}

	return ct;
}
/* ---------------------------------------------------------------------------- */
int bsp_uart_puts_uartnum(int uartnum,char* p)	// Select by uart number (1 - n)
{
	if (((uartnum - 1) < 0) || ((uartnum - 1) > 7)) return 0;
	return bsp_uart_puts_ptr(&cb_uart[uartnum - 1], p);
}
/*******************************************************************************
 * int bsp_uart_putn_ptr(struct CB_UART* pctl, char* p, int len); // Select: control blk ptr
 * int bsp_uart_putn_uartnum(int uartnum, char* p, int len);	// Select: uart number
 * @brief	: Put char wit count.  Add 'n' chars to output buffer
 * @param	: pctl = pointer uart control block
 *       	: uartnum = uart number 1 - n
 *       	: fd = file descriptor
 * @param	: p = input char pointer
 * @param	: count = number of bytes to be added
 * @return	: count of bytes added to buffer
 * NOTE: this starts either DMA or interrupt driven 
*******************************************************************************/
int bsp_uart_putn_ptr(struct CB_UART* pctl, char* p, int len ) // Select: control blk ptr
{
	u32 ct = len;
	int ret;
/* 'ct' and 'len' separate in case we decide to change it to return when the ct loaded is not equal len. */
	if (pctl == 0) return -1;	// Pointer not set
	
	while (len > 0)
	{
		while ((ret = bsp_uart_putc_ptr(pctl, *p)) == 0) ; // Loop if full 
		if (ret < 0 ) return ret; // Error.
		p++;		// 
		len -= 1;	// Decrement input count
	}
	return (ct - len);	// Return count added to buffer
}
/* ---------------------------------------------------------------------------- */
int bsp_uart_putn_uartnum(int uartnum, char* p, int len )	// Select by uart number (1 - n)
{
	if (((uartnum - 1) < 0) || ((uartnum - 1) > 7)) return 0;
	return bsp_uart_putn_ptr(&cb_uart[uartnum - 1], p, len);
}

/*#######################################################################################
 * No ISR routines needed for RX!
 *####################################################################################### */

/* ---------------------- glorious rx'ing ----------------------------------------------- */

/*#######################################################################################
 * static void common_dma(struct CB_UART* pctl);
 * @brief	: Set up a disabled (idle) DMA to send if there are byte buffered
 * @param	: pctl = pointer to uart control block
 *####################################################################################### */

static void common_dma(struct CB_UART* pctl)
{
/* NOTE: This routine is entered from mainline only if the DMA is idle, and entered
   after a DMA interrupt, after it is disabled.  Therefore, the following is based on
   a given stream DMA interrupt not occuring while the mainline is mucking around in this
   routine. */
	int tmp;
	u8* ptmp;

	/* Are there bytes buffered? (Certainly yes if entered from mainline.) */  
	tmp = (pctl->txbuff_in - pctl->txbuff_out);
	if (tmp == 0) return;	

	if (tmp < 0)	// Wrap around check.
	{ // Here, there is wrap-around, so send what remains in (non-circular) buffer.
	  // (Upon the next DMA interrupt what remains after the wrap-around will be sent.)

		// Compute number of bytes remaining to the end of the buffer
		tmp = (pctl->txbuff_end - pctl->txbuff_out); 			// Remaining ct
		DMA_SM0AR(pctl->idma,pctl->txdma_stream) = pctl->txbuff_out; 	// Set TX mem address
		pctl->txbuff_dmanext = pctl->txbuff_base;				// Save new start ptr
	}
	else
	{ // Here, no wrap around, so all buffered bytes can be sent with one setting
		DMA_SM0AR(pctl->idma,pctl->txdma_stream) = pctl->txbuff_out; // Set TX mem address
// redundant	tmp = pctl->txbuff_in - pctl->txbuff_out);		// Number of bytes to send
		ptmp = tmp + pctl->txbuff_out;
		if (ptmp >= pctl->txbuff_end)
			ptmp = pctl->txbuff_base;
		pctl->txbuff_dmanext = ptmp;				// Save new start ptr
	}

	DMA_SNDTR(pctl->idma,pctl->txdma_stream)  = tmp;		// Set number of bytes
	DMA_SCR (pctl->idma,pctl->txdma_stream) |= 0x1;		// Enable DMA and away we go!

	return;	
}

/*#######################################################################################
 * ISR DMA TX routine: Entered from nvic_dma_mgr.c which dispatches the DMAx_STREAMy interrupts
 *
 * void DMA_UART_IRQHandler(volatile u32* pctl) // USART1 Tx
 * @param	: pctl = pointer to uart control block
 *####################################################################################### */
const char tcif_tbl[4] = {0, 6, 16, 22};	// Shifts for resetting interrupt flags

void DMA_UART_IRQHandler(volatile u32* pall)
{
	/* The following is for consistency in the code in this file. ('nvic_dma_mgr.c' uses volatile u32*) */
	struct CB_UART* pctl = (struct CB_UART*)pall;

	/* Clear all interrupt flags for this DMA stream */
	if (pctl->txdma_stream > 3)
	{// High register
		DMA_HIFCR(pctl->idma) = (0x3d << tcif_tbl[pctl->txdma_stream-4]);
	}
	else
	{ // Low register
		DMA_LIFCR(pctl->idma) = (0x3d << tcif_tbl[pctl->txdma_stream]);
	}
	pctl->txbuff_out = pctl->txbuff_dmanext; // Update where in the buffer we have xmitted

	/* Here, check if there are bytes buffered and if so, figure out how to send them. */
	common_dma(pctl);
	return;

}
/* &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
   Interrupting 
   &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& */
/*******************************************************************************
 * int bsp_uart_int_init_number(u32 uartnumber, u32 baud, u32 rxbuffsize, u32 txbuffsize,  u32 uart_int_priority, u8 block, u8 nstop);
 * @brief	: A wrapper for 'int bsp_uart_int_init' with uart number (1-6) rather than uart base address as the first argument
 * @return	: Same as bsp_uart_int_init
*******************************************************************************/
int bsp_uart_int_init_number(u32 uartnumber, u32 baud, u32 rxbuffsize, u32 txbuffsize,  u32 uart_int_priority, u8 block, u8 nstop)
{
	u32 iuart = uartnumber_to_baseaddr(uartnumber);
	return  bsp_uart_int_init(iuart, baud, rxbuffsize, txbuffsize, uart_int_priority, block, nstop);
}
/*******************************************************************************
 * int bsp_uart_int_init(u32 iuart, u32 baud, u32 rxbuffsize, u32 txbuffsize,  u32 uart_int_priority, u8 block, u8 nstop);
 * @brief	: Initialize USART/UART for DMA transfers
 * @param	: iuart: pointer to UART base, e.g. 'USART1'
 * @param	: baud: name says it all, e.g. '921600'
 * @param	: rxbuffsize: number of bytes in a circular rx buffer
 * @param	: txbuffsize" number of bytes in a circular tx buffer
 * @param	: uart_int_priority: interrupt priority, (0x00 - 0xf0) e.g. 0xc0, low 4 bits zero
 * @param	: block: 0 = loop when buffer full; non-zero = discard char when buff full
 * @param	: nstop: 0 = 1.0; 1 = 0.5; 2 = 2; 3 = 1.5 bits
 * @return	: 0 = success; fail traps to 'panic_leds'
*******************************************************************************/

int bsp_uart_int_init(u32 iuart, u32 baud, u32 rxbuffsize, u32 txbuffsize,  u32 uart_int_priority, u8 block, u8 nstop)
{
	u32 cb_Idx;
	struct UARTPINS uartpins;

	if ((uart_int_priority & 0xf) != 0) bsp_panic(-3); // Bogus priority
	
	/* Map usart/uart register base to control block index & and enable APBxENR for USART/UART */
	cb_Idx = mapindex(iuart);	
		
	cb_uart[cb_Idx].iuart = iuart;	// Save uart register base address
	cb_uart[cb_Idx].flag = 1;	// Show this setup for interrupt driven
	cb_uart[cb_Idx].txblock = block;// How to handle buffer full

	/* Mapping adjusted file number to a control block pointer */
	cb_map[cb_Idx] = &cb_uart[cb_Idx];

	/* Set up UART pins, port & uart clockings.  THIS IS BOARD SPECIFIC */
	if (uart_pins(iuart, &uartpins) != 0) bsp_panic(-15);

	/* Setup baud rate */
	usartx_setbaud (iuart, uartpins.pclk, baud);

	/* Obtain some buffer space */
	getbuff(cb_Idx, rxbuffsize, txbuffsize);

	/* Number of stop bits code. */
	USART_CR2(iuart) |= ((nstop & 0x3) << 12);

	/* Set up CR1 ---------------------------------------------------- */
	//                    UE       RXNEIE    TE       RE
	USART_CR1(iuart) |= (1<<13) | (1<<5) | (1<<3) | (1<<2);

	/* UART interrupt */
	NVICIPR (uartpins.irqnumber, uart_int_priority);	// Set uart interrupt priority (tx)
	NVICISER(uartpins.irqnumber);				// Enable interrupt
	
	return 0;
}

/*#######################################################################################
 * ISR routine
 *####################################################################################### */
/* USART/UART interrupts use a common IRQ handler.  Pass the control block pointer. */
void UART_IRQHandler(struct CB_UART* pctl);	// Prototype
void USART1_IRQHandler(void) {UART_IRQHandler(&cb_uart[0]); return;}
void USART2_IRQHandler(void) {UART_IRQHandler(&cb_uart[1]); return;}
void USART3_IRQHandler(void) {UART_IRQHandler(&cb_uart[2]); return;}
void  UART4_IRQHandler(void) {UART_IRQHandler(&cb_uart[3]); return;}
void  UART5_IRQHandler(void) {UART_IRQHandler(&cb_uart[4]); return;}
void USART6_IRQHandler(void) {UART_IRQHandler(&cb_uart[5]); return;}
void  UART7_IRQHandler(void) {UART_IRQHandler(&cb_uart[6]); return;}
void  UART8_IRQHandler(void) {UART_IRQHandler(&cb_uart[7]); return;}

void UART_IRQHandler(struct CB_UART* pctl)
{
	/* Receive */
	if ((USART_SR(pctl->iuart) & USART_SR_RXNE) != 0) // Receive reg loaded?
	{  // Here, receive interrupt flag is on. 
		*pctl->rxbuff_in = USART_DR(pctl->iuart);// Read and store char

		/* Advance pointers to line buffers and array of counts and reset when end reached */	
		pctl->rxbuff_in = rxbuff_adv(pctl, pctl->rxbuff_in);	// Advance pointers common routine
	}

	/* Transmit */
	if ( (USART_CR1(pctl->iuart) & USART_CR1_TXEIE) != 0)
	{  // Here, yes.  Transmit interrupts are enabled so check if a tx interrupt
		if ( (USART_SR(pctl->iuart) & USART_SR_TXE) != 0) // Transmit register empty?
		{ // Here, yes.
			USART_DR(pctl->iuart) = *pctl->txbuff_out;// Send next char, step pointer

			/* Advance output pointer */
			pctl->txbuff_out = txbuff_adv(pctl, pctl->txbuff_out);

			/* Was the last byte loaded the last to send? */		
			if (pctl->txbuff_out == pctl->txbuff_in)
			{ // Here yes. 
				USART_CR1(pctl->iuart) &= ~USART_CR1_TXEIE;		// Disable Tx interrupt	
			}	
		}
	}
	return;
}




