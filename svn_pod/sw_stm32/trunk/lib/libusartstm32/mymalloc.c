/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : mymalloc.c
* Hackor             : deh
* Date First Issued  : 09/29/2010 deh
* Description        : *One-time-only* memory allocation from a MYHEAP, align 4
* Updates            : 10-27-2010 Changed to use static block rather than .ld file
*******************************************************************************/ 
/* 
$ = These commented-out statements were used when the linker script (.ld) defined
       the memory block in which allocations are made in this routine.
*/
#include "../libopenstm32/common.h"
#include "mymalloc.h"

/*******************************************************************************
* void* mymalloc(u16 size)
* @brief	: *One-time-only* memory allocation from MYHEAP align 4
* @param	: Size of block (in bytes) to be allocated
* @return	: == 0 for out of space; non-zero is pointer to beginning of block
*******************************************************************************/
// $ extern	unsigned char _MYHEAP_START;	// Defined in .ld file
// $ extern	unsigned char _MYHEAP_END;	// Defined in .ld file
// $ unsigned char* myheap __attribute__ ((aligned (4))) = &_MYHEAP_START;// Next available memory location

unsigned char  myheapblock[MYHEAPBLOCKSIZE];	// Block for all usart buffers & control blocks
unsigned char* myheap = &myheapblock[0];		// Pointer to block for usart buffers

void* mymalloc(u16 size)
{
	unsigned char* prev_heap = myheap;

	prev_heap = myheap;
	myheap += size;
	
	myheap += 3;
	myheap = (unsigned char*)((u32)myheap & ~0x03);	// Round upwards to align to full words

// $	if (myheap >= &_MYHEAP_END)		// Are we out of space?
	if (myheap >= (&myheapblock[0] + MYHEAPBLOCKSIZE))	// Are we out of space?
		return 0;		// Here, yes.  Return zero (null)  

	return prev_heap;		// Return pointer to beginning of block
}

