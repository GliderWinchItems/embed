/******************************************************************************
* File Name          : crc-32.h
* Date First Issued  : 10/03/2014
* Board              : ...
* Description        : CRC-32 (byte-byt-byte) computation
*******************************************************************************/

#ifndef __CRC_32B
#define __CRC_32B

#include "common.h"
/******************************************************************************/
uint32_t rc_crc32(u8 *buf, u32 len);
/* @brief 	: Compute CRC-32 byte-byt-byte
 * @param	: pData = pointer input bytes
 * @param	: len = byte count
 * @return	: crc
*******************************************************************************/

#endif 

