/******************************************************************************
* File Name          : f3bms_led_pinconfig.h
* Date First Issued  : 01/25/2016
* Board              : bmsf3 board (F373)
* Description        : Configure gpio for pins connected to LEDs
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BMSF3_LED_PIN
#define __BMSF3_LED_PIN

/* Includes ------------------------------------------------------------------*/
#include "../libopencm3/stm32/gpio.h"


/* Subroutine */
/******************************************************************************/
void f3bms_led_pinconfig_init(void);
/* @brief	: Configure gpio pins hooked to LED's
 ******************************************************************************/

/* ----------------------------- PORTE -------------------------------------- */
#define LEDSALL_on	GPIO_BSRR(GPIOE)=(0x007f<<8);		// Set bit
#define LEDSALL_off	GPIO_BSRR(GPIOE)=(0x007f<<(8+16));	// Reset bit

#define LED_1_off	GPIO_BSRR (GPIOE)=(0x0001<<(0+16));	// Reset bit
#define LED_2_off	GPIO_BSRR (GPIOE)=(0x0001<<(1+16));	// Reset bit
#define LED_3_off	GPIO_BSRR (GPIOE)=(0x0001<<(2+16));	// Reset bit
#define LED_4_off	GPIO_BSRR (GPIOE)=(0x0001<<(3+16));	// Reset bit
#define LED_5_off	GPIO_BSRR (GPIOE)=(0x0001<<(4+16));	// Reset bit
#define LED_6_off	GPIO_BSRR (GPIOE)=(0x0001<<(5+16));	// Reset bit
#define LED_7_off	GPIO_BSRR (GPIOE)=(0x0001<<(6+16));	// Reset bit

#define LED_1_on	GPIO_BSRR (GPIOE)=(0x0001<<(0));	// Reset bit
#define LED_2_on	GPIO_BSRR (GPIOE)=(0x0001<<(1));	// Reset bit
#define LED_3_on	GPIO_BSRR (GPIOE)=(0x0001<<(2));	// Reset bit
#define LED_4_on	GPIO_BSRR (GPIOE)=(0x0001<<(3));	// Reset bit
#define LED_5_on	GPIO_BSRR (GPIOE)=(0x0001<<(4));	// Reset bit
#define LED_6_on	GPIO_BSRR (GPIOE)=(0x0001<<(5));	// Reset bit
#define LED_7_on	GPIO_BSRR (GPIOE)=(0x0001<<(6));	// Reset bit


#endif 


