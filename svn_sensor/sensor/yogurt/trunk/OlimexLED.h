/******************************************************************************
* File Name          : OlimexLED.h
* Date First Issued  : 08/20/2015
* Board              : f103
* Description        : LED flashing for the Olimex board's LED
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __OLIMEXLED
#define __OLIMEXLED

#include <stdint.h>

/******************************************************************************/
void OlimexLED_init(void);
/* @brief	: Init LED on Olimex board
******************************************************************************/
void OlimexLED_togglepoll(void);
/* @brief	: Toggle LED at a rate (call this in polling loop)
*******************************************************************************/
void OlimexLED_flashpin(uint32_t gpio, uint32_t pinnumber);
/* @brief	: Turn LED ON when gpio pin is high, (assume pin was configured)
 * @param	: gpio = address of gpio port
 * @param	: pinnumber = gpio pin number on port
*******************************************************************************/
void OlimexLED_settogglerate(int32_t togglerate);
/* @brief	: Set rate for toggling LED
 * @param	: togglerate = toggles per 10 sec; 0 = continous OFF; -1 = ON
*******************************************************************************/

#endif 
