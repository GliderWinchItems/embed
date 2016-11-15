/******************************************************************************
* File Name          : idx_v_struct.h
* Date First Issued  : 07/14/2015
* Board              :
* Description        : Translate parameter index into pointer into struct
*******************************************************************************/

#include <stdint.h>
#include "common_can.h"
//#include "tmpstruct.h"

#ifndef __IDX_V_STRUCT
#define __IDX_V_STRUCT



/******************************************************************************/
uint32_t idx_v_struct_cp_arraytostruct((void* parray, struct TENSIONLC* strptr);
/* @brief	: Find pointer to struct element, when given the id number of the element
 * @param	: parray = pointer to array of void*
 * @param	: strptr = pointer to element x in struct for a given array[x] pointer
 * @return	: number of elements 
 ******************************************************************************/
uint32_t idx_v_struct_cp_arraytostruct(struct TENSIONLC* strptr, void* parray);
/* @brief	: Find pointer to struct element, when given the id number of the element
 * @param	: strptr = pointer to element x in struct for a given array[x] pointer
 * @param	: parray = pointer to array of void*
 * @return	: number of elements 
 ******************************************************************************/

#endif


