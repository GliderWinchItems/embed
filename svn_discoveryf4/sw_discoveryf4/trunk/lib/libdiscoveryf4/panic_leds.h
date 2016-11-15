/******************************************************************************
* File Name          : panic_leds.h
* Date First Issued  : 05/24/2013
* Board              : STM32F103RxT6 sensor board
* Description        : Panic flash LED's
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PANICFLASHLEDS
#define __PANICFLASHLEDS


/* Subroutines */
/******************************************************************************/
void panic_leds(unsigned int count);
/* @param	: Number of fast flashes
 * @brief	: Configure gpio pins
 ******************************************************************************/

#endif 
