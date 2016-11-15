/*****************************************************************************
* File Name          : spi1rwB.h
* Date First Issued  : 01/21/2015
* Board              : Discovery F4
* Description        : SPI1 routine for simultaneous read/write
*******************************************************************************/


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPI1RWB
#define __SPI1RWB

/* Includes ------------------------------------------------------------------*/


/* Note: The lower four bits of the priority byte are not used.
 The higher the priority number, the lower the priority */
#define SPI1_PRIORITY		0x50	// Interrupt priority for SPI1 interrupt

/******************************************************************************/
void spi1rw_initB(void);
/*  @brief Initialize SPI 
*******************************************************************************/
unsigned short spi1_busyB(void);
/* @brief	: Test if spi1 is busy
 * @return	: 0 = /CS line is low; not-zero (busy) = /CS line is high (not busy)
*******************************************************************************/
void spi1_rwB(char *pout, char * pin, int count);
/* @brief	: Write and read bytes
 * @param	: char *pout = pointer to byte array with bytes to output
 * @param	: char *pin  = pointer to byte array to receive bytes coming in
 * @param	: int count  = byte count of number of write/read cycles
*******************************************************************************/

extern void (*spi1_readdoneptrB)(void);	// Address of function to call upon completion of read

#endif 
