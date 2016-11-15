/******************************************************************************
* File Name          : commonbitband.h
* Date First Issued  : 03/01/2012
* Description        : Macros for bit banding operations
*******************************************************************************/
/*
From the book--
_The Definitive Guide To The ARM-CORTEX-M3_, second edition, Elesevier (2010), p91_
*/
#ifndef __COMMONBITBAND_H
#define __COMMONBITBAND_H

#define BITBAND(addr,bitnum) ((addr & 0xf0000000)+0x02000000+((addr & 0xfffff)<<5)+(bitnum<<2))
#define MEM_ADDR(addr) *((volatile unsigned long *) (addr))


#endif





