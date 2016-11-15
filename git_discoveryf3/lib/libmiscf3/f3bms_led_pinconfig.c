/******************************************************************************
* File Name          : f3bms_led_pinconfig.c
* Date First Issued  : 01/25/2016
* Board              : bmsf3 board (F373)
* Description        : Configure gpio for pins connected to LEDs
*******************************************************************************/
#include "../libopencm3/stm32/rcc.h"
#include "../libopencm3/stm32/gpio.h"
#include "f3DISCpinconfig.h"

const static struct PINCONFIG ledpin = { \
		GPIO_MODE_OUTPUT,		// mode: alternate function
		GPIO_OTYPE_PP, 		// output type: push-pull 		
		GPIO_OSPEED_100MHZ, 	// speed: highest drive level
		GPIO_PUPD_NONE, 	// pull up/down: none
		0 };			// 

/******************************************************************************
 * void f3bms_led_pinconfig_init(void);
 * @brief	: Configure gpio pins hooked to LED's
 ******************************************************************************/
void f3bms_led_pinconfig_pinconfig_init(void)
{
	int i;
	for (i = 0; i < 7; i++)
	{
		f3gpiopins_Config ((volatile u32*)GPIOE,  i, (struct PINCONFIG*)&ledpin);
		GPIO_BSRR(GPIOE) = (1<<(i+16));	// Be sure register is OFF
	}
	return;
}

