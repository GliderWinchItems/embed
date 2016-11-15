/******************************************************************************
* File Name          : cansender_printf.h
* Date First Issued  : 09/08/2016
* Board              : f103
* Description        : Print the values in the struct derived from the parameters table.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CANSENDER_PRINTF
#define __CANSENDER_PRINTF

#include <stdint.h>
#include <stdio.h>
#include "cansender_idx_v_struct.h"

/* ************************************************************************************** */
void cansender_printf(struct CANSENDERLC* psend);
/* @brief	: Print the values
 * @param	: psend = pointer to struct with the values 
 * ************************************************************************************** */

#endif
