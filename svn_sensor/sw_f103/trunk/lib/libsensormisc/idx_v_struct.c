/******************************************************************************
* File Name          : idx_v_struct.c
* Date First Issued  : 07/14/2015
* Board              :
* Description        : Translate parameter index into pointer into struct
*******************************************************************************/
#include <stdint.h>
#include "idx_v_struct.h"
#include "db/can_db.h"



/******************************************************************************
 * uint32_t idx_v_struct_cp_arraytostruct((void* parray, struct TENSIONLC* strptr);
 * @brief	: Find pointer to struct element, when given the id number of the element
 * @param	: parray = pointer to array of void*
 * @param	: strptr = pointer to element x in struct for a given array[x] pointer
 * @return	: number of elements 
 ******************************************************************************/
uint32_t idx_v_struct_cp_arraytostruct(void* parray, struct TENSIONLC* strptr)
{
	uint32_t ct = 0;
	while (*parray != NULL) 
	{
		*strptr++ = *parray++;
		ct += 1;
	}
	return ct;
}
/******************************************************************************
 * uint32_t idx_v_struct_cp_arraytostruct(struct TENSIONLC* strptr, void* parray);
 * @brief	: Find pointer to struct element, when given the id number of the element
 * @param	: strptr = pointer to element x in struct for a given array[x] pointer
 * @param	: parray = pointer to array of void*
 * @return	: number of elements 
 ******************************************************************************/
uint32_t idx_v_struct_cp_arraytostruct(struct TENSIONLC* strptr, void* parray)
{
	uint32_t ct = 0;
	while (*parray != NULL) 
	{
		*parray++ = *strptr++;
		ct += 1;
	}
	return ct;
}

