/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : xb_common.h
* Hackee             : deh
* Date First Issued  : 03/25/2012
* Board              : STM32F103VxT6_pod_mm with XBee
* Description        : Common for all XBee routines
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __XB_COMMON
#define __XB_COMMON

#include "libopenstm32/common.h"

#define	XBFRAMEDELIMITER	0x7e	// Start of frame delimiter (escaped)
#define XBESCAPE		0x7d	// Escape byte
#define XBXON			0x11	// XON (escaped)
#define XBXOFF			0x13	// XOFF (escaped)
#define XBXOR			0x20	// XOR' byte for escape


#endif 

