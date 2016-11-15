/******************************************************************************
* File Name          : register_clear.c
* Date First Issued  : 06-21-2015
* Board              : F103 or F4
* Description        : Clearing registers when going from loader to app program
*******************************************************************************/
#include "nvicdirect.h"
#include "libopencm3/stm32/can.h"
/******************************************************************************
 * static void can_registers_common(u32 canaddr);
 * @brief 	: Reset registers when going from loader to app
 * @param	: canaddr = base address for CAN module
*******************************************************************************/
static void can_registers_common(u32 canaddr)
{
	CAN_BTR(canaddr) = 0;
	CAN_MCR(canaddr) = 0x00010002;
	CAN_MSR(canaddr) = 0x00000C02;
	CAN_TSR(canaddr) = 0x1C000000;
	while ((CAN_RF0R(canaddr) & 0x3) != 0) CAN_RF0R(canaddr) = 0X38;
	while ((CAN_RF1R(canaddr) & 0x3) != 0) CAN_RF1R(canaddr) = 0X38;
	CAN_IER(canaddr) = 0;
	CAN_ESR(canaddr) = 0;
	CAN_BTR(canaddr) = 0x01230000;
	return;
}

/******************************************************************************
 * void register_clear(void);
 * @brief 	: Reset registers when going from loader to app
*******************************************************************************/
void register_clear(void)
{
	/* NVIC */
	NVICICER(NVIC_CAN1_TX_IRQ);	/* TX  interrupt number */
	NVICICER(NVIC_CAN1_RX0_IRQ);	/* RX0 interrupt number */
	NVICICER(NVIC_CAN1_RX1_IRQ);	/* RX1 interrupt number */

	NVICICER(NVIC_CAN2_TX_IRQ);	/* TX  interrupt number */
	NVICICER(NVIC_CAN2_RX0_IRQ);	/* RX0 interrupt number */
	NVICICER(NVIC_CAN2_RX1_IRQ);	/* RX1 interrupt number */

	/* CAN1 registers */
	can_registers_common(CAN1);
	can_registers_common(CAN2);

	return;
}
