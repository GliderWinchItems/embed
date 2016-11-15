/******************************************************************************
* File Name          : f3DISCpinconfig.h
* Date First Issued  : 01/20/2016
* Board              : STM32F373
* Description        : Configure gpio port pins used, but not used for (hardware) functions
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DISCPIN_F4
#define __DISCPIN_F4

/* Includes ------------------------------------------------------------------*/
#include "../libopencm3/stm32/gpio.h"
#include "libopencm3/cm3/common.h"

struct PINCONFIG
{
	u8 	mode;
	u8	type;
	u8	speed;
	u8	pupdn;
	u8	afrl;
};

/******************************************************************************/
void f3DISCgpiopins_Config(void);
/* @brief	: Configure gpio pins
 ******************************************************************************/
void f3DISCgpiopins_default(void);
/* @brief	: Set pins to low power (default setting)
 ******************************************************************************/
void f3gpiopins_Config(volatile u32 * p, u16 pinnumber, struct PINCONFIG* s);
/* @param	: See comments on each in source code
 * @brief	: Configure one gpio pin 
 ******************************************************************************/



#endif 


