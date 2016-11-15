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

#include "bsp_uart.h"
#include "libopencm3/stm32/usart.h"
#include "libopencm3/stm32/f3/nvic.h"
//#include "libopencm3/stm32/nvic_f4.h"
#include "libopencm3/stm32/dma.h"
#include "libopencm3/stm32/gpio.h"
#include "libopencm3/stm32/rcc.h"
#include "nvicdirect.h"
#include "panic_ledsDf3.h"
#include "nvic_dma_mgrf3.h"
#include "f3DISCpinconfig.h"
#include "usartx_setbaud.h"
#include "uart_pinsf3.h"


/* Static routines */
static void common_dma(struct CB_UART* pctl);

/* Error trapping */
int PANIC_x;	// Global variable to look at.
void bsp_panic (int x)
{ PANIC_x = x; panic_ledsDf3(7); } // Save code, and flash LEDs 7x

//urt2 = bsp_uart_dma(USART2, 921600, 256, 256, 7, 2); // UARTx, baud, tx buff size, rx buff size, tx dma channel, rx dma channel -- FTDI

#define FDMAX 5	// Number of file descriptors allowed
struct CB_UART cb_uart[FDMAX];	// Control blocks for each possible UART
struct CB_UART* cb_map[FDMAX];	// Pointers to control block versus array index (USART1 = 0,...,USART5 = 4)

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

	default:
		bsp_panic(-61);	// No match for request
	}
	/* Enable USART/UART module on appropriate bus */
	switch (idx)
	{
	case 0:	RCC_APB2ENR |= (1 << 14); break;
	case 1:	RCC_APB1ENR |= (1 << 17); break;
	case 2:	RCC_APB1ENR |= (1 << 18); break;
	case 3:	RCC_APB1ENR |= (1 << 19); break;
	case 4:	RCC_APB1ENR |= (1 << 20); break;

	default:
		bsp_panic(-62);	// Wrong
	}

	return idx;
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
/* This struct is used during initialization. */
struct DMASTUFF
{
	u32	dma;	// Base address for this uart
	int	numtx;	// DMA vector IRQ number for the dma channel
	int	numrx;	// DMA vector IRQ number for the dma channel
	uint32_t dmachanneltx;	// TX channel number
	uint32_t dmachannelrx;	// RX channel number	
	u32 	iuart;	// USART/UART base address
	u32	uartnumber;	// Save it
};

static void uartnumber_to_baseaddr(struct DMASTUFF* p, u32 uartnumber)
{
	p->uartnumber = uartnumber;	// Save of use by others
	p->dma        = DMA1_BASE;
	p->numtx      = DMASTRM11-1;
	p->numrx      = DMASTRM11-1;
	switch (uartnumber)
	{
	case 1:	p->iuart = USART1;	
		p->dmachanneltx = 4;
		p->dmachannelrx = 5;
		break;
	case 2:	p->iuart = USART2;
		p->dmachanneltx = 6;
		p->dmachannelrx = 7;
		break;
	case 3:	p->iuart = USART3;
		p->dmachanneltx = 2;
		p->dmachannelrx = 3;
		break;
	case 4:	p->iuart =  UART4;
		p->dma = DMA2_BASE;
		p->dmachanneltx = 5;
		p->dmachannelrx = 3;
		p->numtx = DMASTRM21-1;
		p->numrx = DMASTRM21-1;
		break;
	case 5:	p->iuart =  UART5;
		p->dmachanneltx = -1;	// Can' do DMA with UART5
		p->dmachannelrx = -1;
		break;
	default: // (Test for this routine call should not make this possible)
		bsp_panic(-66);	// No match for request
	}
	p->numtx += p->dmachanneltx; // Set IRQ numbers
	p->numrx += p->dmachannelrx;	
	return;
}
/*******************************************************************************
 * int bsp_uart_dma_init_number(u32 uartnumber, u32 baud,u32 rxbuffsize, u32 txbuffsize,, u32 dma_tx_int_priority, u8 block, u8 nstop);
 * @brief	: Initialize USART/UART for DMA transfers
 * @param	: uartnumber (1 - 5)
 * @param	: baud: name says it all, e.g. '921600'
 * @param	: rxbuffsize: number of bytes in a circular rx buffer
 * @param	: txbuffsize" number of bytes in a circular tx buffer
 * @param	: uart_int_priority: interrupt priority, (0x00 - 0xf0) e.g. 0xc0, low 4 bits zero
 * @param	: block: 0 = loop when buffer full; non-zero = discard char when buff full
 * @param	: nstop: 0 = 1.0; 1 = 0.5; 2 = 2; 3 = 1.5 bits
 * @return	: 0 = success; fail traps to 'panic_leds'
*******************************************************************************/
static u32 qqq;	// Debug
int bsp_uart_dma_init_number(u32 uartnumber, u32 baud,u32 rxbuffsize, u32 txbuffsize, u32 dma_tx_int_priority, u8 block, u8 nstop)
{
	int tmp;
	u32 cb_Idx;	// Used "everywhere" to index into the control block array
	struct UARTPINS uartpins;
	struct DMASTUFF dmastuff;	// Stuff for initialization 

#ifdef STM32F373
#pragma message ("bsp_uart.c -- DMA STM32F373")
	if ((uartnumber == 0) || (uartnumber > 3)) bsp_panic(-222);	// Bozo check.

#elif STM32F303
#pragma message ("bsp_uart.c -- DMA STM32F303")
	if ((uartnumber == 0) || (uartnumber > 4)) bsp_panic(-222);	// Bozo check.
#else
  #  error "either STM32F303 or STM32F373 must be specified for DMA selection, e.g., export STM32TYPE=STM32F373"
#endif
	/* Convert serial port number to a USART/UART module base address and other stuff. */
	uartnumber_to_baseaddr(&dmastuff, uartnumber);

	/* Be sure arguments passed are within range */
	if ((dma_tx_int_priority & 0xffffff0f) != 0) bsp_panic(-3); // Bogus priority
	
	/* Map usart/uart register base to control block index and enable RCC register for clocking. */
	cb_Idx = mapindex(dmastuff.iuart);	

	/* Set dma channel interrupt to revector to this routine; check if dma is in use. */
	tmp = nvic_dma_channel_vector_addf3( (void(*)(u32*))&DMA_UART_IRQHandler, (u32*)&cb_uart[cb_Idx], dmastuff.numtx, dmastuff.dmachanneltx);
	if (tmp != 0) bsp_panic(-30 + tmp);

	/* RX doesn't interrupt, but we need to show that the channel has been taken */
	tmp = nvic_dma_channel_vector_addf3( (void(*)(u32*))&DMA_UART_IRQHandler, (u32*)&cb_uart[cb_Idx], dmastuff.numrx, dmastuff.dmachannelrx);
	if (tmp != 0) bsp_panic(-130 + tmp);


	/* Load some parameters that might be important. */
	cb_uart[cb_Idx].idma = dmastuff.dma;	// Save dma register base address
	cb_uart[cb_Idx].rxdma_channel = dmastuff.dmachannelrx; // Save channel number
	cb_uart[cb_Idx].txdma_channel = dmastuff.dmachanneltx; // Save channel number
	cb_uart[cb_Idx].iuart = dmastuff.iuart;	// Save uart register base address
	cb_uart[cb_Idx].flag = 2;	// Show this setup for dma driven
	cb_uart[cb_Idx].txblock = block;// How to handle buffer full

	qqq = dmastuff.numrx; // Debug

/* At this point we (should!) have a good DMA channel and channel that associates with the USART/UART. */

	/* Set up UART pins, port & uart clockings.  THIS IS BOARD SPECIFIC */
	if (uart_pinsf3(dmastuff.iuart, &uartpins) != 0) bsp_panic(-15);

	/* Setup baud rate */
	usartx_setbaud (dmastuff.iuart, uartpins.pclk, baud);

	/* Obtain some buffer space */
	getbuff(cb_Idx, rxbuffsize, txbuffsize);

	/* ---------- Set up UART ----------------------------------------------------------------------------------- */

	/* Set up CR1 ---------------------------------------------------- */
	USART_CR1(dmastuff.iuart) |= (1<<3) | (1<<2);// Set Usart enable, tx enable, rx enable

	/* Hook up usart tx and rx to dma channels */
	USART_CR3(dmastuff.iuart) |= (1<<7) | (1<<6);

	/* Setup CR2 ------------------------------------------------------------------- */
	/* Number of stop bits code. */
	USART_CR2(dmastuff.iuart) |= ((nstop & 0x3) << 12);

	/* --------- Set up the DMA channels ------------------------------------------------------------------------ */

	/* Set peripheral address for RX */
	DMA_CPAR(dmastuff.dma,dmastuff.dmachannelrx) = (dmastuff.iuart + 0x24); // Address of uart RDR register

	/* Set peripheral address for TX */
	DMA_CPAR(dmastuff.dma,dmastuff.dmachanneltx) = (dmastuff.iuart + 0x28); // Address of uart TDR register

	/* DMA channel configuration register */
	//                                                     MINC | CIRC         | Per->Mem 
	DMA_CCR(dmastuff.dma,dmastuff.dmachannelrx) = (DMA_CCR_MINC | DMA_CCR_CIRC | (0x0<<4));   

	/* DMA channel configuration register--TX */
	//                                                     MINC |   Mem->per | priority
	DMA_CCR(dmastuff.dma,dmastuff.dmachanneltx) = (DMA_CCR_MINC |  (0x1<<4)  | (0x1<<12));

	/* Set RX memory address (stays forever) */
	DMA_CMAR(dmastuff.dma,dmastuff.dmachannelrx) = (u32)cb_uart[cb_Idx].rxbuff_base;

	/* Set the number of bytes in the RX buff */
	DMA_CNDTR(dmastuff.dma,dmastuff.dmachannelrx) = cb_uart[cb_Idx].rxbuff_size;

	/* DMA for TX interrupts */
	NVICIPR (dmastuff.numtx,dma_tx_int_priority);	// Set dma interrupt priority (tx)
	NVICISER(dmastuff.numtx);			// Enable dma interrupt (tx)

	/* Final enabling of DMAs */
	DMA_CCR(dmastuff.dma,dmastuff.dmachannelrx) |= (0x1);	// Enable rx channel
	DMA_CCR(dmastuff.dma,dmastuff.dmachanneltx) |= (1<<1);	// TCIE (not EN)

	USART_CR1(dmastuff.iuart) |= 0x1;	// Enable usart/uart

	return 0;	// SUCCESS!...(maybe)...
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
		Diff = ( pctl->rxbuff_end - DMA_CNDTR(pctl->idma,pctl->rxdma_channel) - pctl->rxbuff_out );
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
	if (i >= 50000) bsp_panic(-50);

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
	if ((DMA_CCR(pctl->idma,pctl->txdma_channel) & 0x1) != 0) return;	// Already running

	// Check if the counter has gone to zero
//$	if ( DMA_CNDTR(pctl->idma,pctl->txdma_channel) != 0) return;
	
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
	volatile u32 limitctr = 5000000;

	if (pctl == 0) return -1;	// Bogus control block pointer.

	/* Check if buffer filled.  Return zero if full. */
	p = txbuff_adv(pctl, pctl->txbuff_in); 	// Advance input pointer (circularly)

	/* Don't overwrite buffer of txblock not zero. */
	if (pctl->txblock != 0)
	{ // Loop for about 1 sec, waiting for txbuff_out to catch up. */
		while ((p == pctl->txbuff_out) && (limitctr-- > 0));
		if(limitctr <= 0) return 0;
	}
	*pctl->txbuff_in = (u8)c;		// Store char

	/* Update the input buffer pointer to the next position. */
	pctl->txbuff_in = p;

	/* Start sending if non-dma/interrupt driven */
	bsp_uart_send_int(pctl);

	/* Start sending: if dma driven and not idle. */
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
   a given channel DMA interrupt not occuring while the mainline is mucking around in this
   routine. */
	int tmp;
	u8* ptmp;

	/* Are there bytes buffered? (Certainly yes if entered from mainline.) */  
	tmp = (pctl->txbuff_in - pctl->txbuff_out);
	if (tmp == 0) 
	{
		return;	// No bytes in the buffer	
	}

	if (tmp < 0)	// Wrap around check.
	{ // Here, there is wrap-around, so send what remains in (non-circular) buffer.
	  // (Upon the next DMA interrupt what remains after the wrap-around will be sent.)

		// Compute number of bytes remaining to the end of the buffer
		tmp = (pctl->txbuff_end - pctl->txbuff_out); 			// Remaining ct
		DMA_CMAR(pctl->idma,pctl->txdma_channel) = (u32)pctl->txbuff_out; // Set TX mem address
		pctl->txbuff_dmanext = pctl->txbuff_base;			// Save new start ptr
	}
	else
	{ // Here, no wrap around, so all buffered bytes can be sent with one setting
		DMA_CMAR(pctl->idma,pctl->txdma_channel) = (u32)pctl->txbuff_out; // Set TX mem address
// redundant	tmp = pctl->txbuff_in - pctl->txbuff_out);	// Number of bytes to send
		ptmp = tmp + pctl->txbuff_out;	// Pointer to last char + 1
		if (ptmp >= pctl->txbuff_end)	// Handle wrap
			ptmp = pctl->txbuff_base;// Point to where next dma starts
		pctl->txbuff_dmanext = ptmp;			// Save new start ptr
	}

	DMA_CNDTR(pctl->idma,pctl->txdma_channel)  = tmp;	// Set number of bytes
	DMA_CCR  (pctl->idma,pctl->txdma_channel) |= 0x1;	// Enable DMA and away we go!

	return;	
}
/*#######################################################################################
 * ISR DMA TX routine: Entered from nvic_dma_mgr.c which dispatches the DMAx_STREAMy interrupts
 *
 * void DMA_UART_IRQHandler(volatile u32* pctl) // USART1 Tx
 * @param	: pctl = pointer to uart control block
 *####################################################################################### */
void DMA_UART_IRQHandler(volatile u32* pall)
{
	u32 tmp;

	/* The following is for consistency in the code in this file. ('nvic_dma_mgr.c' uses volatile u32*) */
	struct CB_UART* pctl = (struct CB_UART*)pall;
	
	tmp = (pctl->txdma_channel-1)*4;// Position bits to clear flags for this channel

	/* Clear all interrupt flags for this DMA channel */
	DMA_IFCR(pctl->idma) = (0xf << tmp);

	pctl->txbuff_out = pctl->txbuff_dmanext; // Update where in the buffer we have xmitted

//$	pctl->busy = 0;

	DMA_CCR  (pctl->idma,pctl->txdma_channel) &= ~0x1;

	/* Here, check if there are bytes buffered and if so, figure out how to send them. */
	common_dma(pctl);
	return;
}
/* &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
   Interrupting 
   &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& */
/*******************************************************************************
 * int bsp_uart_int_init_number(u32 uartnumber, u32 baud, u32 rxbuffsize, u32 txbuffsize,  u32 uart_int_priority, u8 block, u8 nstop);
 * @brief	: Initialize USART/UART for interrupt driven transfers
 * @param	: iuart: pointer to UART base, e.g. 'USART1'
 * @param	: baud: name says it all, e.g. '921600'
 * @param	: rxbuffsize: number of bytes in a circular rx buffer
 * @param	: txbuffsize" number of bytes in a circular tx buffer
 * @param	: uart_int_priority: interrupt priority, (0x00 - 0xf0) e.g. 0xc0, low 4 bits zero
 * @param	: block: 0 = loop when buffer full; non-zero = discard char when buff full
 * @param	: nstop: 0 = 1.0; 1 = 0.5; 2 = 2; 3 = 1.5 bits
 * @return	: 0 = success; fail traps to 'panic_leds'
*******************************************************************************/
u32 cb_Idx;
int bsp_uart_int_init_number(u32 uartnumber, u32 baud, u32 rxbuffsize, u32 txbuffsize,  u32 uart_int_priority, u8 block, u8 nstop)
{
	
	struct UARTPINS uartpins;
	struct DMASTUFF dmastuff;	// Stuff for initialization 

	/* Handle USART1,2,3 UART 4,5 but no DMA on 5. (F373 only does USART1,2,3) */
#ifdef STM32F373
#pragma message ("bsp_uart.c -- INTERRUPT STM32F373")
	if ((uartnumber == 0) || (uartnumber > 3)) bsp_panic(-222);	// Bozo check.

#elif STM32F303
#pragma message ("bsp_uart.c -- INTERRUPT STM32F303")
	if ((uartnumber == 0) || (uartnumber > 5)) bsp_panic(-222);	// Bozo check.
#else
  #  error "either STM32F303 or STM32F373 must be specified for DMA selection, e.g., export STM32TYPE=STM32F373"
#endif

	/* Convert serial port number to a USART/UART module base address and other stuff. */
	uartnumber_to_baseaddr(&dmastuff, uartnumber);

	if ((uart_int_priority & 0xf) != 0) bsp_panic(-3); // Bogus priority
	
	/* Map usart/uart register base to control block index & and enable APBxENR for USART/UART */
	cb_Idx = mapindex(dmastuff.iuart);	
		
	cb_uart[cb_Idx].iuart = dmastuff.iuart;	// Save uart register base address
	cb_uart[cb_Idx].flag = 1;	// Show this setup for interrupt driven
	cb_uart[cb_Idx].txblock = block;// How to handle buffer full

	/* Mapping adjusted file number to a control block pointer */
	cb_map[cb_Idx] = &cb_uart[cb_Idx];

	/* Set up UART pins, port & uart clockings.  THIS IS BOARD SPECIFIC */
	if (uart_pinsf3(dmastuff.iuart, &uartpins) != 0) bsp_panic(-15);

	/* Setup baud rate */
	usartx_setbaud (dmastuff.iuart, uartpins.pclk, baud);

	/* Obtain some buffer space */
	getbuff(cb_Idx, rxbuffsize, txbuffsize);

	/* Number of stop bits code. */
	USART_CR2(dmastuff.iuart) |= ((nstop & 0x3) << 12);

	/* Set up CR1 ---------------------------------------------------- */
	// Note: Setting TE sends an idle frame, therefore TXEIE must be enabled to handle
 	// the interrupt.
	//                           TXEIE     RXNEIE    TE       RE
	USART_CR1(dmastuff.iuart) |= ((1<<7) | (1<<5) | (1<<3) | (1<<2));
//USART_CR1(dmastuff.iuart) |= ((1<<3) | (1<<2));

	/* UART interrupt */
	NVICIPR (uartpins.irqnumber, uart_int_priority);	// Set uart interrupt priority (tx)
	NVICISER(uartpins.irqnumber);				// Enable interrupt
	
	USART_CR1(dmastuff.iuart) |= 0x1;	// Enable usart/uart

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

void UART_IRQHandler(struct CB_UART* pctl)
{
	/* Receive */
	if ((USART_ISR(pctl->iuart) & USART_ISR_RXNE) != 0) // Receive reg loaded?
	{  // Here, receive interrupt flag is on. 
		*pctl->rxbuff_in = USART_RDR(pctl->iuart);// Read and store char

		/* Advance pointers to line buffers and array of counts and reset when end reached */	
		pctl->rxbuff_in = rxbuff_adv(pctl, pctl->rxbuff_in);	// Advance pointers common routine
	}

	/* Transmit */
	if ( (USART_CR1(pctl->iuart) & USART_CR1_TXEIE) != 0)
	{  // Here, yes.  Transmit interrupts are enabled so check if a tx interrupt
		if ( (USART_ISR(pctl->iuart) & USART_ISR_TXE) != 0) // Transmit register empty?
		{ // Here, yes.
			USART_TDR(pctl->iuart) = *pctl->txbuff_out; // Send next char, step pointer

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

