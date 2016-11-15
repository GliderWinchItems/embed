/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : mymalloc.h
* Hackor             : deh
* Date First Issued  : 09/29/2010
* Description        : Allocate memory from heap. Pointer is 4 aligned
* Updates            : 10-27-2010 Changed to use static block rather than .ld file
*******************************************************************************/

/*******************************************************************************
* void* usartx_putsS(u16 size);
* @brief	: Allocate memory from heap
* @param	: Pointer
* @return	: == 0 for out of space; non-zero is pointer to beginning of 4 aligned block
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MYMALLOC_H
#define __MYMALLOC_H

#define MYHEAPBLOCKSIZE 2048	// Size of static block set aside for USART buffers

void* mymalloc(u16 size);

#endif

