/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : sdcard_csd.c
* Hacker             : deh
* Date First Issued  : 08/28/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Extraction of sd card csd fields 
*******************************************************************************/
#include "sdcard_csd.h"


static unsigned short get_bit(char* csd,unsigned short usG)
{
	int x = 15 - (usG >> 3);
	int y = usG & 0x07;
	return	( (*(csd+x) >> y) & 0x01);
}

/**********************************************************************************
 * int sdcard_csd_extract_v1 (char* csd, unsigned short usFieldBitNumber, unsigned short usNumBits);
 * @brief	: Extract the bit field into an easy-to-use int from the CSD data (sd v1)
 * @param	: Pointer to CSD record (16 bytes)
 * @param	: Bit number of the field of the low order bit
 * @param	: Number of bits
 * @return	: value in the field
**********************************************************************************/
int sdcard_csd_extract_v1 (char* csd, unsigned short usFieldBitNumber, unsigned short usNumBits)
{
	int i;
	int x = 0;

	for (i = (usNumBits-1); i >= 0; i--)
	{
		x = (x << 1) + get_bit(csd, usFieldBitNumber + i);
	}
	return x;

}
/**********************************************************************************
 * unsigned int sdcard_csd_memory_size (char* csd);
 * @brief	: Compute memory size
 * @param	: Pointer to CSD record (16 bytes)
 * @return	: Card size (number of blocks)
**********************************************************************************/
unsigned int sdcard_csd_memory_size (char* csd)
{
/*
See page 81 (p 92 in pdf viewer) for computation
*/
	if((*csd & 0xc0) == 0)
	{
		unsigned int c_mult = sdcard_csd_extract_v1 (csd,SDC_C_SIZE_MULT );
		unsigned int c_size = sdcard_csd_extract_v1 (csd,SDC_C_SIZE);

		/* This returns the card size in blocks.  All cards have 512 byte
		 * blocks _except_ the 2GB card.  The 2GB card has 1024 byte blocks.
		 */
		return (c_size+1)*(1<<(c_mult+2));
	}
	else
/* ############### note the + 0! The correct value is + 1, but that screws up POD stuff. ############## */
		return (sdcard_csd_extract_v1 (csd, 48, 22) + 0) * 1024;
}
/**********************************************************************************
 * unsigned int sdcard_csd_block_size (char* csd);
 * @brief	: Compute block size
 * @param	: Pointer to CSD record (16 bytes)
 * @return	: Block size (in bytes)
**********************************************************************************/
unsigned int sdcard_csd_block_size (char* csd)
{
/*
See page 81 (p 92 in pdf viewer) for computation
*/
	return (1<< (sdcard_csd_extract_v1 (csd,SDC_READ_BL_LEN )));
}
