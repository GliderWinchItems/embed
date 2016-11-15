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
void spi1ad7799_ten_init(void);
/*  @brief Initialize SPI for SD Card Adapater
*******************************************************************************/
void spi1_read (unsigned char *p, int count, char xmit);
/* @brief	: read 'count' bytes, into buffer 'p'
 * @param	: unsigned char *p = pointer to byte buffer
 * @param	: int count = byte count to be read (i.e. number of "xmit/read cycles"
 * @param	: char xmit  = outbound char during spi cycle
*******************************************************************************/
void spi1_write (unsigned char *p, int count);
/* @brief	: write 'count' bytes, out of buffer 'p'
 * @param	: char *p = pointer to byte buffer
 * @param	: int count = byte count to be read (i.e. number of "xmit/read cycles"
*******************************************************************************/
char spi1_busy (void);
/* @brief	: Check for buffer bytes remaining and spi1 busy bit 
 * @return	: not zero means busy
*******************************************************************************/
unsigned short ad7799_1_ready(void);
/* @brief	: Test ad7799_1 data out line low (data is ready)
 * @return	: Zero = ready; not-zero = data line still high
*******************************************************************************/
void SPIsd_dummy(void);
/* @brief	: Function called from the ISR that merely returns
**************************************************************************/

void spi1_ad7799_reset_noint (void);
/* @brief	: Non-interrupting (test) for reseting the AD7799
*******************************************************************************/
#endif 
