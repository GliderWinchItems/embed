/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : pinconfig_all.c
* Hackeroo           : deh
* Date First Issued  : 01/21/2013
* Board              : STM32F103xx
* Description        : Configure gpio port pins
*******************************************************************************/

#include "libopenstm32/rcc.h"
#include "libopenstm32/gpio.h"
#include "pinconfig_all.h"

/* ************************************************************************************************************** 
 * int pinconfig_all (struct PINCONFIGALL *p);
 * @brief	: configure a pin according to the parameters in the struct
 * @param	: p = pointer to struct (see .h file)
 * @return	: negative = error; 0 = OK
 **************************************************************************************************************** */
int pinconfig_all (struct PINCONFIGALL *p)
{
	u8 mode;	// Temp for building 'mode'
	u8 cnf;		// Temp for building 'cnf'
	u8 n = p->pin;			// bit number for port pin.  Save for later
	volatile u32 *pp = p->port;	// port register address.  Save for later
	
	/* Check for out-of-bounds parameters */
	if (p->pin > 15) return -3;	// Pin number out of range
	if (p->speed > 3) return -2;	// Speed code out of range
	if (p->usecode > 7) return -1;	// Useage code out of range

	/* For input pull-up/dn set ODR register first */
	if (p->usecode == IN_PU)
		// Here, input with pull up, set bit in ODR register
		GPIO_BSRR((unsigned int)p->port) = (1 << p->pin); // Set bit
	if (p->usecode == IN_PD)
		// Here, input with pull down, clear bit in ODR register
		GPIO_BRR((unsigned int)p->port) = (1 << p->pin); // Reset bit
	
	/* Adjust for port address for high and low registers that have the CNF|MODE bits. */
	if (n >= 8)	// Is this pin in the high or low register?
	{ // Here, the high byte register (CRH), else low byte register (CRL)
		pp = pp + 0x01;	// point to high register (pins 8-15)
		n -= 8;		// adjust shift count 
	}

	/* Reset CNF & MODE bits */
	*pp &= ~((0x000f ) << ( 4 * n ));	// Clear CNF reset bit 01 = Floating input (reset state)

	/* Derive 'mode' code from use code */
	if (p->usecode < 4)	// Is this an output useage?
	{ // Here, yes.
		if (p->speed == 0) return -4;	// Outputs require a speed code
		mode = p->speed;	// mode 1,2,3
		cnf = p->usecode; 	// cnf = 0-3.
	}
	else
	{ // Here, input use
		mode = 0;
		cnf = (p->usecode - 4);	// cnf = 0-3
		if (p->usecode > 5)	// 6 or 7 = input pull up/dn
			cnf = 2;	// Same cnf code for pull up & pull dn.
	}

	/* Set cnf and mode */
	*pp |=  (( (cnf << 2) | (mode) ) << ( 4 * n ));	
		
	return 0;
}

