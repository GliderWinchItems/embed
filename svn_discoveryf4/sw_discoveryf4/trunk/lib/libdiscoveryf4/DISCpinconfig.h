/******************************************************************************
* File Name          : DISCpinconfig.h
* Date First Issued  : 02/29/2012
* Board              : STM32 Discovery
* Description        : Configure gpio port pins used, but not used for (hardware) functions
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DISCPIN_F4
#define __DISCPIN_F4

/* Includes ------------------------------------------------------------------*/
#include "libopencm3/stm32/f4/gpio.h"

struct PINCONFIG
{
	u8 	mode;
	u8	type;
	u8	speed;
	u8	pupdn;
	u8	afrl;
};


/* Subroutines */
/******************************************************************************/
void DISCgpiopins_Config(void);
/* @brief	: Configure gpio pins
 ******************************************************************************/
void DISCgpiopins_default(void);
/* @brief	: Set pins to low power (default setting)
 ******************************************************************************/
void f4gpiopins_Config(volatile u32 * p, u16 pinnumber, struct PINCONFIG * s);
/* @param	: See comments on each in source code
 * @brief	: Configure one gpio pin 
 ******************************************************************************/


/* Macros for inline code */

/* Note: 
Switches and on-board LEDs are OFF when the pin is high, 
and power regulators are OFF when the pin is low.  

'-CS' or '-RESET' means NOT CS, NOT RESET, etc. 
*/

/* ----------------------------- PORTA -------------------------------------- */
	//  PA0- - DISC_box (external) LED: gpio out
#define PA0_WKUP_hi	GPIO_BSRR(GPIOA)=(1<<0);	// Set bit
#define PA0_WKUP_low	GPIO_BRR (GPIOA)=(1<<0);	// Reset bit
	//  PA4 - AD7799_2 /CS: gpio out

/* ----------------------------- PORTB -------------------------------------- */
/* ----------------------------- PORTC -------------------------------------- */
/* ----------------------------- PORTD -------------------------------------- */
#define LEDSALL_on	GPIO_BSRR(GPIOD)=(0x000f<<12);		// Set bit
#define LEDSALL_off	GPIO_BSRR(GPIOD)=(0x000f<<(12+16));	// Reset bit

#define LED_green_off	GPIO_BSRR (GPIOD)=(0x0001<<(12+16));	// Reset bit
#define LED_orange_off	GPIO_BSRR (GPIOD)=(0x0001<<(13+16));	// Reset bit
#define LED_red_off	GPIO_BSRR (GPIOD)=(0x0001<<(14+16));	// Reset bit
#define LED_blue_off	GPIO_BSRR (GPIOD)=(0x0001<<(15+16));	// Reset bit

#define LED_green_on	GPIO_BSRR (GPIOD)=(0x0001<<12);	// Set bit
#define LED_orange_on	GPIO_BSRR (GPIOD)=(0x0001<<13);	// Set bit
#define LED_red_on	GPIO_BSRR (GPIOD)=(0x0001<<14);	// Set bit
#define LED_blue_on	GPIO_BSRR (GPIOD)=(0x0001<<15);	// Set bit

/* ----------------------------- PORTE -------------------------------------- */

#endif 


