/*****************************************************************************
* File Name          : spi2rwB.c
* Date First Issued  : 06/12/2011
* Board              : Discovery F4
* Description        : SPI2 routine for simultaneous read/write
*******************************************************************************/
/*
02-06-2014 rev 169: debugged with 'scope, and MOSI-MISO loopback and spi2test.c

*/
#include "nvicdirect.h"
#include "libopencm3/stm32/nvic.h"
#include "libopencm3/stm32/f4/rcc.h"
#include "libopencm3/stm32/spi.h"
#include "DISCpinconfig.h"	// Pin configuration for STM32 Discovery board
#include "spi2rw.h"

/* Pointers and counter used during the transfer */
static char* 	spi2_outptr;	// Pointer to outgoing array
static char* 	spi2_inptr;	// Pointer to incoming array
static char* 	spi2_outptr_end;	// Pointer to outgoing array
static char* 	spi2_inptr_end;	// Pointer to incoming array

void 	(*spi2_readdoneptrB)(void);	// Address of function to call upon completion of read

/* Pin configuration to use pin as gp output.  For the options see--
'../svn_discoveryf4/sw_discoveryf4/trunk/lib/libopencm3/stm32/f4/gpio.h' */

/* Pin configurations for SPI */

// Chip select line for shift registers
static const struct PINCONFIG	outputcs = { \
	GPIO_MODE_OUTPUT,	// mode: output 
	GPIO_OTYPE_PP, 		// output type: push-pull 		
	GPIO_OSPEED_100MHZ, 	// speed: highest drive level
	GPIO_PUPD_NONE, 	// pull up/down: none
	0 };			// Alternate function code: not applicable

//  SPI input pin configuration (MISO)  */
static const struct PINCONFIG	inputaf = { \
	GPIO_MODE_AF,		// mode: Input alternate function 
	0, 			// output type: not applicable 		
	0, 			// speed: not applicable
	GPIO_PUPD_PULLUP, 	// pull up/down: pullup 
	GPIO_AF5 };		// AFRLy & AFRHy selection

//  SPI output pin configuration (SCK, MOSI) */
static const struct PINCONFIG	outputaf = { \
	GPIO_MODE_AF, 		// mode: Output alternate function
	GPIO_OTYPE_PP, 		// output type: push pull	
	GPIO_OSPEED_100MHZ, 	// speed: fastest 
	GPIO_PUPD_NONE, 	// pull up/down: none
	GPIO_AF5 };		// AFRLy & AFRHy selection

/******************************************************************************
 * void spi2rw_initB(void);
 *  @brief Initialize SPI 
*******************************************************************************/ 
void spi2rw_initB(void)
{
	/* Enable bus clocking for SPI2 */
	RCC_APB1ENR |= (1<<14);	// Enable SPI2 clocking

	/* Set up pins for SPI2 use. */
	f4gpiopins_Config ((volatile u32*)GPIOB, 12, (struct PINCONFIG*)&outputcs);	// CS
	f4gpiopins_Config ((volatile u32*)GPIOB, 13, (struct PINCONFIG*)&outputaf);	// SCK
	f4gpiopins_Config ((volatile u32*)GPIOB, 14, (struct PINCONFIG*)&inputaf);	// MISO
	f4gpiopins_Config ((volatile u32*)GPIOB, 15, (struct PINCONFIG*)&outputaf);	// MOSI

	GPIO_BSRR(GPIOB)  = (1<<12);// Set /CS high

	// Set divisor to max.  If APB1 is 42 Mhz, then divide by 256 = 164062.5 Hz, 48 us per byte
/* NOTE: The following line is where the "phase" is set for the clock and polarity */
	//          (SSM SSI)  |enable peripheral | baud divisor | master select | CK 1 when idle    | phase  )
	SPI2_CR1 =  (0x3 << 8) |   (1 << 6)       | (0x7 << 3)   |   (1 << 2)    |    (1 << 1)       |  0x01    ;
	
	/* SPI-CR2 use default, no interrupt masks enabled at this point */

	/* Set and enable interrupt controller for SPI2 */
	NVICIPR (NVIC_SPI2_IRQ, SPI2_PRIORITY );	// Set interrupt priority
	NVICISER(NVIC_SPI2_IRQ);			// Enable interrupt controller for SPI2

	return;
}
/******************************************************************************
 * unsigned short spi2_busyB(void);
 * @brief	: Test if spi2 is busy
 * @return	: 0 = /CS line is low; not-zero (busy) = /CS line is high (not busy)
*******************************************************************************/
unsigned short spi2_busyB(void)
{
/* The /CS line is used to show if the SPI transfer is in progress */
	return (GPIOB_ODR & (1<<12));	// Return /CS bit
}

/******************************************************************************
 * void spi2_rwB (char *pout, char * pin, int count);
 * @brief	: Initiate a transfer for: Write and read bytes
 * @param	: char *pout = pointer to byte array with bytes to output
 * @param	: char *pin  = pointer to byte array to receive bytes coming in
 * @param	: int count  = byte count of number of write/read cycles
*******************************************************************************/
void spi2_rwB (char *pout, char * pin, int count)
{
	/* The following should not be necessary, but it is here JIC. */
	while ( spi2_busyB() == 0 );	// Loop until a prior spi communication is complete

	GPIO_BSRR(GPIOB)  = (1<<(12+16));	// Set /CS low (show busy)
	spi2_outptr = pout;		// Set pointer for interrupt handler to store outgoing data
	spi2_outptr_end = (pout + count);
	spi2_inptr = pin;		// Set pointer for interrupt handler to store incoming data
	spi2_inptr_end = (pin + count);
	SPI2_CR2 |= ((SPI_CR2_RXNEIE) | (SPI_CR2_TXEIE));	// Enable TX & RX interrupts
	return;
}
/*#######################################################################################
 * ISR routine
 *####################################################################################### */
void SPI2_IRQHandlerB(void)
{      
/* 
The read-side (read buffer not empty, i.e. loaded) causes the interrupt.  The byte is read
from the register which resets the interrupt flag.  If there are more bytes to transfer the
tx data register is loaded with the next outgoing byte.  

When the count of bytes to transfer has been exhausted the read interrupt enable is turned
off (not really necessary), and the i/o pin with the CS line is brought back high. 
*/

	__attribute__ ((unused))  unsigned int dummy;


	if (((SPI2_SR & SPI_SR_RXNE) != 0) && ((SPI2_CR2 & SPI_CR2_RXNEIE) != 0))  // Check for bogus interrupt
	{ /* Here, valid receive interrupt. */
		*spi2_inptr++ = SPI2_DR;	// Get byte that was read
		if (spi2_inptr >= spi2_inptr_end)		// Have we exhausted the count?
		{ /* Here, yes, this byte is the reponse from the last byte transmitted */ 
			SPI2_CR2 &= ~SPI_CR2_RXNEIE;	// Turn off RXE interrupt enable
			GPIO_BSRR(GPIOB)  = (1<<12);// Set /CS high (show not busy)
			if (spi2_readdoneptrB != 0) // Skip if pointer is NULL
				(*spi2_readdoneptrB)();	// In case we want to do something else
			return;
		}
	}
	if ( ((SPI2_SR & SPI_SR_TXE) != 0) && ((SPI2_CR2 & SPI_CR2_TXEIE) != 0) ) // Check for bogus interrupt
	{ /* Here, transmit buffer empty interrupt. */
		SPI2_DR = *spi2_outptr++;	// Load outgoing byte to start next spi cycle
		if (spi2_outptr >= spi2_outptr_end)		// Have we exhausted the count?
		{ /* Here, yes, this byte is the reponse from the last byte transmitted */ 
			SPI2_CR2 &= ~SPI_CR2_TXEIE;	// Turn off RXE interrupt enable
			return;
		}
	}
	dummy = SPI2_SR; // Prevent tail-chaining.
	return;
}

