/******************************************************************************
* File Name          : spi1sad7799_ten.h
* Date First Issued  : 07/03/2015
* Board              : STM32F103VxT6_pod_mm
* Description        : SPI1 routines for AD7799_1 winch tension
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __spi1ad7799_ten
#define __spi1ad7799_ten

/* Includes ------------------------------------------------------------------*/
#include "libopenstm32/common.h"	// Has things like 'u16' defined
#include "libopenstm32/gpio.h"
#include "libopenstm32/spi.h"


/* Note: The lower four bits of the priority byte are not used.
 The higher the priority number, the lower the priority */
#define SPI1_PRIORITY		0x20	// Interrupt priority for SPI1 interrupt


/******************************************************************************/
int spi1ad7799_ten_init(void);
/* @brief 	: Initialize SPI for AD7799
 * @return	: 0 = ok; not 0 badness
*******************************************************************************/
void spi1_ten_read (unsigned char *p, int count, char xmit);
/* @brief	: read 'count' bytes, into buffer 'p'
 * @param	: unsigned char *p = pointer to byte buffer
 * @param	: int count = byte count to be read (i.e. number of "xmit/read cycles"
 * @param	: char xmit  = outbound char during spi cycle
*******************************************************************************/
void spi1_ten_write (unsigned char *p, int count);
/* @brief	: write 'count' bytes, out of buffer 'p'
 * @param	: char *p = pointer to byte buffer
 * @param	: int count = byte count to be read (i.e. number of "xmit/read cycles"
*******************************************************************************/
char spi1_ten_busy (void);
/* @brief	: Check for buffer bytes remaining and spi1 busy bit 
 * @return	: not zero means busy
*******************************************************************************/
unsigned short ad7799_1_ready(void);
/* @brief	: Test ad7799_1 data out line low (data is ready)
 * @return	: Zero = ready; not-zero = data line still high
*******************************************************************************/
unsigned short ad7799_2_ready(void);
/* @brief	: Test ad7799_2 data out line low (data is ready)
 * @return	: Zero = ready; not-zero = data line still high
*******************************************************************************/
void ad7799_1_select(void);
/* @brief	: Select #1, deselect #2
*******************************************************************************/
void ad7799_2_select(void);
/* @brief	: Select #2, deselect #1
*******************************************************************************/
void SPIsd_dummy(void);
/* @brief	: Function called from the ISR that merely returns
**************************************************************************/
void spi1_ten_ad7799_reset_noint (void);
/* @brief	: Non-interrupting (test) for reseting the AD7799
*******************************************************************************/

extern unsigned char ad7799_selected;	// Current /CS line selected: 0 = AD7799_1, 1 = AD7799_2

#endif 
