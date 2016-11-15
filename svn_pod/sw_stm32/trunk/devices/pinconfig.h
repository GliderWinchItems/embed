/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : pinconfig.h
* Hackeroo           : deh
* Date First Issued  : 03/15/2012
* Board              : STM32F103VxT6
* Description        : Configure gpio port pins
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PODPIN_1
#define __PODPIN_1

/* Includes ------------------------------------------------------------------*/
#include "libopenstm32/common.h"	// Has things like 'u16' defined
#include "libopenstm32/gpio.h"

/* Subroutines */
/* **************************************************************************************************************/
void configure_pin_input_pull_up_dn ( volatile u32 * p, int n, int up);
/* @example	: configure_pin ( (volatile u32 *)GPIOD, 12, 1); // Configures PD12 for input with pull up
 * @brief	: Configures pin for input with pull up or down
 * @param	: pointer to port base
 * @param	: bit in port to set or reset
 * @parm	: 0 = pull down, 1 = pull up.
 **************************************************************************************************************** */

#endif 


