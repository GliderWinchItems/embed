/******************************************************************************
* File Name          : idx_v_struct_print.h
* Date First Issued  : 08/21/2015
* Board              : f103
* Description        : Print values with idx and struct (for debugging/monitoring)
*******************************************************************************/
/*
This is used for the F103 since the launchpad compiler was not working for 
floating pt printf (which works for the 'F4).
*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __IDX_V_STRUCT_PRINT
#define __IDX_V_STRUCT_PRINT

#include "yogurt_idx_v_struct.h"

/* **************************************************************************************/
void idx_v_struct_print(struct YOGURTTHERMS* p);
/* @brief	: Print values of struct after init from flat table
 * @param	: p = pointer to struct that has been init'd
 * ************************************************************************************** */

#endif 
