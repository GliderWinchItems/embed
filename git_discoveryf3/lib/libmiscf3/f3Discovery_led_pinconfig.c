/******************************************************************************
* File Name          : f3Discovery_led_pinconfig.c
* Date First Issued  : 01/25/2016
* Board              : Discovery F3
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
 * void f3Discovery_led_pinconfig_init(void);
 * @brief	: Configure gpio pins hooked to LED's
 ******************************************************************************/
void f3Discovery_led_pinconfig_init(void)
{
	int i;
	for (i = 8; i < 16; i++)
	{
		f3gpiopins_Config ((volatile u32*)GPIOE,  i, (struct PINCONFIG*)&ledpin);
		GPIO_BSRR(GPIOE) = (1<<(i+16));	// Be sure register is OFF
	}
	return;
}

