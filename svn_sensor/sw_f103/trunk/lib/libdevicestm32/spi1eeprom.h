/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : spi1eeprom.h
* Authorship         : deh
* Date First Issued  : 05/25/2013
* Board              : ../svn_sensor/hw/trunk/eagle/f103R/RxT6
* Description        : SPI1 routine for handling eeprom on sensor board
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPI1EEPROM
#define __SPI1EEPROM

/* Includes ------------------------------------------------------------------*/
#include "libopenstm32/common.h"	// Has things like 'u16' defined
#include "libopenstm32/gpio.h"
#include "libopenstm32/spi.h"


/* Note: The lower four bits of the priority byte are not used.
 The higher the priority number, the lower the priority */
#define DMA1_CH2_PRIORITY	0xB0	// Interrupt priority for DMA1 Channel2 interrupt (SPI1 Receive)

/* eeprom chip select pin macros */
#define EEPROM_NCS_hi	GPIO_BSRR(GPIOA)=(1<<4);	// Set bit
#define EEPROM_NCS_lo	GPIO_BRR(GPIOA) =(1<<4);	// Reset bit

/* eeprom write protect pin macros */
#define EEPROM_NWP_hi	GPIO_BSRR(GPIOC)=(1<<15);	// Set bit
#define EEPROM_NWP_lo	GPIO_BRR(GPIOC)=(1<<15);	// Reset bit


/******************************************************************************/
int spi1eeprom_init(void);
/*  @return	: zero = success; not zero = failed
 *  @brief 	: Initialize SPI1 for eeprom
*******************************************************************************/
void spi1_write (char *p, int count, void (*isr_ptr )(void));
/* @brief	: read 'count' bytes into buffer 'p'
 * @param	: char *p = pointer to byte buffer
 * @param	: int count = byte count
 * @param	: pointer to routine that is called when ISR completes
*******************************************************************************/
void spi1_read (volatile char *p, int count, char xmit, void (*spi2_readdoneptr)(void));
/* @brief	: read 'count' bytes, into buffer 'p' using DMA (see p 674)
 * @param	: char *p = pointer to byte buffer
 * @param	: int count = byte count to be read
 * @param	: char xmit  = outbound char during spi cycle
 * @param	: pointer to routine that is called when ISR completes
*******************************************************************************/
char spi1_busy (void);
/* @brief	: See if this whole mess is finished
 * @return	: 0 = not busy; 1 = busy doing all manner of things
*******************************************************************************/
int spi1_sclk_set_divisor_code (unsigned short usM);
/* @brief	: Change the baud rate divisor code
 * @param	: usM divisor code: 0 - 7.  Change is skipped for numbers outside range
 * @return	: zero = no Err; 1 = Busy, 2 = bad code number
*******************************************************************************/
unsigned int spi1_sclk_get_divisor_code (void);
/* @brief	: Get the current baud rate divisor code
 * @return	: Divisor code: 0 - 7
*******************************************************************************/

/* Pointer to a function that is called when DMA interrupt completes. */
extern void 	(*spi1_isrdoneptr )(void);	// Address of function to call upon completion of read.



#endif 
