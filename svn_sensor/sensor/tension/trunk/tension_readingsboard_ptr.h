/******************************************************************************
* File Name          : tension_readingsboard_ptr.h
* Date First Issued  : 06/11/2016
* Board              :
* Description        : Pointer to reading versus code for program level readings
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TENSION_READINGSBOARD
#define __TENSION_READINGSBOARD

#include <stdint.h>

struct READINGSBOARDPTR 
{
	void*	pread;	// Pointer to memory location
	void*	pptr;	// Pointer pointer
	void*	pptr2;	// Pointer to dummy struct
	uint16_t code;	// Identification number
	uint16_t type;	// Type of number (for format purposes)
};

/* Table lookup return */
struct READINGSBOARDPTR_RET
{
	uint32_t val;	// 4 byte value
	uint16_t type;	// Type code			
};

/* **************************************************************************************/
struct READINGSBOARDPTR_RET tension_readingsboard_ptr_get(uint16_t code);
/* @brief	: Get the 4 byte value and type code, given the code
 * @param	:  code = code number for reading
 * @return	:  struct: 
 *		:  .val: value as uint32_t
 *		:  .type: number type code (for format and usage)
 *		:  Code not found-- .type = 0  
 * ************************************************************************************** */

#endif

