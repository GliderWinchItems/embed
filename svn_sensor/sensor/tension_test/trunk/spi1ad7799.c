/******************************************************************************
* File Name          : spi1sad7799_ten.c
* Date First Issued  : 07/03/2015
* Board              : STM32F103VxT6_pod_mm
* Description        : SPI1 routines for AD7799_1 winch tension
*******************************************************************************/

#include "spi1ad7799_ten.h"
#include "PODpinconfig.h"
#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/gpio.h"
#include "libopenstm32/spi.h"
#include "libopenstm32/rcc.h"



char 	 spi1_xmt;	// SPI1 dummy byte sent when reading
char	xmt_flag;	// Flag for ISR: 0 = xmit; not zero = rcv 
int 	 spi1_cnt;	// SPI1 byte counter
unsigned char 	*spi1_ptr;	// SPI1 buffer pointer
void 	(*spi1_writedoneptr)(void);	// Address of function to call upon completion of write
void 	(*spi1_readdoneptr)(void);	// Address of function to call upon completion of read

/* APB2 bus frequency is used for setting up the divider for SPI1 */
extern unsigned int	pclk2_freq;	/*	SYSCLKX/APB2 divider	E.g. 36000000 	*/
/* AD7799 minimum SCLK pulse width is 100ns, i.e. 5 MHz.  Set bus divider to stay at or below this freq */
#define SCLKFREQ	5000000	/* Max frequency (Hz) to be used with AD7799 */

/**************************************************************************
* void SPIsd_dummy(void);
* @brief	: Function called from the ISR that merely returns
**************************************************************************/
void SPIsd_dummy(void)
{
	return;
}
/******************************************************************************
 * void spi1ad7799_ten_init(void);
 *  @brief Initialize SPI for SD Card Adapater
*******************************************************************************/
void spi1ad7799_ten_init(void)
{
	unsigned int uiX,uiZ,uiM;	// Used in computing baud divisor

	/* Set end of interrupt handling of read and write addresses */
	spi1_writedoneptr = &SPIsd_dummy;	// Set a valid address to go to when write completes
	spi1_readdoneptr  = &SPIsd_dummy;	// Set a valid address to go to when read completes

	/* Enable bus clocking for SPI1 */
	RCC_APB2ENR |= RCC_APB2ENR_SPI1EN;	// (p 103)

	/* Enable bus clocking for alternate function */
	RCC_APB2ENR |= (RCC_APB2ENR_AFIOEN);		// (p 103) 

	//  PB10 - AD7799_1 /CS: gpio in, float.  AD7799 DOUT pulls this high or low for /READY.
	GPIO_CRH(GPIOB) &= ~((0x000f ) << (4*2));	// Clear CNF reset bit 01 = Floating input (reset state)
	GPIO_CRH(GPIOB) |=  (( (GPIO_CNF_INPUT_FLOAT) | (GPIO_MODE_INPUT) ) << (4*2));	

	/* Turn on switch for 3.3v digital power to AD7799(s) */
	ADC7799VCCSW_on		// gpio macro (macro is in PODpinconfig.h)

	/* PD10	5V regulator enable--Analog--strain guage & AD7799: gpio_out */
	STRAINGAUGEPWR_on	// 5v analog power must be on for digital part to work	(macro is in PODpinconfig.h)

	/* Analog power to AD7799 is required for digital side to work */
	ANALOGREG_on		// Turn switch on for 3.2v processor power 		(macro is in PODpinconfig.h)

	/* Wait 10 ms for power regulator to stabilize.  Code goes here */

	/* Configure gpio pins for SPI1 use */
	// Clear CNF reset bit 01 = Floating input (reset state)
		// PA4	AD7799_2 /CS:	gpio_out	
		// PA5  AD7799(s) SCK :	SPI1_SCK  
		// PA6	AD7799(s) DOUT/RDY:SPI1_MISO 
		// PA7	AD7799(s) DIN:	SPI1_MOSI 
	GPIO_CRL(GPIOA) &= 0x0000ffff ;	// Mask out CNF & MODE bits for PA4-PA7

	// In/Out and mode bits
		// PA4	AD7799_2 /CS:	gpio_out	
		// PA5  AD7799(s) SCK :	SPI1_SCK  
		// PA6	AD7799(s) DOUT/RDY:SPI1_MISO 
		// PA7	AD7799(s) DIN:	SPI1_MOSI 
	GPIO_CRL(GPIOA) |= ((( (GPIO_CNF_OUTPUT_PUSHPULL       <<2) | (GPIO_MODE_OUTPUT_50_MHZ) ) << (4*4)) \
			 |  (( (GPIO_CNF_OUTPUT_ALTFN_PUSHPULL <<2) | (GPIO_MODE_OUTPUT_50_MHZ) ) << (4*5)) \
			 |  (( (GPIO_CNF_INPUT_FLOAT           <<2) | (GPIO_MODE_INPUT        ) ) << (4*6)) \
			 |  (( (GPIO_CNF_OUTPUT_ALTFN_PUSHPULL <<2) | (GPIO_MODE_OUTPUT_50_MHZ) ) << (4*7)));

	/* Note: PB10	AD7799_1 /CS:gpio_out and PA4	AD7799_2 /CS:	gpio_out was setup by earlier call to PODgpiopins_Config() */
	
	/* Compute baud divisor - (uiM = 0 for divide 2 to uiM = 7 for divide by 256)*/
	uiX = (pclk2_freq / SCLKFREQ) + 1;
	uiM = 0; uiZ = 2;
	while ( uiZ < uiX){ uiM += 1; uiZ *= 2; } // Find divisor that exceeds uiX division ratio
	uiM += 1;
uiM += 1;	// Debug

	if (uiM > 7) uiM = 7;	// Greater than 7 means the SCLK speed may be higher than spec.

	/* SPI-CR1 (see p 693 Ref Manual) */
	//         (enable peripheral| baud divisor | master select | CK 1 when idle    | phase  )
	SPI1_CR1 =       (1 << 6)    | (uiM << 3)   |   (1 << 2)    |    (1 << 1)       |  0x01    ;
	
	/* Enable AD7799_1; disable AD7799_2 (if present) */
	AD7799_1_CS_low		// Enable AD7799_1 (not chip select low) 		(macro is in PODpinconfig.h)
	AD7799_2_CS_hi		// This is routoi	(macro is in PODpinconfig.h)

	/* SPI-CR2 use default, no interrupt masks enabled at this point */

	/* Set and enable interrupt controller for SPI1 */
	NVICIPR (NVIC_SPI1_IRQ, SPI1_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_SPI1_IRQ);			// Enable interrupt controller for SPI1

	return;
}
/******************************************************************************
 * unsigned short ad7799_1_ready(void);
 * @brief	: Test ad7799_1 data out line low (data is ready)
 * @return	: Zero = ready; not-zero = data line still high
*******************************************************************************/
unsigned short ad7799_1_ready(void)
{
	return (GPIOB_IDR & GPIO10);	// Return bit 10 of port B
}

/******************************************************************************
 * void spi1_read (unsigned char *p, int count, char xmit);
 * @brief	: read 'count' bytes, into buffer 'p'
 * @param	: char *p = pointer to byte buffer
 * @param	: int count = byte count to be read (i.e. number of "xmit/read cycles"
 * @param	: char xmit  = outbound char during spi cycle
*******************************************************************************/
void spi1_read (unsigned char *p, int count, char xmit)
{
	/* The following should not be necessary, but it is here JIC. */
	while ( spi1_busy() != 0 );	// Loop until spi communication is complete

	xmt_flag = 1;			// Show ISR this is a rcv sequence
	spi1_ptr = p;			// Set pointer for interrupt handler to store incoming data
	spi1_cnt = count;		// Set byte count for interrupt handler
	spi1_xmt = SPI1_DR;		// Clear last recieve buffer full flag
	SPI1_DR  = xmit;		// Start 1st xmit dummy byte to get 1st receive byte
	spi1_xmt = xmit;		// Save dummy byte that is sent for subsequent reads
	SPI1_CR2 |= (SPI_CR2_RXNEIE);	// Turn on receive buffer loaded interrupt enable
	return;
}
/******************************************************************************
 * void spi1_write (unsigned char *p, int count);
 * @brief	: read 'count' bytes, into buffer 'p'
 * @param	: char *p = pointer to byte buffer
 * @param	: int count = byte count to be read (i.e. number of "xmit/read cycles"
*******************************************************************************/
void spi1_write (unsigned char *p, int count)
{
	/* If the programmer bozo didn't check for busy we have no choice but to loop */
	while ( spi1_busy() != 0 );	// Loop until operation complete
	spi1_ptr = p;			// Set pointer for interrupt handler
	spi1_cnt = count;		// Set byte count for interrupt handler
	xmt_flag = 0;			// Show ISR this is a xmit sequence
	SPI1_CR2 |= (SPI_CR2_TXEIE);	// Turn on xmit buffer empty interrupt enable
	/* At this point the xmit buffer interrupt will pick up TXE interrupt and send the first byte */
	return;
}
/******************************************************************************
 * char spi1_busy (void);
 * @brief	: Check for buffer bytes remaining and spi1 busy bit 
 * @return	: not zero means busy
*******************************************************************************/
char spi1_busy (void)
{
	return ( spi1_cnt | ((SPI1_SR & SPI_SR_BSY) != 0) );
}
/******************************************************************************
 * void spi1_ad7799_reset_noint (void);
 * @brief	: Non-interrupting (test) for reseting the AD7799
*******************************************************************************/
void spi1_ad7799_reset_noint (void)
{
	int dummy;
	SPI1_DR  = 0xff;		// Start xmit
	while ( (SPI1_SR & SPI_SR_TXE) == 0 );// Wait for TX buffer to be empty
	SPI1_DR  = 0xff;		// Start xmit
	while ( (SPI1_SR & SPI_SR_TXE) == 0 );// Wait for TX buffer to be empty
	SPI1_DR  = 0xff;		// Start xmit
	while ( (SPI1_SR & SPI_SR_TXE) == 0 );// Wait for TX buffer to be empty
	SPI1_DR  = 0xff;		// Start xmit
	while ( (SPI1_SR & SPI_SR_TXE) == 0 );// Wait for TX buffer to be empty
	while ( (SPI1_SR & SPI_SR_BSY) != 0 );
	while ( (SPI1_SR & SPI_SR_RXNE) == 1 )	dummy = SPI1_DR;// Clear read buffer

	return;
}
/*#######################################################################################
 * ISR routine
 *####################################################################################### */
void SPI1_IRQHandler(void)
{
	unsigned int dummy;

	if (xmt_flag == 0)
	{
		if ( ((SPI1_CR2 & SPI_CR2_TXEIE) != 0) && ((SPI1_SR & SPI_SR_TXE) != 0) )	
		{ /* Here, this is a xmit sequence being executed AND transmit buffer empty flag is on */
			if (spi1_cnt <= 0)	// Have we exhausted the count?
			{ /* Here, yes, the last byte was loaded into xmit buffer the last time the ISR executed */
				SPI1_CR2 &= ~SPI_CR2_TXEIE;	// Turn off xmit buffer empty interrupt enable
				dummy = SPI1_DR;		// Clear receive buffer full flag
				SPI1_CR2 |=  SPI_CR2_RXNEIE;	// Enable receive buffer full interrupt		
				return;
			}
			else
			{ /* Here, the byte count shows there is more to do */
				SPI1_DR = *spi1_ptr++;	// Load next byte for xmit
				spi1_cnt -= 1; 		// Decrement byte count
				return;
			}
		}
		else
		{ /* Here, the expected interrupt is a receive buffer full, which was loaded from the last byte of the xmit sequence */
			if ( ((SPI1_CR2 & SPI_CR2_RXNEIE) != 0) && ((SPI1_SR & SPI_SR_RXNE) != 0) ) // Check for bogus interrupt
			{ /* Here, valid interrupt.  Xmit sequence should be completed, including the last byte sending finished */
				SPI1_CR2 &= ~SPI_CR2_RXNEIE;	// Turn off RXE interrupt enable
				(*spi1_writedoneptr)();		// Go do next SPI1 step (see note 1)
			}
		}
	}
	else
	{ /* Here, a receive sequence being executed */
		/* Here, we are doing a receive */
		if ( ((SPI1_CR2 & SPI_CR2_RXNEIE) != 0) && ((SPI1_SR & SPI_SR_RXNE) != 0) ) // Check for bogus interrupt
		{ /* Here, valid interrupt. */
			*spi1_ptr++ = SPI1_DR;		// Get byte that was read
			spi1_cnt -= 1;	 		// Decrement byte count
			if (spi1_cnt <= 0)		// Have we exhausted the count?
			{ /* Here, yes, this byte is the reponse from the last dummy byte transmitted */ 
				SPI1_CR2 &= ~SPI_CR2_RXNEIE;	// Turn off RXE interrupt enable
				(*spi1_readdoneptr)();	// Go do next SPI1 step (see note 1)
				return;
			}
			else
			{
				SPI1_DR = spi1_xmt;	// Load dummy byte to start next spi cycle
			}
		}
	}
	return;
}
/* 
Note 1: At this point the last byte in the shift register has been sent therefore loading the receiver buffer.  A call to a routine to do the next step 
is made using a pointer to the function.  The function called might call spi1_read, or spi1_write to start the next operation.  The 'busy' flag should be
off since the last char was completed.  Consequently, the routine setting up the next step should be have to loop waiting for the busy.
*/

