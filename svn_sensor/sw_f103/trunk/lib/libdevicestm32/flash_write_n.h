/******************************************************************************
* File Name          : flash_write_n.h
* Date First Issued  : 07/28/2013
* Board              : ../svn_sensor/hw/trunk/eagle/f103R/RxT6
* Description        : write 'n' bytes to flash (crosses block boundaries)
*******************************************************************************/

#ifndef __FLASH_WRITE_N
#define __FLASH_WRITE_N

#include "common.h"
#include "flash_write.h"

/******************************************************************************/
int flash_write_n(u8* pflash, u8* psrc, u32 n);
/*  @brief 	: Write 'n' bytes to flash
 *  @param	: pstart = pointer to flash 
 *  @param	: psrc = pointer to source bytes
 *  @param	: n = number of bytes
 *  @return	: zero = success; not zero = failed
*******************************************************************************/


#endif

