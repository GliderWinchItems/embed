/******************************************************************************
* File Name          : tension_a_param.h
* Date First Issued  : 04/08/2015
* Board              : f103
* Description        : Initialize tension function struct with default values
*******************************************************************************/
#ifndef __TENSION_A_PARAM_Q
#define __TENSION_A_PARAM_Q

#include <stdint.h>
#include "common_misc.h"
#include "common_can.h"
//#include "tension_function.h"

/* **************************************************************************************/
void tension_param_default_init(struct TENSIONLC* pten); 
/* @brief	: Initialize struct with default values
 * ************************************************************************************** */
void* tension_param_lookup_ptr(u32 id_num);
/* @brief	: Lookup the pointer to the parameter in the struct, given the id number
 * @param	: id_num = id number of the parameter (might not be the index in the table)
 * @return	: NULL = Not found, otherwise the pointer
 * ************************************************************************************** */

#endif 

