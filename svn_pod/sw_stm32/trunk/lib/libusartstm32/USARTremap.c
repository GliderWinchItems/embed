/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : USARTremap.c
* Hackor             : deh
* Date First Issued  : 07/14/2011 deh
* Description        : Post 'init remapping of USARTs
*******************************************************************************/
/* NOTE:
This routine is to be called *after* the usual 'init routine has setup the USART.  This
routine changes the gpio configuration TX pin that was setup in the 'init routine back
to the "input, floating" condition.  The other pins, e.g, RX, were not changed in the
'init routine so nothing needs to be done.

This routine expects the gpio pin configuration to be in the reset condition (input, floating).

Furthermore, both the 'init and this routine must be executed before another routine
that uses the un-remapped pins as the first call to the USART initialization will configure
the pins without remapping and the second call to this routine to remap the pins will 
configure the remapped pins, but not "un-configure" the un-remapped pins.  Therefore, if
the un-remapped pins are used for something else they need to be configured following the
usart initialization and this remapping. 

This routine returns zero if the call was OK.  If the remap code is zero, no 
remapping is needed as the 'init routine has already setup the gpio pins.

Remember that all pins in the USART are remapped.  Only the TX pin needs to be setup.

*/


#include "../libopenstm32/usart.h"
#include "../libopenstm32/gpio.h"
#include "../libopenstm32/rcc.h"

#include "../libusartstm32/usartprotoprivate.h" // Subroutine prototypes for internal use

/*******************************************************************************
 * u16 USARTremap (u32 usartx, u8 u8Code);
 * @brief	: Remaps the USART (see p 166, 170-172 Ref Manual)
 * @param	: u32 USART base address, e.g. 'USART1'...'UART5' (see libopenstm32/usart.h)
 * @param	: u8  Code for remapping: USART1,2: 0 or 1 USART3: 0,1,or 3; UART4,5 not remapped 
 * @return	: 0 == OK, not 0 for error: 1=USART3 code, 2=UART4,5 don't remap, 3=USART1,2 remap, 4=base address (p172)
 *******************************************************************************/
u16 USARTremap (u32 usartx, u8 u8Code)
{
	switch (usartx)
	{


		case USART1:	// Two possibilities
			if (u8Code == 0) return 0;		// Return since no action required.
			if (u8Code >  1) return 3;		// Code is out of range

	/* Enable bus clocking for alternate function */
	RCC_APB2ENR |= (RCC_APB2ENR_AFIOEN);		// (p 103) 
	AFIO_MAPR |= AFIO_MAPR_USART1_REMAP;	// (p 170, 172)

	/* Revert pin back to reset state: GPIO_USART1 tx (PA9) (See Ref manual, page 158) */
	GPIO_CRH(GPIOA) &= ~((0x000f ) << (4*1));		// Clear existing CNF bits
	GPIO_CRH(GPIOA) |=  (( (GPIO_CNF_INPUT_FLOAT<<2) | (GPIO_MODE_INPUT) ) << (4*1));			
			
	/* Set remap pins: GPIO_USART1 tx (PB6) (See Ref manual, page 158) */
	GPIO_CRL(GPIOB) &= ~((0x000f ) << (4*6));	// Clear CNF bits
	GPIO_CRL(GPIOB) |=  (( (GPIO_CNF_OUTPUT_ALTFN_PUSHPULL<<2) | (GPIO_MODE_OUTPUT_50_MHZ) ) << (4*6));			
			break;


		case USART2: // Two possibilities
			if (u8Code == 0) return 0;		// Return since no action required.
			if (u8Code > 1 ) return 3;		// Code is out of range

	/* Enable bus clocking for alternate function */
	RCC_APB2ENR |= (RCC_APB2ENR_AFIOEN);		// (p 103) 
	AFIO_MAPR |= AFIO_MAPR_USART2_REMAP;// (p 170, 172)

	/* Revert pin back to reset state: GPIO_USART2 tx (PA2) (See Ref manual, page 157) */
	GPIO_CRL(GPIOA) &= ~((0x000f ) << (4*2));	// // Clear existing CNF bits
	GPIO_CRL(GPIOA) |=  (( (GPIO_CNF_INPUT_FLOAT<<2) | (GPIO_MODE_INPUT) ) << (4*2));

	/* Set remap pins: GPIO_USART2 tx (PD5) (See Ref manual, page 157) */
	GPIO_CRL(GPIOD) &= ~((0x000f ) << (4*5));	// Clear CNF bits
	GPIO_CRL(GPIOD) |=  (( (GPIO_CNF_OUTPUT_ALTFN_PUSHPULL<<2) | (GPIO_MODE_OUTPUT_50_MHZ) ) << (4*5));

			break;


		case USART3:	// Three possibilities
			if (u8Code == 0) return 0;		// Return since no action required.
			if ( (u8Code == 2) || (u8Code > 3) ) return 1;// Illegal remap code

			/* Enable bus clocking for alternate function */
			RCC_APB2ENR |= (RCC_APB2ENR_AFIOEN);	// (p 103) 
			AFIO_MAPR |= (u8Code<<4);		// Set remap code (p 172)

	/* Revert previously configured pin back to reset state: GPIO_USART3 tx (PB10) (See Ref manual, page 158) */
	GPIO_CRH(GPIOB) &= ~((0x000f ) << (4*2));	// Clear CNF bits
	GPIO_CRH(GPIOB) |=  (( (GPIO_CNF_INPUT_FLOAT<<2) | (GPIO_MODE_INPUT) ) << (4*2));

			if ( u8Code == 1)
			{
	/* Set remap pins: GPIO_USART3 tx (PC10) (See Ref manual, page 157) */
	GPIO_CRH(GPIOC) &= ~((0x000f ) << (4*2));	// Clear CNF bits
	GPIO_CRH(GPIOC) |=  (( (GPIO_CNF_OUTPUT_ALTFN_PUSHPULL<<2) | (GPIO_MODE_OUTPUT_50_MHZ) ) << (4*2));

			}
			else
			{ // Here, code == 3
	/* Set remap pins: GPIO_USART3 tx (PD8) (See Ref manual, page 157) */
	GPIO_CRL(GPIOD) &= ~((0x000f ) << (4*0));	// Clear CNF bits
	GPIO_CRL(GPIOD) |=  (( (GPIO_CNF_OUTPUT_ALTFN_PUSHPULL<<2) | (GPIO_MODE_OUTPUT_50_MHZ) ) << (4*0));

			}
			break;


		case UART4:
		case UART5:
			if (u8Code == 0) return 0;		// Return since no action required.
			else	return 2;			// UART4 and 5 cannot be remapped
			break;
		default:
			return 4;	// Base address did not match up
	}
		return 0;	// Remapping was OK.
}

