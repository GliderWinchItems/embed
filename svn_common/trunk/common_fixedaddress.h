/******************************************************************************
* File Name          : common_fixedaddress.h
* Date First Issued  : 09/23/2014
* Board              : 
* Description        : Flash space at a fixed address
*******************************************************************************/
/*


*/

#ifndef __COMMON_FIXEDADDRESS
#define __COMMON_FIXEDADDRESS

struct FIXEDADDRESS
{
	unsigned int canid_ldr;		// Unique to each mfg'ed unit	
	unsigned int board_typ;		// Board type code
	// See 'ldr.c' for functions set into the following--
	void 	(*func1)(void);		// Subroutine pointers
	void 	(*func2)(void);		// Subroutine pointers
	void 	(*func3)(void);		// Subroutine pointers
	void 	(*func4)(void);		// Subroutine pointers
};

void unique_can_block(void);

extern const struct FIXEDADDRESS fixedaddress;

#endif
