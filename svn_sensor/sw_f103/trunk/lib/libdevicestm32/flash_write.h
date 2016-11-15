/******************************************************************************
* File Name          : flash_write.h
* Date First Issued  : 07/28/2013
* Board              : ../svn_sensor/hw/trunk/eagle/f103R/RxT6
* Description        : flash write: small bits of code that execute in ram
*******************************************************************************/

#include "flash.h"
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FLASH_WRITE
#define __FLASH_WRITE

#define FLASH_SIZE_REG	MMIO16(0x1FFFF7E0)	//This field value indicates the Flash memory size of the device in Kbytes.
						//Example: 0x0080 = 128 Kbytes. (p 1044 ref man)


/* Includes ------------------------------------------------------------------*/
#include "libopenstm32/common.h"	// Has things like 'u16' defined

/******************************************************************************/
int flash_write_ram(u16 *pflash, u16 *pfrom, int count);
/*  @brief 	: Write "count" u16 (1/2 words) to flash
 *  @param	: pflash = 1/2 word pointer to address in flash
 *  @param	: pfrom = 1/2 word pointer to address with source data
 *  @param	: count = number of 1/2 words to write
 *  @return	: zero = success; not zero = failed
*******************************************************************************/
int flash_write(u16 *pflash, u16 *pfrom, int count);
/*  @brief 	: Write "count" u16 (1/2 words) to flash
 *  @param	: pflash = 1/2 word pointer to address in flash
 *  @param	: pfrom = 1/2 word pointer to address with source data
 *  @param	: count = number of 1/2 words to write
 *  @return	: 
 *           0 = success
 *          -1 = address greater than 1 MB
 *          -2 = unlock sequence failed for upper bank
 *          -3 = address below start of ram.
 *          -4 = unlock sequence failed for lower bank
 *          -5 = error at some point in the writes, flash_err has the bits
*******************************************************************************/
int flash_erase(u16 *pflash);
/*  @brief 	: Erase one page
 *  @param	: pflash = 1/2 word pointer to address in flash
 *  @param	: pfrom = 1/2 word pointer to address with source data
 *  @return	: 
 *           0 = success
 *          -1 = address greater than 1 MB
 *          -2 = unlock sequence failed for upper bank
 *          -3 = address below start of ram.
 *          -4 = unlock sequence failed for lower bank
 *          -5 = error at some point in the writes, flash_err has the bits
*******************************************************************************/


#endif

