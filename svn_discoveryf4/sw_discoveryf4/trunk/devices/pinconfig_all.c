/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : pinconfig_all.c
* Hackeroo           : deh
* Date First Issued  : 01/21/2013
* Board              : STM32F103xx
* Description        : Configure gpio port pins
*******************************************************************************/

//#include "rcc.h"
//#include "gpio.h"
#include "pinconfig_all.h"

#define	ASSERT(boolean) if(!(boolean)) { while(1==1); }

/* ************************************************************************************************************** 
 * void pinconfig_all (struct PINCONFIGALL *p);
 * @brief	: configure a pin according to the parameters in the struct
 * @param	: p = pointer to struct (see .h file)
 * @return	: negative = error; 0 = OK
 **************************************************************************************************************** */
void pinconfig_all (struct PINCONFIGALL *p)
{
	u8 n = p->pin;			// bit number for port pin.  Save for later
	volatile u32 *pp = p->port;	// port register base address.  Save for later

	
	/* Check for out-of-bounds parameters */
	ASSERT(p->port >= (volatile u32 *)GPIOA); // IO register within bounds
	ASSERT(p->port <= (volatile u32 *)GPIOI);
	ASSERT(p->pin < 16);	// Pin number out of range
	ASSERT(p->speed < 4);	// Speed code out of range
	ASSERT(p->usecode < 8);	// Useage code out of range
	ASSERT(p->pullcode < 3);// Pull up/dn code out of range

	/* Determine mode bits */
/* p 198 Sec 7.4.1 GPIO port mode register (GPIOx_MODER) (x = A..I/)
These bits are written by software to configure the I/O direction mode.
00: Input (reset state)
01: General purpose output mode
10: Alternate function mode
11: Analog mode
*/
	/* Table for converting pin use to mode code */
	static const u8 modex[8] = {1,1,2,2,3,0,0,0};

/* 7.4.1 GPIO port mode register (GPIOx_MODER) (x = A..I/) */
	*pp &= ~( 0x3 << (p->pin * 2));		// Clear mode code
	*pp |= ((modex[p->usecode]) << (p->pin * 2));	// Set mode code

	/* */
	if (p->usecode < 4)
	{ // Here, an output use of some type

		/* Type of output code */
/* 7.4.2 GPIO port output type register (GPIOx_OTYPER) (x = A..I/) */
		if ((p->usecode == OUT_OD) || (p->usecode == OUT_AF_OD))
			*(pp + 1) |=  (1 << (p->pin * 1));	// Set bit
		else
			*(pp + 1) &= ~(1 << (p->pin * 1));	// Clear bit

		/* Output pin speed */
/* 7.4.3 GPIO port output speed register (GPIOx_OSPEEDR) (x = A..I/) */

		*(pp + 2) &= ~(0x3 << (p->pin * 2));	// Clear speed code
		*(pp + 2) |= (p->speed << (p->pin * 2));	// Set speed code
	}
/*
These bits are written by software to configure the I/O pull-up or pull-down
00: No pull-up, pull-down
01: Pull-up
10: Pull-down
11: Reserved
7.4.4 GPIO port pull-up/pull-down register (GPIOx_PUPDR) (x = A..I/) */
	*(pp + 3) &= ~(0x3 << (p->pin * 2));	// Clear pull code
	*(pp + 3) |= (p->pullcode << (p->pin * 2));	// Set new pull code
	
	/* Adjust for port address for high and low registers that have the CNF|MODE bits. */
/* 7.4.9 GPIO alternate function low register (GPIOx_AFRL) (x = A..I/) */
	pp = (p->port + 0x20/4);	// Point to alternate function low register
	if (n >= 8)	// Is this pin in the high or low register?
	{ // Here, the high byte register (CRH), else low byte register (CRL)
/* 7.4.10 GPIO alternate function high register (GPIOx_AFRH) (x = A..I/) */
		pp = pp + 0x01;	// point to high register (pins 8-15)
		n -= 8;		// adjust shift count 
	}

	/* Setup alternate function bits */
	*pp &= ~((0x000f ) << ( 4 * n ));	// Clear Alternate function bits
	*pp |=  ((p->afcode) << ( 4 * n ));
	
	return;
}

