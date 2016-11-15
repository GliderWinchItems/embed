/******************************************************************************
* File Name          : register_clear.c
* Date First Issued  : 06-21-2015
* Board              : F103 or F4
* Description        : Clearing registers when going from loader to app program
*******************************************************************************/
#include "libusartstm32/nvicdirect.h" 
#include "libopenstm32/can.h"
#include "../../../../svn_pod/sw_stm32/trunk/lib/libusartstm32/nvicdirect.h" 

/******************************************************************************
 * void register_clear(void);
 * @brief 	: Reset registers when going from loader to app
*******************************************************************************/
void register_clear(void)
{
	NVICICER(NVIC_USB_HP_CAN_TX_IRQ);	/* TX  interrupt (number & interrupt priority) */
	NVICICER(NVIC_USB_LP_CAN_RX0_IRQ);	/* RX0 interrupt (number & interrupt priority) */
	NVICICER(NVIC_CAN_RX1_IRQ);

	while ( (CAN_TSR(CAN1) & CAN_TSR_TME0) != 0)         // Wait for transmit mailbox 0 to be empty
		CAN_TSR(CAN1)  = 0x01;

	CAN_MCR(CAN1) = 0x00010002;
	CAN_MSR(CAN1) = 0x00000C02;
	CAN_IER(CAN1) = 0;
	while ((CAN_RF0R(CAN1) & 0x3) != 0) CAN_RF0R(CAN1) = 0X18;
	while ((CAN_RF1R(CAN1) & 0x3) != 0) CAN_RF1R(CAN1) = 0X18;

	CAN_ESR(CAN1) = 0;
	CAN_BTR(CAN1) = 0x01230000;

	return;
}
