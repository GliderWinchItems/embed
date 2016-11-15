/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : spi1eeprom.c
* Authorship         : deh
* Date First Issued  : 05/25/2013
* Board              : ../svn_sensor/hw/trunk/eagle/f103R/RxT6
* Description        : SPI1 routine for handling eeprom on sensor board
*******************************************************************************/

#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/gpio.h"
#include "libopenstm32/spi.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/dma.h"


#include "SENSORpinconfig.h"
#include "spi1eeprom.h"
#include "pinconfig_all.h"

/*
See p 672 Ref Manual (RM0008, Rev 14) for SPI.  Unless otherwise noted page numbers
refer to the Ref Manual.

Peripherals driven by bus clocks--
SYSCLK:
  AHB: (this is driven by SYSCLK)
    DMA1,2   
    APB1: (max 36 MHz)
      SPI2,3
    APB2
      SPI1

The above chain, each with dividers, determines the spi1 clock rate.
*/

/* This is used for determining the pre-scale divider code */
extern unsigned int	pclk2_freq;	// APB2 bus frequency in Hz (see 'lib/libmiscstm32/clockspecifysetup.c')

/* Set bus divider to stay at or below this freq */
#define SPI1CLKFREQ		2000000	// Set sclk freq

/* Pointer to a function that is called when DMA interrupt completes. */
void 	(*spi1_isrdoneptr )(void);	// Address of function to call upon completion of read.

static char 	 spi1_xmt_dummy;// SPI1 dummy byte sent when reading.
static volatile char	xmit1_busy_flag;// Flag: 0 = no transfer started; 1 = busy, a transfer is in progress.
static char	dummyread;	// Bit bucket for dumping dma read bytes, during a spi write.

/* I/O pin setup: {port, pin number, pin type setup, pin speed}  */
const struct PINCONFIGALL eeprom_ncs   = {(volatile u32 *)GPIOA,  4, OUT_PP   , MHZ_50};
const struct PINCONFIGALL eeprom_sck   = {(volatile u32 *)GPIOA,  5, OUT_AF_PP, MHZ_50};
const struct PINCONFIGALL eeprom_so    = {(volatile u32 *)GPIOA,  6, IN_FLT   , 0};
const struct PINCONFIGALL eeprom_si    = {(volatile u32 *)GPIOA,  7, OUT_AF_PP, MHZ_50};
const struct PINCONFIGALL eeprom_nwp   = {(volatile u32 *)GPIOC, 15, OUT_PP   , MHZ_50};

/******************************************************************************
 * int spi1eeprom_init(void);
 *  @return	: zero = success; not zero = failed
 *  @brief 	: Initialize SPI1 for eeprom
*******************************************************************************/
int spi1eeprom_init(void)
{
	int err;
	unsigned int uiX,uiZ,uiM;		// Used in computing baud divisor
	
	spi1_isrdoneptr  = 0;			// JIC--Set a null address to go to when read completes


	/* Enable: SPI1 and  bus clocking for alternate function */
	RCC_APB2ENR |= ((RCC_APB2ENR_SPI1EN) | (RCC_APB2ENR_AFIOEN));		// (p 109) 


	/* Configure gpio pins for SPI1 use */
		// PA4	EEPROM_/CS:	
		// PA5	EEPROM_SCLK:   SPI1_SCK  
		// PA6	EEPROM_SI:     SPI1_MOSI
		// PA7	EEPROM_SO:     SPI1_MISO 
		// PC15	EEPROM_/WP

	err =  pinconfig_all( (struct PINCONFIGALL *)&eeprom_sck);	// Clock out AF
	err |= pinconfig_all( (struct PINCONFIGALL *)&eeprom_so);	// SPI in
	err |= pinconfig_all( (struct PINCONFIGALL *)&eeprom_si);	// SPI out AF
	err |= pinconfig_all( (struct PINCONFIGALL *)&eeprom_ncs);	// Not chip select
	err |= pinconfig_all( (struct PINCONFIGALL *)&eeprom_nwp);	// Not write protect

	EEPROM_NCS_hi;	// Disable chip select
	EEPROM_NWP_lo;	// Enable write protect

	/* Compute baud divisor - (uiM = 0 for divide 2 to uiM = 7 for divide by 256)*/
	uiX = (pclk2_freq / SPI1CLKFREQ) + 1;
	uiM = 0; uiZ = 2;
	while ( uiZ < uiX){ uiM += 1; uiZ *= 2; } // Find divisor that exceeds uiX division ratio
	uiM += 1;

	/* SPI-CR1 (see p 714) */
	//         (    SSM  |  SSI   |enable peripheral|baud divisor | master select |CK level when idle | phase )
	SPI1_CR1 =    (1<<9) | (1<<8) |    (1 << 6)    | (uiM << 3)   |   (1 << 2)    |    (0 << 1)       |  0x00;

	/* SPI-CR2 (see p 692, ) */
	// Enable SPI1 to work with DMA (which would be DMA1CH3,2) 
	SPI1_CR2 =  0x03;			// TXDMAEN & RXDMAEN bits on.

	/* p 265, 273 (chart) */
	/* Setup DMA1 (p 672) */
	RCC_AHBENR |= RCC_AHBENR_DMA1EN;	// Enable DMA1 clock (p 102)

	/* Note: CCRx register will be setup when a read or a write operation is initiated */

	/* Set and enable interrupt controller for DMA transfer complete interrupt handling */
	NVICIPR (NVIC_DMA1_CHANNEL2_IRQ, DMA1_CH2_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_DMA1_CHANNEL2_IRQ);			// Enable interrupt controller DMA1CH2

	return err;
}
/******************************************************************************
 * void spi1_read (volatile char *p, int count, char xmit, void (*spi1_isrdoneptr )(void));
 * @brief	: read 'count' bytes, into buffer 'p' using DMA (see p 674)
 * @param	: char *p = pointer to byte buffer
 * @param	: int count = byte count to be read
 * @param	: char xmit  = outbound char during spi cycle
 * @param	: pointer to routine that is called when ISR completes
*******************************************************************************/
void spi1_read (volatile char *p, int count, char xmit, void (*isr_ptr )(void))
{
	/* The following should not be necessary, but it is here JIC. */
	while ( spi1_busy() != 0 );	// Loop until spi communication is complete

	spi1_isrdoneptr = isr_ptr;	// Save address that ISR should use when dma is finished
	spi1_xmt_dummy = xmit;		// Save the dummy byte that is xmitted for a read

	xmit1_busy_flag = 1;		// Show that a transfer operation is about to start

	EEPROM_NCS_lo;		// Enable chip select
	EEPROM_NWP_lo;		// Enable write protect (jic)

	/* Be sure the DMA's are disabled, otherwise we can't load the registers */
	DMA1_CCR2 &= ~0x01;		// Disable DMA1CH2 (spi1 read)
	DMA1_CCR3 &= ~0x01;		// Disable DMA1CH3 (spi1 write)

	/* If for some crazy reason the transfer complete flag is on, we want it off...now! p 276 */
	DMA1_IFCR = (0xf<<(4*(3-1))) | (0xf<<(4*(2-1)));// Clear all flags for CH2 & CH3

	// Channel configuration reg for channel 2 (spi read) (p 277)
	//            priority low |  8b mem xfrs | 8b spi xfrs  | mem increment | Interrupt 
	DMA1_CCR2 =  ( 0x00 << 12) | (0x00 << 10) |  (0x00 << 8) |   (1<<7)      | (1<<1)   ; 

	/* Seupup the DMA that will be sending dummy bytes */
	// Channel configuration reg for channel 3 (spi write) (p 277)
	//            priority low |  8b mem xfrs | 8b spi xfrs  | NO mem increment | Read from mem | No Interrupt 
	DMA1_CCR3 =  ( 0x00 << 12) | (0x00 << 10) |  (0x00 << 8) |   (0<<7)         |  (1<<4)       | (0<<1);

	/* Setup the DMA that will be writing bytes */
	DMA1_CNDTR3 = count;		// Number of data items (bytes in this case) for read (p 210)
	DMA1_CPAR3 = (u32)&SPI1_DR;	// DMA channel 3 peripheral address (p 211, 697)
	DMA1_CMAR3 = (u32)&spi1_xmt_dummy;// Memory address of the dummy byte to be sent

	/* Setup the DMA that will be reading bytes */	
	DMA1_CNDTR2 = count;		// Number of data items (bytes in this case) for read (p 210)
	DMA1_CPAR2 = (u32)&SPI1_DR;	// Set DMA peripheral address with spi data register address (p 211, 697)
	DMA1_CMAR2 = (u32)p;		// Memory address of first buffer array for storing data (p 211)

	DMA1_CCR2 |= 0x01;		// Enable DMA1CH2 (read) (do this first so it is ready for incoming bytes)
	DMA1_CCR3 |= 0x01;		// Enable DMA1CH3 (write)(now start up the writing)

	/* At this point DMA1CH3 will begin sending 'dummy'.  Since the memory is not incremented after
	   each transfer it sends the same byte.  When the DMA1CH3 transfer is complete there will be one
	   byte just loaded into the shift register, and the last byte loaded into the xmit buffer.  For the
	   read, DMA1CH2, the transfer complete will be almost two byte times later.  When DMA1CH2 does its
	   transfer complete interrupt all the write-side chars have completed and the last byte from the 
	   read buffer has been transfered.  All done.  */

	return;
}

/******************************************************************************
 * void spi1_write (char *p, int count, void (*isr_ptr )(void));
 * @brief	: read 'count' bytes into buffer 'p'
 * @param	: char *p = pointer to byte buffer
 * @param	: int count = byte count
 * @param	: pointer to routine that is called when ISR completes
*******************************************************************************/
void spi1_write (char *p, int count, void (*isr_ptr )(void))
{
	/* If the programmer bozo didn't check for busy we have no choice but to loop */
	while ( spi1_busy() != 0 );	// Loop until operation complete

	spi1_isrdoneptr = isr_ptr;	// Save address that ISR should use when dma is finished

	xmit1_busy_flag = 1;		// Show that a transfer operation is about to start

	EEPROM_NCS_lo;			// Enable chip select
	EEPROM_NWP_hi;			// Disable write protect

	/* Be sure the DMA's are disabled, otherwise we can't load the registers */
	DMA1_CCR2 &= ~0x01;		// Disable DMA1CH2 (spi1 read)
	DMA1_CCR3 &= ~0x01;		// Disable DMA1CH3 (spi1 write)

	/* If for some crazy reason the transfer complete flag is on, we want it off...now! */
	DMA1_IFCR = (0xf<<(4*(3-1))) | (0xf<<(4*(2-1)));// Clear all flags for CH2 & CH3

	/* Setup the DMA so that all the read bytes go into a dummy bucket */
	// Channel configuration reg for channel 2 (spi read) (p 209)                Data direction
	//            priority low |  8b mem xfrs | 8b spi xfrs  | NO mem increment | from peripheral | interrupt
	DMA1_CCR2 =  ( 0x00 << 12) | (0x00 << 10) |  (0x00 << 8) |   (0<<7)         | (0<<4)        | (1<<1);

	// Channel configuration reg for channel 3 (spi write) (p 209)                Data direction
	//            priority low |  8b mem xfrs | 8b spi xfrs  | mem increment    | Read from mem | Interrupt
	DMA1_CCR3 =  ( 0x00 << 12) | (0x00 << 10) |  (0x00 << 8) |   (1<<7)         |  (1<<4)       | (1<<1)  ;

	/* Place these instructions here so that there is sufficient delay between the /CS hi which might have been done
           by the calling progrlam (rw_eeprom.c) and the following /CS low. */
	EEPROM_NCS_lo;			// Enable chip select
	EEPROM_NWP_hi;			// Disable write protect

	/* Setup the DMA that will be writing bytes */
	DMA1_CNDTR3 = count;		// Number of data items (bytes in this case) to transfer (p 210)
	DMA1_CPAR3 = (u32)&SPI1_DR;	// Set DMA peripheral address with spi data register address (p 211, 697)
	DMA1_CMAR3 = (u32)p;		// Set DMA memory address with data pointer (p 211)

	/* Setup the DMA that will be reading bytes */	
	DMA1_CNDTR2 = count;		// Number of data items (bytes in this case) for read (p 210)
	DMA1_CPAR2 = (u32)&SPI1_DR;	// Set DMA peripheral address with spi data register address (p 211, 697)
	DMA1_CMAR2 = (u32)&dummyread;	// Set DMA memory address to an address for dumping bytes (p 211)

	DMA1_CCR2 |= 0x01;		// Enable DMA1CH2 (read) (do this first so it is ready for incoming bytes)
	DMA1_CCR3 |= 0x01;		// Enable DMA1CH3 (write)(now start up the writing)

/* At this point DMA1CH3 will respond to the spi xmit buffer empty and begin transfering bytes to
the xmit buffer each time it comes on.  When the last byte has been transferred, there will be two 
bytes "in progress."  One will be in the shift register being sent, and the last one in the xmit
buffer to be sent.  The communication is still in progress until the xmit buffer flag comes on, AND
the busy flag goes away */
	return;
}
/******************************************************************************
 * char spi1_busy (void);
 * @brief	: See if this whole mess is finished
 * @return	: 0 = not busy; 1 = busy doing all manner of things
*******************************************************************************/
char spi1_busy (void)
{
	if ( (xmit1_busy_flag != 0 ) 		|	/* Overall transfer flag */
	     ( (SPI1_SR & SPI_SR_TXE) == 0 )	|	/* Wait until transmit buffer is empty */
	     ( (SPI1_SR & SPI_SR_BSY) != 0 ) 	)	/* Then wait until busy goes away (see note p 673)*/
		return 1;
	return 0;
}

/******************************************************************************
 * int spi1_sclk_set_divisor_code (unsigned short usM);
 * @brief	: Change the baud rate divisor code
 * @param	: usM divisor code: 0 - 7.  Change is skipped for numbers outside range
 * @return	: zero = no Err; 1 = Busy, 2 = bad code number
*******************************************************************************/
int spi1_sclk_set_divisor_code (unsigned short usM)
{
	if (xmit1_busy_flag != 0 ) return 1;	// Check for busy
	if (usM > 7)	return 2;		// Check for bad divisor code
	SPI1_CR1 =  (SPI1_CR1 & ~(0x07<<3)) | (usM << 3); // Replace baud divisor
	return 0;
}
/******************************************************************************
 * unsigned int spi1_sclk_get_divisor_code (void);
 * @brief	: Get the current baud rate divisor code
 * @return	: Divisor code: 0 - 7
*******************************************************************************/
unsigned int spi1_sclk_get_divisor_code (void)
{
	return	(SPI1_CR1 >> 3) & 0x07;	// Get divisor code from registor
}
/*#######################################################################################
 * ISR routine: SPI/DMA receive
 *####################################################################################### */
void DMA1CH2_IRQHandler(void)
{ // Here, READ dma transfer complete interrupt
	if ( (DMA1_ISR & DMA_ISR_TCIF2) != 0 )	// Is this a transfer complete interrupt?
	{ // Here, yes.
		DMA1_IFCR = DMA_IFCR_CTCIF2;	// Clear transfer complete flag (p 208)
		DMA1_CCR2 &= ~0x01;		// Disable DMA1CH2 and transfer complete interrupt
		xmit1_busy_flag = 0;		// Show we are done with the write/read
		// Here this interrupt was for a spi read operation.  
		if (spi1_isrdoneptr != 0)	// Is there an address set to go to?
			(*spi1_isrdoneptr)();	// Yes, start the next step
	}
	return;
/* NOTE: For either read or write calls (above), the *dma* (dma1ch2, read) transfer complete interrupt
occurs when the spi has completed the last byte, so the spi and dma are now idle, i.e. not-busy.  The
spi xmit buffer goes empty when the last byte has been transfered to the shift register, but the receive buffer
flag comes on when the last byte has transfered to the receive buffer which in turn causes the last byte
of the DMA1CH2 to store and assert the interrupt flag.  */
}

