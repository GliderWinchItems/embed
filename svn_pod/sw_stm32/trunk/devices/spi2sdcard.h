/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : spi2sdcard.h
* Hackeroos           : deh, caw, others too numerous to mention
* Date First Issued  : 07/11/2011
* Board              : STM32F103VxT6_pod_mm or Olimex P103
* Description        : SPI2 routines for SDCARDs
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPI2SDCARD
#define __SPI2SDCARD

/* Includes ------------------------------------------------------------------*/
#include "libopenstm32/common.h"	// Has things like 'u16' defined
#include "libopenstm32/gpio.h"
#include "libopenstm32/spi.h"


/* Note: The lower four bits of the priority byte are not used.
 The higher the priority number, the lower the priority */
#define DMA1_CH4_PRIORITY	0xB0	// Interrupt priority for DMA1 Channel4 interrupt (SPI2 Receive)

/******************************************************************************/
void spi2sdcard_init(void);
/*  @brief Initialize SPI for SD Card Adapater
*******************************************************************************/
void spi2_write (char *p, int count, void (*isr_ptr )(void));
/* @brief	: read 'count' bytes into buffer 'p'
 * @param	: char *p = pointer to byte buffer
 * @param	: int count = byte count
 * @param	: pointer to routine that is called when ISR completes
*******************************************************************************/
void spi2_read (volatile char *p, int count, char xmit, void (*spi2_readdoneptr )(void));
/* @brief	: read 'count' bytes, into buffer 'p' using DMA (see p 674)
 * @param	: char *p = pointer to byte buffer
 * @param	: int count = byte count to be read
 * @param	: char xmit  = outbound char during spi cycle
 * @param	: pointer to routine that is called when ISR completes
*******************************************************************************/
char spi2_busy (void);
/* @brief	: See if this whole mess is finished
 * @return	: 0 = not busy; 1 = busy doing all manner of things
*******************************************************************************/
void spi2_poi (void);
/* @brief	: Power on insertion: more than 74 SCLKs while /CS and DI held high
*******************************************************************************/
int spi2_sclk_set_divisor_code (unsigned short usM);
/* @brief	: Change the baud rate divisor code
 * @param	: usM divisor code: 0 - 7.  Change is skipped for numbers outside range
 * @return	: zero = no Err; 1 = Busy, 2 = bad code number
*******************************************************************************/
unsigned int spi2_sclk_get_divisor_code (void);
/* @brief	: Get the current baud rate divisor code
 * @return	: Divisor code: 0 - 7
*******************************************************************************/


#endif 
