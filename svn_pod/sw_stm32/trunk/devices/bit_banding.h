/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : bit_banding.h
* Hackeroo           : deh
* Date First Issued  : 10/25/2011
* Board              : STM32F103
* Description        : Macros for doing bit-banding
*******************************************************************************/

#ifndef __BIT_BANDING_MACRO
#define __BIT_BANDING_MACRO

/* Bit banding needs to be used to reset the update-event interrupt flag (so as to 
not cause a input capture flag coming on and getting reset */


#define MMIO32_BIT_BAND(addr,bitnum) (*(volatile u32 *)(((long)addr & 0xf0000000)+0x02000000+(((long)addr & 0xfffff)<<5)+(bitnum<<2)))



#endif

