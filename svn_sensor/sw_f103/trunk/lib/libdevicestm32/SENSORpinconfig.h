/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : SENSORpinconfig.h
* Author             : deh
* Date First Issued  : 05/20/2013
* Board              : STM32F103RxT6 sensor board
* Description        : Setup basic I/O pins for sensor board
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SENSORPIN
#define __SENSORPIN

/* Includes ------------------------------------------------------------------*/
#include "libopenstm32/common.h"	// Has things like 'u16' defined
#include "libopenstm32/gpio.h"

/* Subroutines */
/******************************************************************************/
void SENSORgpiopins_Config(void);
/* @brief	: Configure gpio pins
 ******************************************************************************/
void configure_pin ( volatile u32 * p, int n);
/* @example	: configure_pin ( (volatile u32 *)GPIOD, 12); // Configures PD12 for pushpull output
 * @brief	: configure the pin push/pull output
 * @param	: pointer to port base
 * @param	: bit in port to set or reset
 **************************************************************************** */

#define LED21GREEN_on 	GPIO_BSRR(GPIOB)=(1<<15);	// Set bit
#define LED21GREEN_off	GPIO_BRR(GPIOB) =(1<<15);	// Reset bit
#define LED19RED_on	GPIO_BSRR(GPIOC)=(1<<4);	// Set bit
#define LED19RED_off	GPIO_BRR(GPIOC) =(1<<4);	// Reset bit
#define LED20RED_on	GPIO_BSRR(GPIOC)=(1<<5);	// Set bit
#define LED20RED_off	GPIO_BRR(GPIOC) =(1<<5);	// Reset bit
#define TOGGLE_GREEN 	if ((GPIO_ODR(GPIOB) & (1<<15)) == 0) GPIO_BSRR(GPIOB)=(1<<15);else GPIO_BRR(GPIOB)=(1<<15);


#endif 
