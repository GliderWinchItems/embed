/******************************************************************************
* File Name          : can_driver_port.c
* Date First Issued  : 05-29-2015
* Board              : F103
* Description        : CAN port pin setup for 'can_driver.c' for F103
*******************************************************************************/
#include "../can_driver_port.h"
#include "../../../svn_pod/sw_stm32/trunk/lib/libopenstm32/gpio.h"
#include "../../../svn_pod/sw_stm32/trunk/lib/libopenstm32/rcc.h"
#include "../../../svn_pod/sw_stm32/trunk/devices/pinconfig_all.h"
/******************************************************************************
 * int can_driver_port(u8 port, u8 cannum );
 * @brief 	: Setup CAN pins and hardware
 * @param	: port = port code number
 * @param	: cannum = CAN num (0 or 1, for CAN1 or CAN2)
 * @return	: 0 = success; not zero = failed miserably.
 *		: -1 = cannum: not CAN1 
*******************************************************************************/
int can_driver_port(u8 port, u8 cannum)
{
	if (cannum != 1) return -1;	// ONLY CAN1 allowed on F103

	/* Enable clocking to CAN module */
	RCC_APB1ENR |= (1<<25);		// CAN1_EN p 144

	RCC_APB2ENR |= RCC_APB2ENR_AFIOEN;	// enable clock for Alternate Function

	/* REMAP p 180
	Bits 14:13 CAN_REMAP[1:0]: CAN alternate function remapping
            These bits are set and cleared by software. They control the mapping of alternate functions
            CAN_RX and CAN_TX in devices with a single CAN interface.
            00: CAN_RX mapped to PA11, CAN_TX mapped to PA12
            01: Not used
            10: CAN_RX mapped to PB8, CAN_TX mapped to PB9 (not available on 36-pin package)
            11: CAN_RX mapped to PD0, CAN_TX mapped to PD1 
	OLIMEX = 0x2
	*/

/* Output pins configure for alternate function output push-pull */
const struct PINCONFIGALL pa12 = {(volatile u32 *)GPIOA, 12, OUT_AF_PP, MHZ_50};
const struct PINCONFIGALL pb9  = {(volatile u32 *)GPIOB,  9, OUT_AF_PP, MHZ_50};
const struct PINCONFIGALL pd1  = {(volatile u32 *)GPIOD,  1, OUT_AF_PP, MHZ_50};

/* Input pins configure for input pull-up */
const struct PINCONFIGALL pa11 = {(volatile u32 *)GPIOA, 11, IN_PU, 0};
const struct PINCONFIGALL pb8  = {(volatile u32 *)GPIOB,  8, IN_PU, 0};
const struct PINCONFIGALL pd0  = {(volatile u32 *)GPIOD,  0, IN_PU, 0};

	/* Setup remapping and configure port pins */
	switch (port)
	{
	case 0:	// CAN on port A

		/*  Setup CAN TXD: PA12 for alternate function push/pull output p 156, p 163  */
		pinconfig_all((struct PINCONFIGALL *)&pa12);

		/* Setup CAN RXD: PB11 for input pull up p 164, 167 */
		pinconfig_all((struct PINCONFIGALL *)&pa11);

		AFIO_MAPR |= (0x0 << 13);	 // 00: CAN_RX mapped to PA11, CAN_TX mapped to PA12 p 179
		break;

	case 1:	// CAN on port B

		/*  Setup CAN TXD: PB9 for alternate function push/pull output p 156, p 163  */
		pinconfig_all((struct PINCONFIGALL *)&pb9);

		/* Setup CAN RXD: PB8 for input pull up p 164, 167 */
		pinconfig_all((struct PINCONFIGALL *)&pb8);

		AFIO_MAPR |= (0x2 << 13);	 // 00: CAN_RX mapped to PA11, CAN_TX mapped to PA12 p 179
		break;

	case 2:	// CAN on port D

		/*  Setup CAN TXD: PD1 for alternate function push/pull output p 156, p 163  */
		pinconfig_all((struct PINCONFIGALL *)&pd1);

		/* Setup CAN RXD: PD0 for input pull up p 164, 167 */
		pinconfig_all((struct PINCONFIGALL *)&pd0);

		AFIO_MAPR |= (0x3 << 13);	 // 00: CAN_RX mapped to PA11, CAN_TX mapped to PA12 p 179
		break;
	default:
		return -2;
	}
	return 0;
}
