/******************************************************************************
* File Name          : tension_a_printf.h
* Date First Issued  : 05/24/2015
* Board              : f103
* Description        : Print the values in the struct derived from the parameters table.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TENSION_A_PRINTF
#define __TENSION_A_PRINTF

#include <stdint.h>
#include <stdio.h>
#include "tension_idx_v_struct.h"

/* ************************************************************************************** */
void tension_a_printf(struct TENSIONLC* pten);
/* @brief	: Print the values
 * @param	: pten = pointer to struct with the values 
 * ************************************************************************************** */

#endif
