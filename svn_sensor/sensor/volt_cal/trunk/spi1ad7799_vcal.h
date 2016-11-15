/******************************************************************************
* File Name          : spi1ad7799_vcal.h
* Date First Issued  : 10/07/2015
* Board              : STM32F103VxT6_pod_mm
* Description        : SPI1 routines for AD7799, multi-channel readings
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPI1AD7799_VCAL
#define __SPI1AD7799_VCAL

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "libopenstm32/common.h"	// Has things like 'u16' defined
#include "libopenstm32/gpio.h"
#include "libopenstm32/spi.h"

/* Note: The lower four bits of the priority byte are not used.
 The higher the priority number, the lower the priority */
#define SPI1_PRIORITY		0x20	// Interrupt priority for SPI1 interrupt

/******************************************************************************/
int spi1ad7799_vcal_init(void);
/*  @brief Initialize SPI for SD Card Adapter
 *  @return	: zero = success; not zero = failed
*******************************************************************************/
void spi1_vcal_read (unsigned char *p, int count, char xmit);
/* @brief	: read 'count' bytes, into buffer 'p'
 * @param	: unsigned char *p = pointer to byte buffer
 * @param	: int count = byte count to be read (i.e. number of "xmit/read cycles"
 * @param	: char xmit  = outbound char during spi cycle
*******************************************************************************/
void spi1_vcal_write (unsigned char *p, int count);
/* @brief	: write 'count' bytes, out of buffer 'p'
 * @param	: char *p = pointer to byte buffer
 * @param	: int count = byte count to be read (i.e. number of "xmit/read cycles"
*******************************************************************************/
char spi1_vcal_busy (void);
/* @brief	: Check for buffer bytes remaining and spi1 busy bit 
 * @return	: not zero means busy
*******************************************************************************/
unsigned short ad7799_vcal_1_ready(void);
/* @brief	: Test ad7799_vcal_1 data out line low (data is ready)
 * @return	: Zero = ready; not-zero = data line still high
*******************************************************************************/
void spi1_vcal_ad7799_vcal_reset_noint (void);
/* @brief	: Non-interrupting (test) for reseting the AD7799
*******************************************************************************/
void spi1ad7799_vcal_DOUT_ext(void* pexti2);
/* @brief	: Configure AD7799 DOUT pin: for external interrupt
 * @param	: pexti2 = pointer to function to call when interrupt occurs
*******************************************************************************/
void spi1ad7799_vcal_push(void* pexti2 );
/* @brief	: Stack next function pointers
 * @param	: pexti2 = pointer to next function
*******************************************************************************/
void spi1ad7799_vcal_stkcall(void);
/* @brief	: Pop address from stack and call function 
*******************************************************************************/



#endif 
