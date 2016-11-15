/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : commonbitband.h
* Hackor             : deh
* Date First Issued  : 10/07/2010
* Description        : Macros for bit banding operations
*******************************************************************************/
/*
Referecne--
_The Definitive Guide To The ARM-CORTEX-M3_, second edition, Elesevier (2010), p91_
*/
#ifndef __COMMONBITBAND_H
#define __COMMONBITBAND_H

#define BITBAND(addr,bitnum) ((addr & 0xf0000000)+0x02000000+((addr & 0xfffff)<<5)+(bitnum<<2))
#define MEM_ADDR(addr) *((volatile unsigned long *) (addr))


#endif





