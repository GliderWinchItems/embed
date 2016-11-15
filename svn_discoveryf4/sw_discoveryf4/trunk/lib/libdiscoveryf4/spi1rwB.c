/*****************************************************************************
* File Name          : spi1rwB.c
* Date First Issued  : 06/12/2011
* Board              : Discovery F4
* Description        : SPI1 routine for simultaneous read/write (MEMs accelerometer)
*******************************************************************************/
/*
02-06-2014 rev 169: debugged with 'scope, and MOSI-MISO loopback and spi1test.c
01-20-2015 Hacke spi2rwB.ch for spi1 and board MEMs accelerometer.

*/
#include "nvicdirect.h"
#include "libopencm3/stm32/nvic.h"
#include "libopencm3/stm32/f4/rcc.h"
#include "libopencm3/stm32/spi.h"
#include "DISCpinconfig.h"	// Pin configuration for STM32 Discovery board
#include "spi1rwB.h"

/* Pointers and counter used during the transfer */
static char* 	spi1_outptr;	// Pointer to outgoing array
static char* 	spi1_inptr;	// Pointer to incoming array
static char* 	spi1_outptr_end;	// Pointer to outgoing array
static char* 	spi1_inptr_end;	// Pointer to incoming array

void 	(*spi1_readdoneptrB)(void);	// Address of function to call upon completion of read

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
 * void spi1rw_initB(void);
 *  @brief Initialize SPI 
*******************************************************************************/ 
void spi1rw_initB(void)
{
	/* Enable bus clocking for SPI1 */
	RCC_APB2ENR |= (1<<12);	// Enable SPI1 clocking

	/* Set up pins for SPI1 use. */
	f4gpiopins_Config ((volatile u32*)GPIOE,  3, (struct PINCONFIG*)&outputcs);	// CS
	f4gpiopins_Config ((volatile u32*)GPIOA,  5, (struct PINCONFIG*)&outputaf);	// SCK
	f4gpiopins_Config ((volatile u32*)GPIOA,  6, (struct PINCONFIG*)&inputaf);	// MISO
	f4gpiopins_Config ((volatile u32*)GPIOA,  7, (struct PINCONFIG*)&outputaf);	// MOSI

	GPIO_BSRR(GPIOE)  = (1<<3);// Set /CS high

	// Set divisor to max.  If APB2 is 84 Mhz, then divide by 16 = 5.25 MHz, 1.52 us per byte
/* NOTE: The following line is where the "phase" is set for the clock and polarity */
	//          (SSM SSI)  |enable peripheral | baud divisor | master select | CK 1 when idle    | phase  )
	SPI1_CR1 =  (0x3 << 8) |   (1 << 6)       | (0x3 << 3)   |   (1 << 2)    |    (1 << 1)       |  0x01    ;
	
	/* SPI-CR2 use default, no interrupt masks enabled at this point */

	/* Set and enable interrupt controller for SPI1 */
	NVICIPR (NVIC_SPI1_IRQ, SPI1_PRIORITY);	// Set interrupt priority
	NVICISER(NVIC_SPI1_IRQ);			// Enable interrupt controller for SPI1

	return;
}
/******************************************************************************
 * unsigned short spi1_busyB(void);
 * @brief	: Test if spi1 is busy
 * @return	: 0 = /CS line is low; not-zero (busy) = /CS line is high (not busy)
*******************************************************************************/
unsigned short spi1_busyB(void)
{
/* The /CS line is used to show if the SPI transfer is in progress */
	return (GPIOE_ODR & (1<<3));	// Return /CS bit
}

/******************************************************************************
 * void spi1_rwB (char *pout, char * pin, int count);
 * @brief	: Initiate a transfer for: Write and read bytes
 * @param	: char *pout = pointer to byte array with bytes to output
 * @param	: char *pin  = pointer to byte array to receive bytes coming in
 * @param	: int count  = byte count of number of write/read cycles
*******************************************************************************/
void spi1_rwB (char *pout, char * pin, int count)
{
	/* The following should not be necessary, but it is here JIC. */
	while ( spi1_busyB() == 0 );	// Loop until a prior spi communication is complete

	GPIO_BSRR(GPIOE)  = (1<<(3+16));	// Set /CS low (show busy)
	spi1_outptr = pout;		// Set pointer for interrupt handler to store outgoing data
	spi1_outptr_end = (pout + count);
	spi1_inptr = pin;		// Set pointer for interrupt handler to store incoming data
	spi1_inptr_end = (pin + count);
	SPI1_CR2 |= ((SPI_CR2_RXNEIE) | (SPI_CR2_TXEIE));	// Enable TX & RX interrupts
	return;
}
/*#######################################################################################
 * ISR routine
 *####################################################################################### */
void SPI1_IRQHandlerB(void)
{      
/* 
The read-side (read buffer not empty, i.e. loaded) causes the interrupt.  The byte is read
from the register which resets the interrupt flag.  If there are more bytes to transfer the
tx data register is loaded with the next outgoing byte.  

When the count of bytes to transfer has been exhausted the read interrupt enable is turned
off (not really necessary), and the i/o pin with the CS line is brought back high. 
*/

	__attribute__ ((unused))  unsigned int dummy;


	if (((SPI1_SR & SPI_SR_RXNE) != 0) && ((SPI1_CR2 & SPI_CR2_RXNEIE) != 0))  // Check for bogus interrupt
	{ /* Here, valid receive interrupt. */
		*spi1_inptr++ = SPI1_DR;	// Get byte that was read
		if (spi1_inptr >= spi1_inptr_end)		// Have we exhausted the count?
		{ /* Here, yes, this byte is the reponse from the last byte transmitted */ 
			SPI1_CR2 &= ~SPI_CR2_RXNEIE;	// Turn off RXE interrupt enable
			GPIO_BSRR(GPIOE)  = (1<<3);// Set /CS high (show not busy)
			if (spi1_readdoneptrB != 0) // Skip if pointer is NULL
				(*spi1_readdoneptrB)();	// In case we want to do something else
			return;
		}
	}
	if ( ((SPI1_SR & SPI_SR_TXE) != 0) && ((SPI1_CR2 & SPI_CR2_TXEIE) != 0) ) // Check for bogus interrupt
	{ /* Here, transmit buffer empty interrupt. */
		SPI1_DR = *spi1_outptr++;	// Load outgoing byte to start next spi cycle
		if (spi1_outptr >= spi1_outptr_end)		// Have we exhausted the count?
		{ /* Here, yes, this byte is the reponse from the last byte transmitted */ 
			SPI1_CR2 &= ~SPI_CR2_TXEIE;	// Turn off RXE interrupt enable
			return;
		}
	}
	dummy = SPI1_SR; // Prevent tail-chaining.
	return;
}

