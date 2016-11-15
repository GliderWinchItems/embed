/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : SENSORpinconfig.c
* Author             : deh
* Date First Issued  : 05/20/2013
* Board              : STM32F103RxT6 sensor board
* Description        : Setup basic I/O pins for sensor board
*******************************************************************************/

#include "libopenstm32/gpio.h"
#include "pinconfig_all.h"

/******************************************************************************
 * int SENSORgpiopins_Config(void);
 * @brief	: Configure gpio pins
 * @return	: 0 = OK, not zero = error
 ******************************************************************************/
const struct PINCONFIGALL led21green = {(volatile u32 *)GPIOB, 15, OUT_PP, MHZ_2};
const struct PINCONFIGALL led20red   = {(volatile u32 *)GPIOC,  5, OUT_PP, MHZ_2};
const struct PINCONFIGALL led19red   = {(volatile u32 *)GPIOC,  4, OUT_PP, MHZ_2};

int SENSORgpiopins_Config(void)
{
	int err;

	/* LEDs set for push-pull output, slowest speed */
	err =  pinconfig_all( (struct PINCONFIGALL *)&led21green);
	err |= pinconfig_all( (struct PINCONFIGALL *)&led20red);
	err |= pinconfig_all( (struct PINCONFIGALL *)&led19red);

	return err;

}
/* ************************************************************************************************************** 
 * void configure_pin ( volatile u32 * p, int n);
 * @example	: configure_pin ( (volatile u32 *)GPIOD, 12); // Configures PD12 for pushpull output
 * @brief	: configure the pin push/pull output
 * @param	: pointer to port base
 * @param	: bit in port to set or reset
 **************************************************************************************************************** */
void configure_pin ( volatile u32 * p, int n)
{
		
	if (n >= 8)
	{ // Here, the high byte register (CRH), else low byte register (CRL)
		p = p + 0x01;	// point to high register
		n -= 8;		// adjust shift count 
	}

	/* Reset CNF bits */
	*p &= ~((0x000f ) << ( 4 * n ));	// Clear CNF reset bit 01 = Floating input (reset state)

	/* Set for pushpull output */
	*p |=  (( (GPIO_CNF_OUTPUT_PUSHPULL<<2) | (GPIO_MODE_OUTPUT_50_MHZ) ) << ( 4 * n ));	

	return;		
}
