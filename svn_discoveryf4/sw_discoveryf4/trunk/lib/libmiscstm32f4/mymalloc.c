/******************************************************************************
* File Name          : mymalloc.c
* Date		     : 03/08/2011
* Description        : *One-time-only* memory allocation from a MYHEAP, align 4
*******************************************************************************/ 
/* 
$ = These commented-out statements were used when the linker script (.ld) defined
       the memory block in which allocations are made in this routine.
*/
#include "../libopencm3/cm3/common.h"

/*******************************************************************************
* void* mymalloc(u16 size)
* @brief	: *One-time-only* memory allocation from MYHEAP align 4
* @param	: Size of block (in bytes) to be allocated
* @return	: == 0 for out of space; non-zero is pointer to beginning of block
*******************************************************************************/
extern	unsigned char _MYHEAP_START;	// Defined in .ld file
extern	unsigned char _MYHEAP_END;	// Defined in .ld file
unsigned char* heap __attribute__ ( (aligned (4) ) ) = &_MYHEAP_START;// Next available memory location

void* malloc_r(unsigned int size);

void* malloc(unsigned int size)
{
	unsigned char* prev_heap = heap;

	prev_heap = heap;
	heap += size;
	
	heap += 3;
	heap = (unsigned char*)((u32)heap & ~0x03);	// Round upwards to align to full words

	if (heap >= &_MYHEAP_END)		// Are we out of space?
		return 0;		// Here, yes.  Return zero (null)  

	return prev_heap;		// Return pointer to beginning of block
}

