/*****************************************************************************
* File Name          : spi2rw.c
* Date First Issued  : 06/12/2011
* Board              : Discovery F4
* Description        : SPI2 routine for simultaneous read/write
*******************************************************************************/


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPI2RW
#define __SPI2RW

/* Includes ------------------------------------------------------------------*/


/* Note: The lower four bits of the priority byte are not used.
 The higher the priority number, the lower the priority */
#define SPI2_PRIORITY		0x50	// Interrupt priority for SPI2 interrupt

/******************************************************************************/
void spi2rw_init(void);
/*  @brief Initialize SPI 
*******************************************************************************/
unsigned short spi2_busy(void);
/* @brief	: Test if spi2 is busy
 * @return	: 0 = /CS line is low; not-zero (busy) = /CS line is high (not busy)
*******************************************************************************/
void spi2_rw (char *pout, char * pin, int count);
/* @brief	: Write and read bytes
 * @param	: char *pout = pointer to byte array with bytes to output
 * @param	: char *pin  = pointer to byte array to receive bytes coming in
 * @param	: int count  = byte count of number of write/read cycles
*******************************************************************************/

extern void (*spi2_readdoneptr)(void);	// Address of function to call upon completion of read

#endif 
