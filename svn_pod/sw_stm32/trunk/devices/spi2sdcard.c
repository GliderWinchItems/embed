/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : spi2sdcard.c
* Hackerees          : deh, caw,...
* Date First Issued  : 07/11/2011
* Board              : STM32F103VxT6_pod_mm or Olimex P103
* Description        : SPI2 routines for SDCARDs
*******************************************************************************/

#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/gpio.h"
#include "libopenstm32/spi.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/dma.h"


#include "PODpinconfig.h"
#include "spi2sdcard.h"
/*
See p 654 Ref Manual (RM0008) for SPI.  Unless otherwise noted page numbers
refer to the Ref Manual.

Peripherals driven by bus clocks--
SYSCLK:
  AHB: (this is driven by SYSCLK)
    DMA1,2   
    APB1: (max 36 MHz)
      SPI2,3
The above chain, each with dividers, determines the spi2 clock rate.
*/

/* HC12 assembly routine- snip
; Note: CPOL|CPHA 00 & 11 work, 10 & 01 do not work.  11 recommended by www.Flashgenie.net
...
*/

/* This is used for determining the pre-scale divider code */
extern unsigned int	pclk1_freq;	// APB1 bus frequency in Hz (see 'lib/libmiscstm32/clockspecifysetup.c')

/* SD card should do 25 MHz.  Set bus divider to stay at or below this freq */
#define SPI2CLKFREQ		7000000	// Set sclk freq

char 	*spi2_ptr;	// SPI2 buffer pointer
int 	 spi2_cnt;	// SPI2 byte counter
char 	 spi2_xmt_dummy;// SPI2 dummy byte sent when reading.
void 	(*spi2_isrdoneptr )(void);	// Address of function to call upon completion of read.
volatile char	xmit2_busy_flag;// Flag: 0 = no transfer started; 1 = busy, a transfer is in progress.
char	dummyread;	// Bit bucket for dumping dma read bytes, during a spi write.

/******************************************************************************
 * void spi2sdcard_init(void);
 *  @brief Initialize SPI for SD Card Adapater
*******************************************************************************/
void spi2sdcard_init(void)
{
	unsigned int uiX,uiZ,uiM;		// Used in computing baud divisor

	spi2_isrdoneptr  = 0;			// Set a null address to go to when read completes

	/* Enable clock for SPI2 */
	RCC_APB1ENR |= RCC_APB1ENR_SPI2EN;	// (p 105,106)

	/* Enable bus clocking for alternate function */
	RCC_APB2ENR |= (RCC_APB2ENR_AFIOEN);		// (p 103) 

	/* Enable regulator to turn on power to SD Card */
	SDCARDREG_on	// gpio macro
	/* Wait at least 1 ms for SD CARD to come up?  Call to timer might have to go here */

	/* Configure gpio pins for SPI2 use */
	// Clear CNF reset bit 01 = Floating input (reset state)
		// PB12	SD_CARD_CD/DAT3/CS: SPI2_NSS	
		// PB13	SD_CARD_CLK/SCLK:   SPI2_SCK  
		// PB14	SD_CARD_Dat0/D0:    SPI2_MISO 
		// PB15	SD_CARD_CMD/DI:	    SPI2_MOSI
	GPIO_CRH(GPIOB) &= 0x0000ffff ;	// Mask out CNF & MODE bits for PB4-PB7

	// Set In/Out and mode bits
		// PB12	SD_CARD_CD/DAT3/CS: SPI2_NSS	
		// PB13	SD_CARD_CLK/SCLK:   SPI2_SCK  
		// PB14	SD_CARD_Dat0/D0:    SPI2_MISO 
		// PB15	SD_CARD_CMD/DI:	    SPI2_MOSI

	GPIO_CRH(GPIOB)  |= ((( (GPIO_CNF_OUTPUT_PUSHPULL       <<2) | (GPIO_MODE_OUTPUT_50_MHZ) ) << (4*4)) \
			 |   (( (GPIO_CNF_OUTPUT_ALTFN_PUSHPULL <<2) | (GPIO_MODE_OUTPUT_50_MHZ) ) << (4*5)) \
			 |   (( (GPIO_CNF_INPUT_FLOAT           <<2) | (GPIO_MODE_INPUT        ) ) << (4*6)) \
			 |   (( (GPIO_CNF_OUTPUT_ALTFN_PUSHPULL <<2) | (GPIO_MODE_OUTPUT_50_MHZ) ) << (4*7)) );

	/* Compute baud divisor - (uiM = 0 for divide 2 to uiM = 7 for divide by 256)*/
	uiX = (pclk1_freq / SPI2CLKFREQ) + 1;
	uiM = 0; uiZ = 2;
	while ( uiZ < uiX){ uiM += 1; uiZ *= 2; } // Find divisor that exceeds uiX division ratio
	uiM += 1;

	/* SPI-CR1 (see p 693) */
	//         ( SSM  | SSI   |enable peripheral| baud divisor | master select | CK 1 when idle    | phase  )
	SPI2_CR1 =  (1<<9)|(1<<8) |     (1 << 6)    | (uiM << 3)   |   (1 << 2)    |    (0 << 1)       |  0x00    ;

	/* SPI-CR2 (see p 695) */
	// Enable SPI2 to work with DMA (which would be DMA1CH5,4) 
	SPI2_CR2 =  0x03;			// TXDMAEN & RXDMAEN bits on

	SDCARD_CS_low	// Macro is in ../devices/PODpinconfig.h

	/* Setup DMA1 (p 672) */
	RCC_AHBENR |= RCC_AHBENR_DMA1EN;	// Enable DMA1 clock (p 102)

	/* Note: CCRx register will be setup when a read or a write operation is initiated */

	/* Set and enable interrupt controller for DMA transfer complete interrupt handling */
	NVICIPR (NVIC_DMA1_CHANNEL4_IRQ, DMA1_CH4_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_DMA1_CHANNEL4_IRQ);			// Enable interrupt controller DMA1CH4

	return;
}
/******************************************************************************
 * void spi2_read (volatile char *p, int count, char xmit, void (*spi2_isrdoneptr )(void));
 * @brief	: read 'count' bytes, into buffer 'p' using DMA (see p 674)
 * @param	: char *p = pointer to byte buffer
 * @param	: int count = byte count to be read
 * @param	: char xmit  = outbound char during spi cycle
 * @param	: pointer to routine that is called when ISR completes
*******************************************************************************/
void spi2_read (volatile char *p, int count, char xmit, void (*isr_ptr )(void))
{
	/* The following should not be necessary, but it is here JIC. */
	while ( spi2_busy() != 0 );	// Loop until spi communication is complete

	spi2_isrdoneptr = isr_ptr;	// Save address that ISR should use when dma is finished
	spi2_xmt_dummy = xmit;		// Save the dummy byte that is xmitted for a read

	xmit2_busy_flag = 1;		// Show that a transfer operation is about to start
	
	/* Be sure the DMA's are disabled, otherwise we can't load the registers */
	DMA1_CCR4 &= ~0x01;		// Disable DMA1CH4 (spi2 read)
	DMA1_CCR5 &= ~0x01;		// Disable DMA1CH5 (spi2 write)

	/* If for some crazy reason the transfer complete flag is on, we want it off...now! */
	DMA1_IFCR = (0xf<<(4*(5-1))) | (0xf<<(4*(4-1)));// Clear all flags for CH4 & CH5

	// Channel configuration reg for channel 4 (spi read) (p 209)
	//            priority low |  8b mem xfrs | 8b spi xfrs  | mem increment | Interrupt 
	DMA1_CCR4 =  ( 0x00 << 12) | (0x00 << 10) |  (0x00 << 8) |   (1<<7)      | (1<<1)   ; 

	/* Seupup the DMA that will be sending dummy bytes */
	// Channel configuration reg for channel 5 (spi write) (p 209)
	//            priority low |  8b mem xfrs | 8b spi xfrs  | NO mem increment | Read from mem | Interrupt 
	DMA1_CCR5 =  ( 0x00 << 12) | (0x00 << 10) |  (0x00 << 8) |   (0<<7)         |  (1<<4)       | (1<<1);

	DMA1_CNDTR5 = count;		// Number of data items (bytes in this case) for read (p 210)
	DMA1_CPAR5 = (u32)&SPI2_DR;	// DMA channel 5 peripheral address (p 211, 697)
	DMA1_CMAR5 = (u32)&spi2_xmt_dummy;	// Memory address of the dummy byte to be sent

	/* Setup the DMA that will be reading bytes */	
	DMA1_CNDTR4 = count;		// Number of data items (bytes in this case) for read (p 210)
	DMA1_CPAR4 = (u32)&SPI2_DR;	// Set DMA peripheral address with spi data register address (p 211, 697)
	DMA1_CMAR4 = (u32)p;		// Memory address of first buffer array for storing data (p 211)

	DMA1_CCR4 |= 0x01;		// Enable DMA1CH4 (read) (do this first so it is ready for incoming bytes)
	DMA1_CCR5 |= 0x01;		// Enable DMA1CH5 (write)(now start up the writing)

	/* At this point DMA1CH5 will begin sending 'dummy'.  Since the memory is not incremented after
	   each transfer it sends the same byte.  When the DMA1CH5 transfer is complete there will be one
	   byte just loaded into the shift register, and the last byte loaded into the xmit buffer.  For the
	   read, DMA1CH4, the transfer complete will be almost two byte times later.  When DMA1CH4 does its
	   transfer complete interrupt all the write-side chars have completed and the last byte from the 
	   read buffer has been transfered.  All done.  */

	return;
}

/******************************************************************************
 * void spi2_write (char *p, int count, void (*isr_ptr )(void));
 * @brief	: read 'count' bytes into buffer 'p'
 * @param	: char *p = pointer to byte buffer
 * @param	: int count = byte count
 * @param	: pointer to routine that is called when ISR completes
*******************************************************************************/
void spi2_write (char *p, int count, void (*isr_ptr )(void))
{
	/* If the programmer bozo didn't check for busy we have no choice but to loop */
	while ( spi2_busy() != 0 );	// Loop until operation complete

	spi2_isrdoneptr = isr_ptr;	// Save address that ISR should use when dma is finished

	xmit2_busy_flag = 1;		// Show that a transfer operation is about to start

	/* Be sure the DMA's are disabled, otherwise we can't load the registers */
	DMA1_CCR4 &= ~0x01;		// Disable DMA1CH4 (spi2 read)
	DMA1_CCR5 &= ~0x01;		// Disable DMA1CH5 (spi2 write)

	/* If for some crazy reason the transfer complete flag is on, we want it off...now! */
	DMA1_IFCR = (0xf<<(4*(5-1))) | (0xf<<(4*(4-1)));// Clear all flags for CH4 & CH5

	/* Setup the DMA so that all the read bytes go into a dummy bucket */
	// Channel configuration reg for channel 4 (spi read) (p 209)                Data direction
	//            priority low |  8b mem xfrs | 8b spi xfrs  | NO mem increment | from peripheral | interrupt
	DMA1_CCR4 =  ( 0x00 << 12) | (0x00 << 10) |  (0x00 << 8) |   (0<<7)         | (0<<4)        | (1<<1);

	// Channel configuration reg for channel 5 (spi write) (p 209)                Data direction
	//            priority low |  8b mem xfrs | 8b spi xfrs  | mem increment    | Read from mem | Interrupt
	DMA1_CCR5 =  ( 0x00 << 12) | (0x00 << 10) |  (0x00 << 8) |   (1<<7)         |  (1<<4)       | (1<<1)  ;
	DMA1_CNDTR5 = count;		// Number of data items (bytes in this case) to transfer (p 210)
	DMA1_CPAR5 = (u32)&SPI2_DR;	// Set DMA peripheral address with spi data register address (p 211, 697)
	DMA1_CMAR5 = (u32)p;		// Set DMA memory address with data pointer (p 211)

	/* Setup the DMA that will be reading bytes */	
	DMA1_CNDTR4 = count;		// Number of data items (bytes in this case) for read (p 210)
	DMA1_CPAR4 = (u32)&SPI2_DR;	// Set DMA peripheral address with spi data register address (p 211, 697)
	DMA1_CMAR4 = (u32)&dummyread;	// Set DMA memory address to an address for dumping bytes (p 211)

	DMA1_CCR4 |= 0x01;		// Enable DMA1CH4 (read) (do this first so it is ready for incoming bytes)
	DMA1_CCR5 |= 0x01;		// Enable DMA1CH5 (write)(now start up the writing)

/* At this point DMA1CH5 will respond to the spi xmit buffer empty and begin transfering bytes to
the xmit buffer each time it comes on.  When the last byte has been transferred, there will be two 
bytes "in progress."  One will be in the shift register being sent, and the last one in the xmit
buffer to be sent.  The communication is still in progress until the xmit buffer flag comes on, AND
the busy flag goes away */
	return;
}
/******************************************************************************
 * char spi2_busy (void);
 * @brief	: See if this whole mess is finished
 * @return	: 0 = not busy; 1 = busy doing all manner of things
*******************************************************************************/
char spi2_busy (void)
{
	if ( (xmit2_busy_flag != 0 ) 		|	/* Overall transfer flag */
	     ( (SPI2_SR & SPI_SR_TXE) == 0 )	|	/* Wait until transmit buffer is empty */
	     ( (SPI2_SR & SPI_SR_BSY) != 0 ) 	)	/* Then wait until busy goes away (see note p 673)*/
		return 1;
	return 0;
}
/******************************************************************************
 * void spi2_poi (void);
 * @brief	: Power on insertion: more than 74 SCLKs while /CS and DI held high
*******************************************************************************/
/*
Set DI and CS high and apply more than 74 clock pulses to SCLK and the card will go 
ready to accept native commands.
*/

#define NUMBERPOIBYTES	10	// Number of POI bytes to send
void spi2_poi (void)
{
	short spi2_poi_ct;		// Counter for sending POI bytes
	char cX = 0xff;
	spi2_poi_ct = NUMBERPOIBYTES;	// Counter for number bytes sent
	SDCARD_CS_hi			// Set /CS high. Macro is in ../devices/PODpinconfig.h
	for (spi2_poi_ct = 0; spi2_poi_ct < NUMBERPOIBYTES; spi2_poi_ct++)
	{	
		spi2_write (&cX, 1, 0);	// Send first byte
		while (xmit2_busy_flag != 0);	//
	}

	SDCARD_CS_low			// Set /CS low. Macro is in ../devices/PODpinconfig.h
	return;	// Here, return without initiating another write operation.
}
/******************************************************************************
 * int spi2_sclk_set_divisor_code (unsigned short usM);
 * @brief	: Change the baud rate divisor code
 * @param	: usM divisor code: 0 - 7.  Change is skipped for numbers outside range
 * @return	: zero = no Err; 1 = Busy, 2 = bad code number
*******************************************************************************/
int spi2_sclk_set_divisor_code (unsigned short usM)
{
	if (xmit2_busy_flag != 0 ) return 1;	// Check for busy
	if (usM > 7)	return 2;		// Check for bad divisor code
	SPI2_CR1 =  (SPI2_CR1 & ~(0x07<<3)) | (usM << 3); // Replace baud divisor
	return 0;
}
/******************************************************************************
 * unsigned int spi2_sclk_get_divisor_code (void);
 * @brief	: Get the current baud rate divisor code
 * @return	: Divisor code: 0 - 7
*******************************************************************************/
unsigned int spi2_sclk_get_divisor_code (void)
{
	return	(SPI2_CR1 >> 3) & 0x07;	// Get divisor code from registor
}
/*#######################################################################################
 * ISR routine: SPI/DMA receive
 *####################################################################################### */
void DMA1CH4_IRQHandler(void)
{ // Here, READ dma transfer complete interrupt
	if ( (DMA1_ISR & DMA_ISR_TCIF4) != 0 )	// Is this a transfer complete interrupt?
	{ // Here, yes.
		DMA1_IFCR = DMA_IFCR_CTCIF4;	// Clear transfer complete flag (p 208)
		DMA1_CCR4 &= ~0x01;		// Disable DMA1CH4 and transfer complete interrupt
		xmit2_busy_flag = 0;		// Show we are done with the write/read
		// Here this interrupt was for a spi read operation.  
		if (spi2_isrdoneptr != 0)	// Is there an address set to go to?
			(*spi2_isrdoneptr)();	// Yes, start the next step
	}
	return;
/* NOTE: For either read or write calls (above), the *dma* (dma1ch4, read) transfer complete interrupt
occurs when the spi has completed the last byte, so the spi and dma are now idle, i.e. not-busy.  The
spi xmit buffer goes empty when the last byte has been transfered to the shift register, but the receive buffer
flag comes on when the last byte has transfered to the receive buffer which in turn causes the last byte
of the DMA1CH4 to store and assert the interrupt flag.  */
}

