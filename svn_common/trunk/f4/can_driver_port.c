/******************************************************************************
* File Name          : can_driver_port.c
* Date First Issued  : 05-29-2015
* Board              : F4 or F373
* Description        : CAN port pin setup for 'can_driver.c'
*******************************************************************************/
#include "../can_driver_port.h"

#include "libopencm3/stm32/f4/gpio.h"
#include "libopencm3/stm32/f4/rcc.h"
#include "libopencm3/stm32/can.h"
#include "DISCpinconfig.h"
#include "libopencm3/stm32/nvic.h"

/******************************************************************************
 struct's for configuring CAN pins
*******************************************************************************/
//  CAN1 input pin configuration  */
static const struct PINCONFIG	inputpup = { \
	GPIO_MODE_AF,	// mode: Input alternate function 
	0, 			// output type: not applicable 		
	0, 			// speed: not applicable
	GPIO_PUPD_PULLUP, 	// pull up/down: pullup 
	GPIO_AF9 };		// AFRLy & AFRHy selection

//  CAN1 output pin configuration */
static const struct PINCONFIG	outputaf = { \
	GPIO_MODE_AF, 		// mode: Output alternate function
	GPIO_OTYPE_PP, 		// output type: push pull	
	GPIO_OSPEED_100MHZ, 	// speed: fastest 
	GPIO_PUPD_NONE, 	// pull up/down: none
	GPIO_AF9 };		// AFRLy & AFRHy selection

/******************************************************************************
 * int can_driver_port(u8 port, u8 cannum );
 * @brief 	: Setup CAN pins and hardware
 * @param	: port = port code number
 * @param	: cannum = CAN num (1, 2 for CAN1 or CAN2)
 * @return	: 0 = success; not zero = failed miserably.
 *		: -1 = cannum: not CAN1 nor CAN2 (i.e. 1 or 2)
 *		: -2 = port out-of-range: CAN1
 *		: -3 = port out-of-range: CAN2
*******************************************************************************/
int can_driver_port(u8 port, u8 cannum)
{
	//* CAN1 module needed for filters with CAN2 */
	RCC_APB1ENR |= (1<<25); // Enable clocking to CAN1 module

	/* Configure port pins */
	if (cannum == 1)
	{ // Setup pins for CAN1
		switch (port)
		{
		case 0:	// CAN1 on port A
	
		/*  Setup CAN TXD: PA12 for alternate function push/pull output */
		f4gpiopins_Config ((volatile u32*)GPIOA, 12, (struct PINCONFIG*)&outputaf);
	
		/* Setup CAN RXD: PB11 for input pull up */
		f4gpiopins_Config ((volatile u32*)GPIOA, 11, (struct PINCONFIG*)&inputpup);
	
			break;
	
		case 2:	// CAN1 on port B
	
		/*  Setup CAN TXD: PB09 for alternate function push/pull output  */
		f4gpiopins_Config ((volatile u32*)GPIOB,  9, (struct PINCONFIG*)&outputaf);
	
		/* Setup CAN RXD: PB08 for input pull up */
		f4gpiopins_Config ((volatile u32*)GPIOB,  8, (struct PINCONFIG*)&inputpup);
	
			break;
	
		case 3:	// CAN1 on port D
	
		/*  Setup CAN TXD: PD01 for alternate function push/pull  */
		f4gpiopins_Config ((volatile u32*)GPIOD,  1, (struct PINCONFIG*)&outputaf);
	
		/* Setup CAN RXD: PD00 for input pull up */
		f4gpiopins_Config ((volatile u32*)GPIOD,  0, (struct PINCONFIG*)&inputpup);
	
			break;

		default: // port number code does not match for CAN1
			return -2;
		}
		return 0; // Success!
	}
	/* Setup pins for CAN2 */
	if (cannum == 2)
	{
		RCC_APB1ENR |= (1<<26); // Enable clocking to CAN2 module
		switch (port)
		{
		case 4:	// CAN2 on port B 8|9
	
			/*  Setup CAN TXD: PB09 for alternate function push/pull output  */
			f4gpiopins_Config ((volatile u32*)GPIOB,  9, (struct PINCONFIG*)&outputaf);
	
			/* Setup CAN RXD: PB08 for input pull up */
			f4gpiopins_Config ((volatile u32*)GPIOB,  8, (struct PINCONFIG*)&inputpup);
		
			break;
		case 5: // CAN2 on port B 12|13
	
			/*  Setup CAN TXD: PB13 for alternate function push/pull output  */
			f4gpiopins_Config ((volatile u32*)GPIOB, 13, (struct PINCONFIG*)&outputaf);
	
			/* Setup CAN RXD: PB12 for input pull up */
			f4gpiopins_Config ((volatile u32*)GPIOB, 12, (struct PINCONFIG*)&inputpup);
			break;
	
		default: // port number code does not match for CAN2
			return -3;
		}
		return 0; // Success!
	}


	return -1; // Fail: cannum: not CAN1 nor CAN2 (i.e. 1 or 2)
}
