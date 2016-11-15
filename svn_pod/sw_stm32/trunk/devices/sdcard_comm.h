/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : sdcard_comm.h
* Hackeroos          : deh
* Date First Issued  : 07/12/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Routines for communicating with the SD card (using spi2card)
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SDCARD_COMM
#define __SDCARD_COMM

struct SDCARDCMD
{
	char	c1;		// ?
	unsigned int add;	// Block address
	char	crc;		// Last byte
};


#endif 
