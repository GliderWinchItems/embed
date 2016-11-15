/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : pinconfig.c
* Hackeroo           : deh
* Date First Issued  : 03/15/2012
* Board              : STM32F103VxT6
* Description        : Configure gpio port pins
*******************************************************************************/

#include "libopenstm32/rcc.h"
#include "libopenstm32/gpio.h"

/* ************************************************************************************************************** 
 * void configure_pin_input_pull_up_dn ( volatile u32 * p, int n, int up);
 * @example	: configure_pin ( (volatile u32 *)GPIOD, 12, 1); // Configures PD12 for input with pull up
 * WARNING: Do not forget (volatile u32 *)!
 * @brief	: Configures pin for input with pull up or down
 * @param	: pointer to port base
 * @param	: bit in port to set or reset
 * @parm	: 0 = pull down, 1 = pull up.
 **************************************************************************************************************** */
void configure_pin_input_pull_up_dn ( volatile u32 * p, int n, int up)
{

	/* Set pull up or pull down */
	if ( up == 0)
	{ // Here, pull down, clear bit in ODR register
		GPIO_BRR((unsigned int)p) = (1 << n);	// Reset bit
	}
	else
	{ // Here, pull up, set bit in ODR register
		GPIO_BSRR((unsigned int)p) = (1 << n);// Set bit
	}


	if (n >= 8)	// Is this pin in the high or low register?
	{ // Here, the high byte register (CRH), else low byte register (CRL)
		p = p + 0x01;	// point to high register
		n -= 8;		// adjust shift count 
	}

	/* Reset CNF and MODE bits for pin 'n' */
	*p &= ~((0x000f ) << ( 4 * n ));	// Clear CNF reset bit 01 = Floating input (reset state)

	/* Set configuration for input pull up/down */
	//             CNF = 1 0                       MODE = 00
	*p |=  (( (GPIO_CNF_INPUT_PULL_UPDOWN<<2) | (GPIO_MODE_INPUT) ) << ( 4 * n ));	


	return;
}
