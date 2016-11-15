/******************************************************************************
* File Name          : settables_print.h
* Date First Issued  : 09/07/2015
* Board              : f103
* Description        : yogurt maker--print things PC can set
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SETTABLES_PRINT
#define __SETTABLES_PRINT

#include <stdint.h>

/* **************************************************************************************/
void settables_print(void);
/* @brief	: Print values for items that can be set from PC
 * ************************************************************************************** */
void settables_print_menu(void);
/* @brief	: Print menu for PC keyboard input
 * ************************************************************************************** */

#endif 
