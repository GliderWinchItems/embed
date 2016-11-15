/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : hwcrc.h
* Generator          : deh
* Date First Issued  : 01/14/2012
* Board              : STM32F103VxT6_pod_mm
* Description        : Hardware CRC
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HWCRC
#define __HWCRC

/* Includes ------------------------------------------------------------------*/

/******************************************************************************/
unsigned int hwcrcgen(unsigned int * p, unsigned int  count);
/* @brief	: Compute a CRC using the hardware CRC feature
 * @param	: p: Pointer to array to be crc'ed
 * @param	: count: (==> INT <==) to be CRC'ed
 * @return	: CRC 
 ******************************************************************************/

#endif 
