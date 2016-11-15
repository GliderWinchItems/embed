/******************************************************************************
* File Name          : spi1sad7799_ten.c
* Date First Issued  : 07/03/2015
* Board              : STM32F103VxT6_pod_mm
* Description        : SPI1 routines for AD7799_1 winch tension
*******************************************************************************/

#include "spi1sad7799_ten.h"
#include "PODpinconfig.h"
#include "pinconfig_all.h"
#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/gpio.h"
#include "libopenstm32/spi.h"
#include "libopenstm32/rcc.h"
#include "DTW_counter.h"

extern unsigned int sysclk_freq;	/* 	SYSCLK freq		E.g. 72000000	*/

char 	 spi1_ten_xmt;	// SPI1 dummy byte sent when reading
char	xmt_flag;	// Flag for ISR: 0 = xmit; not zero = rcv 
int 	 spi1_ten_cnt;	// SPI1 byte counter
unsigned char 	*spi1_ten_ptr;	// SPI1 buffer pointer
void 	(*spi1_ten_writedoneptr)(void);	// Address of function to call upon completion of write
void 	(*spi1_ten_readdoneptr)(void);	// Address of function to call upon completion of read

/* APB2 bus frequency is used for setting up the divider for SPI1 */
extern unsigned int	pclk2_freq;	/*	SYSCLKX/APB2 divider	E.g. 36000000 	*/
/* AD7799 minimum SCLK pulse width is 100ns, i.e. 5 MHz.  Set bus divider to stay at or below this freq */
#define SCLKFREQ	5000000	/* Max frequency (Hz) to be used with AD7799 */

unsigned char ad7799_selected = 0;	// Current /CS line selected: 0 = AD7799_1, 1 = AD7799_2
/******************************************************************************
 * void timedelay_usec (u32 ticks);
 * ticks = processor ticks to wait, using DTW_CYCCNT (32b) tick counter
 *******************************************************************************/
#define WAITDTW(tick)	while (( (int)tick ) - (int)(*(volatile unsigned int *)0xE0001004) > 0 )
static void timedelay_usec (unsigned int usec)
{
	unsigned int ticks = sysclk_freq/1000000; // Sysclk ticks per usec
	u32 t1 = (ticks * usec) + DTWTIME;	// Time to quit looping	
	WAITDTW(t1);				// Loop
}
/**************************************************************************
* void SPIsd_dummy(void);
* @brief	: Function called from the ISR that merely returns
**************************************************************************/
void SPIsd_dummy(void)
{
	return;
}
const struct PINCONFIGALL spi1_ncs1   = {(volatile u32 *)GPIOB, 10, OUT_PP   , MHZ_50};
const struct PINCONFIGALL spi1_ncs2   = {(volatile u32 *)GPIOA,  4, OUT_PP   , MHZ_50};
const struct PINCONFIGALL spi1_sck    = {(volatile u32 *)GPIOA,  5, OUT_AF_PP, MHZ_50};
const struct PINCONFIGALL spi1_so     = {(volatile u32 *)GPIOA,  6, IN_PU    ,      0};
const struct PINCONFIGALL spi1_si     = {(volatile u32 *)GPIOA,  7, OUT_AF_PP, MHZ_50};
/******************************************************************************
 * int spi1ad7799_ten_init(void);
 * @brief 	: Initialize SPI for AD7799
 * @return	: 0 = ok; not 0 badness and nashing of teeth
*******************************************************************************/
int spi1ad7799_ten_init(void)
{
	unsigned int uiX,uiZ,uiM;	// Used in computing baud divisor

	/* Set end of interrupt handling of read and write addresses */
	spi1_ten_writedoneptr = &SPIsd_dummy;	// Set a valid address to go to when write completes
	spi1_ten_readdoneptr  = &SPIsd_dummy;	// Set a valid address to go to when read completes

	/* Enable: SPI1 and  bus clocking for alternate function */
	RCC_APB2ENR |= ((RCC_APB2ENR_SPI1EN) | (RCC_APB2ENR_AFIOEN));

	/* Turn on switch for 3.3v digital power to AD7799(s) */
	ADC7799VCCSW_on		// gpio macro (macro is in PODpinconfig.h)

	/* PD10	5V regulator enable--Analog--strain guage & AD7799: gpio_out */
	STRAINGAUGEPWR_on	// 5v analog power must be on for digital part to work	(macro is in PODpinconfig.h)

	/* Analog power to AD7799 is required for digital side to work */
	ANALOGREG_on		// Turn switch on for 3.2v processor power 		(macro is in PODpinconfig.h)

	/* Wait 10 ms for power regulator to stabilize.  Code goes here */
	timedelay_usec(10000);	// usec delay

	/* Disable select for both AD7799s. */
	GPIOB_BSRR  = (1<<10);		// Disable  AD7799_1 (/CS pin (PB10) high)
	GPIOA_BSRR = (1<<4);		// Disable AD7799_2 (/CS pin (PA4) high)

	/* Configure gpio pins for SPI1 use */
	if ((pinconfig_all((struct PINCONFIGALL *)&spi1_ncs1)) != 0) return -1;	// /CS ad7799-1
	if ((pinconfig_all((struct PINCONFIGALL *)&spi1_ncs2)) != 0) return -2;	// /CS ad7799-2
	if ((pinconfig_all((struct PINCONFIGALL *)&spi1_sck) ) != 0) return -3;	// Clock out AF
	if ((pinconfig_all((struct PINCONFIGALL *)&spi1_so)  ) != 0) return -4;	// MISO
	if ((pinconfig_all((struct PINCONFIGALL *)&spi1_si)  ) != 0) return -5;	// MISO
	
	/* Compute baud divisor - (uiM = 0 for divide 2 to uiM = 7 for divide by 256)*/
	uiX = (pclk2_freq / SCLKFREQ) + 1;
	uiM = 0; uiZ = 2;
	while ( uiZ < uiX){ uiM += 1; uiZ *= 2; } // Find divisor that exceeds uiX division ratio
	uiM += 1;

	if (uiM > 7) uiM = 7;	// Greater than 7 means the SCLK speed may be higher than spec.

	/* SPI-CR1 (see p 693 Ref Manual) */
	//              SSM   |enable periph| baud divisor | master select | CK 1 when idle    | phase  )
	SPI1_CR1 = (0x3 << 8) | (1 << 6)    | (uiM << 3)   |   (1 << 2)    |    (1 << 1)       |  0x01    ;
	
	/* SPI-CR2 use default, no interrupt masks enabled at this point */

	/* Set and enable interrupt controller for SPI1 */
	NVICIPR (NVIC_SPI1_IRQ, SPI1_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_SPI1_IRQ);			// Enable interrupt controller for SPI1

	return 0;
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
 * unsigned short ad7799_2_ready(void);
 * @brief	: Test ad7799_2 data out line low (data is ready)
 * @return	: Zero = ready; not-zero = data line still high
*******************************************************************************/
unsigned short ad7799_2_ready(void)
{
	return (GPIOA_IDR & GPIO4);	// Return bit 10 of port B
}
/******************************************************************************
 * void ad7799_1_select(void);
 * @brief	: Select #1, deselect #2
*******************************************************************************/
void ad7799_1_select(void)
{
	GPIOB_BRR  = (1<<10);		// Enable  AD7799_1 (/CS pin (PB10) low)
	GPIOA_BSRR = (1<<4);		// Disable AD7799_2 (/CS pin (PA4) high)
//	SPI1_CR1 &= ~(0x3<<8);		// SSI OFF: Disable AD7799_2
	ad7799_selected = 0;		// Show current selection is AD7799_1
	return;
}
/******************************************************************************
 * void ad7799_2_select(void);
 * @brief	: Select #2, deselect #1
*******************************************************************************/
void ad7799_2_select(void)
{
	GPIOB_BSRR = (1<<10);		// Disable  AD7799_1 (/CS pin (PB10) high)
	GPIOA_BRR  = (1<<4);		// Enable AD7799_2 (/CS pin (PA4) low)
//	SPI1_CR1 |=  (0x3<<8);		// SSI ON: Enable AD7799_2 (/CS pin (PA4) low)
	ad7799_selected = 1;		// Show current selection is AD7799_2
	return;
}

/******************************************************************************
 * void spi1_ten_read (unsigned char *p, int count, char xmit);
 * @brief	: read 'count' bytes, into buffer 'p'
 * @param	: char *p = pointer to byte buffer
 * @param	: int count = byte count to be read (i.e. number of "xmit/read cycles"
 * @param	: char xmit  = outbound char during spi cycle
*******************************************************************************/
void spi1_ten_read (unsigned char *p, int count, char xmit)
{
	/* The following should not be necessary, but it is here JIC. */
	while ( spi1_ten_busy() != 0 );	// Loop until spi communication is complete

	xmt_flag = 1;			// Show ISR this is a rcv sequence
	spi1_ten_ptr = p;		// Set pointer for interrupt handler to store incoming data
	spi1_ten_cnt = count;		// Set byte count for interrupt handler
	spi1_ten_xmt = SPI1_DR;		// Clear last recieve buffer full flag
	SPI1_DR  = xmit;		// Start 1st xmit dummy byte to get 1st receive byte
	spi1_ten_xmt = xmit;		// Save dummy byte that is sent for subsequent reads
	SPI1_CR2 |= (SPI_CR2_RXNEIE);	// Turn on receive buffer loaded interrupt enable
	return;
}
/******************************************************************************
 * void spi1_ten_write (unsigned char *p, int count);
 * @brief	: read 'count' bytes, into buffer 'p'
 * @param	: char *p = pointer to byte buffer
 * @param	: int count = byte count to be read (i.e. number of "xmit/read cycles"
*******************************************************************************/
void spi1_ten_write (unsigned char *p, int count)
{
	/* If the programmer bozo didn't check for busy we have no choice but to loop */
	while ( spi1_ten_busy() != 0 );	// Loop until operation complete
	spi1_ten_ptr = p;		// Set pointer for interrupt handler
	spi1_ten_cnt = count;		// Set byte count for interrupt handler
	xmt_flag = 0;			// Show ISR this is a xmit sequence
	SPI1_CR2 |= (SPI_CR2_TXEIE);	// Turn on xmit buffer empty interrupt enable
	/* At this point the xmit buffer interrupt will pick up TXE interrupt and send the first byte */
	return;
}
/******************************************************************************
 * char spi1_ten_busy (void);
 * @brief	: Check for buffer bytes remaining and spi1 busy bit 
 * @return	: not zero means busy
*******************************************************************************/
char spi1_ten_busy (void)
{
	return ( spi1_ten_cnt | ((SPI1_SR & SPI_SR_BSY) != 0) );
}
/******************************************************************************
 * void spi1_ten_ad7799_reset_noint (void);
 * @brief	: Non-interrupting (test) for reseting the AD7799
*******************************************************************************/
void spi1_ten_ad7799_reset_noint (void)
{
	__attribute__((__unused__))int dummy;
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
void SPI1_TEN_IRQHandler(void)
{
		 __attribute__((__unused__))unsigned int dummy;

	if (xmt_flag == 0)
	{
		if ( ((SPI1_CR2 & SPI_CR2_TXEIE) != 0) && ((SPI1_SR & SPI_SR_TXE) != 0) )	
		{ /* Here, this is a xmit sequence being executed AND transmit buffer empty flag is on */
			if (spi1_ten_cnt <= 0)	// Have we exhausted the count?
			{ /* Here, yes, the last byte was loaded into xmit buffer the last time the ISR executed */
				SPI1_CR2 &= ~SPI_CR2_TXEIE;	// Turn off xmit buffer empty interrupt enable
				dummy = SPI1_DR;		// Clear receive buffer full flag
				SPI1_CR2 |=  SPI_CR2_RXNEIE;	// Enable receive buffer full interrupt		
				return;
			}
			else
			{ /* Here, the byte count shows there is more to do */
				SPI1_DR = *spi1_ten_ptr++;	// Load next byte for xmit
				spi1_ten_cnt -= 1; 		// Decrement byte count
				return;
			}
		}
		else
		{ /* Here, the expected interrupt is a receive buffer full, which was loaded from the last byte of the xmit sequence */
			if ( ((SPI1_CR2 & SPI_CR2_RXNEIE) != 0) && ((SPI1_SR & SPI_SR_RXNE) != 0) ) // Check for bogus interrupt
			{ /* Here, valid interrupt.  Xmit sequence should be completed, including the last byte sending finished */
				SPI1_CR2 &= ~SPI_CR2_RXNEIE;	// Turn off RXE interrupt enable
				(*spi1_ten_writedoneptr)();		// Go do next SPI1 step (see note 1)
			}
		}
	}
	else
	{ /* Here, a receive sequence being executed */
		/* Here, we are doing a receive */
		if ( ((SPI1_CR2 & SPI_CR2_RXNEIE) != 0) && ((SPI1_SR & SPI_SR_RXNE) != 0) ) // Check for bogus interrupt
		{ /* Here, valid interrupt. */
			*spi1_ten_ptr++ = SPI1_DR;		// Get byte that was read
			spi1_ten_cnt -= 1;	 		// Decrement byte count
			if (spi1_ten_cnt <= 0)		// Have we exhausted the count?
			{ /* Here, yes, this byte is the reponse from the last dummy byte transmitted */ 
				SPI1_CR2 &= ~SPI_CR2_RXNEIE;	// Turn off RXE interrupt enable
				(*spi1_ten_readdoneptr)();	// Go do next SPI1 step (see note 1)
				return;
			}
			else
			{
				SPI1_DR = spi1_ten_xmt;	// Load dummy byte to start next spi cycle
			}
		}
	}
	return;
}
/* 
Note 1: At this point the last byte in the shift register has been sent therefore loading the receiver buffer.  A call to a routine to do the next step 
is made using a pointer to the function.  The function called might call spi1_ten_read, or spi1_ten_write to start the next operation.  The 'busy' flag should be
off since the last char was completed.  Consequently, the routine setting up the next step should be have to loop waiting for the busy.
*/

