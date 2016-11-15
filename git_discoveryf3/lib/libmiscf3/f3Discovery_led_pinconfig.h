/******************************************************************************
* File Name          : f3Discovery_led_pinconfig.h
* Date First Issued  : 01/25/2016
* Board              : Discovery F3
* Description        : Configure gpio for pins connected to LEDs
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DISCOVERYF3_LED_PIN
#define __DISCOVERYF3_LED_PIN

/* Includes ------------------------------------------------------------------*/
#include "libopencm3/stm32/f4/gpio.h"


/* Subroutine */
/******************************************************************************/
void f3Discovery_led_pinconfig_init(void);
/* @brief	: Configure gpio pins hooked to LED's
 ******************************************************************************/

/* ----------------------------- PORTE -------------------------------------- */
#define DF3LEDSALL_on	GPIO_BSRR(GPIOE)=(0x00ff<<8);		// Set bit
#define DF3LEDSALL_off	GPIO_BSRR(GPIOE)=(0x00ff<<(8+16));	// Reset bit

#define LED_E_green_off		GPIO_BSRR (GPIOE)=(0x0001<<(15+16));	// Reset bit
#define LED_NE_orange_off	GPIO_BSRR (GPIOE)=(0x0001<<(14+16));	// Reset bit
#define LED_N_red_off		GPIO_BSRR (GPIOE)=(0x0001<<(13+16));	// Reset bit
#define LED_NW_blue_off		GPIO_BSRR (GPIOE)=(0x0001<<(12+16));	// Reset bit
#define LED_W_green_off		GPIO_BSRR (GPIOE)=(0x0001<<(11+16));	// Reset bit
#define LED_SE_orange_off	GPIO_BSRR (GPIOE)=(0x0001<<(10+16));	// Reset bit
#define LED_S_red_off		GPIO_BSRR (GPIOE)=(0x0001<<( 9+16));	// Reset bit
#define LED_SW_blue_off		GPIO_BSRR (GPIOE)=(0x0001<<( 8+16));	// Reset bit

#define LED_E_green_on		GPIO_BSRR (GPIOE)=(0x0001<<(15));	// Set bit
#define LED_NE_orange_on	GPIO_BSRR (GPIOE)=(0x0001<<(14));	// Set bit
#define LED_N_red_on		GPIO_BSRR (GPIOE)=(0x0001<<(13));	// Set bit
#define LED_NW_blue_on		GPIO_BSRR (GPIOE)=(0x0001<<(12));	// Set bit
#define LED_W_green_on		GPIO_BSRR (GPIOE)=(0x0001<<(11));	// Set bit
#define LED_SE_orange_on	GPIO_BSRR (GPIOE)=(0x0001<<(10));	// Set bit
#define LED_S_red_on		GPIO_BSRR (GPIOE)=(0x0001<<( 9));	// Set bit
#define LED_SW_blue_on		GPIO_BSRR (GPIOE)=(0x0001<<( 8));	// Set bit


#endif 


