/************************************************************
07-14-2011 - Adds UART4,5
...
10/02/2012 - CAN interrupts
*************************************************************/

#include "libopenstm32/scb.h"
#include "libopenstm32/nvic.h"
#include "panic_leds_pod.h"

/*
 * This file is part of the libopenstm32 project.
 *
 * Copyright (C) 2010 Piotr Esden-Tempski <piotr@esden.net>
 * Modified 9/22/2010: D.E. Haselwood <dhaselwood@ieee.org>
 *  Changing naming to be more CMSIS-like.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define WEAK __attribute__ ((weak))

void main(void);
void Reset_Handler(void);
void blocking_handler1(void);
void blocking_handler2(void);
void blocking_handler3(void);
void blocking_handler4(void);
void null_handler(void);

void WEAK nmi_handler(void);
void WEAK hard_fault_handler(void);
void WEAK mem_manage_handler(void);
void WEAK bus_fault_handler(void);
void WEAK usage_fault_handler(void);
void WEAK sv_call_handler(void);
void WEAK debug_monitor_handler(void);
void WEAK pend_sv_handler(void);
void WEAK SYSTICK_IRQHandler(void); // sys_tick_handler(void);
void WEAK wwdg_isr(void);
void WEAK pvd_isr(void);
void WEAK tamper_isr(void);
void WEAK p1_RTC_IRQHandler(void);	    // rtc_isr(void);
void WEAK flash_isr(void);
void WEAK rcc_isr(void);
void WEAK exti0_isr(void);
void WEAK exti1_isr(void);
void WEAK exti2_isr(void);
void WEAK exti3_isr(void);
void WEAK exti4_isr(void);
void WEAK DMA1CH1_IRQHandler_tension(void);	//DMA1CH1_IRQHandler(void);	// dma1_channel1_isr(void);
void WEAK DMA1CH2_IRQHandler(void); 	// dma1_channel2_isr(void);
void WEAK dma1_channel3_isr(void);
void WEAK DMA1CH4_IRQHandler(void); 	// dma1_channel4_isr(void);
void WEAK DMA1CH5_IRQHandler(void); 	// dma1_channel5_isr(void);
void WEAK dma1_channel6_isr(void);
void WEAK DMA1CH7_IRQHandler(void); 	// dma1_channel7_isr(void);
void WEAK ADC1_2_IRQHandler(void);	// adc1_2_isr(void);
void WEAK CAN1_TX_IRQHandler(void);	//USB_HP_CAN_TX_IRQHandler(void);
void WEAK CAN1_RX0_IRQHandler(void);	//USB_LP_CAN_RX0_IRQHandler(void);
void WEAK CAN1_RX1_IRQHandler(void);	//CAN_RX1_Handler(void);
void WEAK CAN_SCE_Handler(void);
void WEAK exti9_5_isr(void);
void WEAK p1_TIM1_BRK_IRQHandler(void);	// tim1_brk_isr(void);
void WEAK p1_TIM1_UP_IRQHandler(void);	// tim1_up_isr(void);
void WEAK p1_TIM1_TRG_COM_IRQHandler(void);// tim1_trg_com_isr(void);
void WEAK p1_TIM1_CC_IRQHandler(void);	// tim1_cc_isr(void);
void WEAK TIM2_IRQHandler(void);	// tim2_isr(void);
void WEAK TIM3_IRQHandler(void);	// tim3_isr(void);
void WEAK TIM4_IRQHandler(void);	// tim4_isr(void);
void WEAK I2C1_EV_IRQHandler(void);	// i2c1_ev_isr(void);
void WEAK I2C1_ER_IRQHandler_ten(void);	// i2c1_er_isr(void);
void WEAK I2C2_EV_IRQHandler(void);	// i2c2_ev_isr(void);
void WEAK I2C2_ER_IRQHandler(void);	// i2c2_er_isr(void);
void WEAK SPI1_TEN_IRQHandler(void);	// spi1_isr(void);
void WEAK SPI2_IRQHandler(void);	// spi2_isr(void);
void WEAK USART1_IRQHandler(void);	// usart1_isr(void);
void WEAK USART2_IRQHandler(void);	// usart2_isr(void);
void WEAK USART3_IRQHandler(void);	// usart3_isr(void);
void WEAK exti15_10_isr(void);
void WEAK p1_RTC_ALARM_IRQHandler(void);	// rtc_alarm_isr(void);
void WEAK usb_wakeup_isr(void);
void WEAK tim8_brk_isr(void);
void WEAK tim8_up_isr(void);
void WEAK tim8_trg_com_isr(void);
void WEAK tim8_cc_isr(void);
void WEAK ADC3_IRQHandler(void);	// adc3_isr(void);
void WEAK FSMC_IRQHandler_ten(void);	//fsmc_isr(void);
void WEAK sdio_isr(void);
void WEAK TIM5_IRQHandler(void);
void WEAK spi3_isr(void);
void WEAK UART4_IRQHandler(void);	// usart4_isr(void);
void WEAK UART5_IRQHandler(void);	// usart5_isr(void);
void WEAK TIM6_IRQHandler(void);	// tim6_isr(void);
void WEAK TIM7_IRQHandler(void);	// tim7_isr(void);
void WEAK dma2_channel1_isr(void);
void WEAK dma2_channel2_isr(void);
void WEAK DMA2CH3_IRQHandler(void);
void WEAK DMA2CH4_IRQHandler(void);	// dma2_channel4_5_isr(void);

__attribute__ ((section(".vectors")))
void (*const vector_table[]) (void) = {
//	(void *)0x20000800,	/* Use 2KB stack (0x800 bytes). */
	(void *)(0x20000000+(20*1024)-4),/* Put stack at top */
//	main,			/* Use main() as reset vector for now. */
	Reset_Handler,		/* Either this of main */
	nmi_handler,
	hard_fault_handler,
	mem_manage_handler,
	bus_fault_handler,
	usage_fault_handler,
	0, 0, 0, 0,		/* Reserved */
	sv_call_handler,
	debug_monitor_handler,
	0,			/* Reserved */
	pend_sv_handler,
	SYSTICK_IRQHandler,	//sys_tick_handler,
	wwdg_isr,
	pvd_isr,
	tamper_isr,
	p1_RTC_IRQHandler,		//rtc_isr,
	flash_isr,
	rcc_isr,
	exti0_isr,
	exti1_isr,
	exti2_isr,
	exti3_isr,
	exti4_isr,
	DMA1CH1_IRQHandler_tension, //DMA1CH1_IRQHandler,	//dma1_channel1_isr,
	DMA1CH2_IRQHandler,	//dma1_channel2_isr
	dma1_channel3_isr,
	DMA1CH4_IRQHandler,	//dma1_channel4_isr
	DMA1CH5_IRQHandler,	//dma1_channel5_isr,
	dma1_channel6_isr,
	DMA1CH7_IRQHandler,	//dma1_channel7_isr
	ADC1_2_IRQHandler,	//adc1_2_isr,
	CAN1_TX_IRQHandler,	//USB_HP_CAN_TX_IRQHandler, //usb_hp_can_tx_isr,
	CAN1_RX0_IRQHandler,	//USB_LP_CAN_RX0_IRQHandler,//usb_lp_can_rx0_isr,
	CAN1_RX1_IRQHandler,	//CAN_RX1_Handler,	//can_rx1_isr,
	CAN_SCE_Handler,	//can_sce_isr, 
	exti9_5_isr,
	p1_TIM1_BRK_IRQHandler,	//tim1_brk_isr,
	p1_TIM1_UP_IRQHandler,	//tim1_up_isr,
	p1_TIM1_TRG_COM_IRQHandler,//tim1_trg_com_isr,
	p1_TIM1_CC_IRQHandler,	//tim1_cc_isr,
	TIM2_IRQHandler,	//tim2_isr,
	TIM3_IRQHandler,	//tim3_isr,
	TIM4_IRQHandler,	//tim4_isr,
	I2C1_EV_IRQHandler,	//i2c1_ev_isr,
	I2C1_ER_IRQHandler_ten,	//i2c1_er_isr,
	I2C2_EV_IRQHandler,	//i2c2_ev_isr,
	I2C2_ER_IRQHandler,	//i2c2_er_isr,
	SPI1_TEN_IRQHandler,	//spi1_isr,
	SPI2_IRQHandler,	//spi2_isr,
	USART1_IRQHandler,	//usart1_isr,
	USART2_IRQHandler,	//usart2_isr,
	USART3_IRQHandler,	//usart3_isr,
	exti15_10_isr,
	p1_RTC_ALARM_IRQHandler,	// rtc_alarm_isr,
	usb_wakeup_isr,
	tim8_brk_isr,
	tim8_up_isr,
	tim8_trg_com_isr,
	tim8_cc_isr,
	ADC3_IRQHandler,	// adc3_isr,
	FSMC_IRQHandler_ten,	//fsmc_isr,
	sdio_isr,
	TIM5_IRQHandler,	// tim5_isr,
	spi3_isr,
	UART4_IRQHandler,	// usart4_isr,
	UART5_IRQHandler,	// usart5_isr,
	TIM6_IRQHandler,	// tim6_isr,
	TIM7_IRQHandler,	// tim7_isr,
	dma2_channel1_isr,
	dma2_channel2_isr,
	DMA2CH3_IRQHandler,	//dma2_channel3_isr
	DMA2CH4_IRQHandler,	//dma2_channel4_5_isr,
};

void hard_fault_handler(void)
{
	while (1) ;
}
void mem_manage_handler(void)
{
	while (1) ;
}
void bus_fault_handler(void)
{
	while (1) ;
}
void usage_fault_handler(void)
{
	while (1) ;
}

void null_handler(void)
{
	while (1) ;
}
/* **************************************************************************************
 * void relocate_vector(void);
 * @brief	: Copy vector to ram and change
 * ************************************************************************************** */

void relocate_vector(void)
{
	/* Copy flash vector to sram */
//	unsigned char *pram   = vectorram;
//	unsigned char *pflash = vector_table;
//	while ( pflash < (unsigned char *)(&hard_fault_handler) )
//	{
//		*pram++ = *pflash++;
//	}
	
	/* Disable all interrupts */
	NVIC_ICER(0) = -1;
	NVIC_ICER(1) = -1;
	NVIC_ICER(2) = -1;

	/* Reset stack pointer */
	
	/* Move vector to beginning of sram*/
//	SCB_VTOR  = (1<<29);
	SCB_VTOR = (unsigned int)vector_table;

	return;
}


#pragma weak nmi_handler = null_handler
//#pragma weak hard_fault_handler = blocking_handler1
//#pragma weak mem_manage_handler = blocking_handler2
//#pragma weak bus_fault_handler = blocking_handler3
//#pragma weak usage_fault_handler = blocking_handler4
#pragma weak sv_call_handler = null_handler
#pragma weak debug_monitor_handler = null_handler
#pragma weak pend_sv_handler = null_handler
#pragma weak SYSTICK_IRQHandler = null_handler
#pragma weak wwdg_isr = null_handler
#pragma weak pvd_isr = null_handler
#pragma weak tamper_isr = null_handler
#pragma weak rtc_isr = null_handler
#pragma weak flash_isr = null_handler
#pragma weak rcc_isr = null_handler
#pragma weak exti0_isr = null_handler
#pragma weak exti1_isr = null_handler
#pragma weak exti2_isr = null_handler
#pragma weak exti3_isr = null_handler
#pragma weak exti4_isr = null_handler
#pragma weak DMA1CH1_IRQHandler_tension = null_handler //dma1_channel1_isr = null_handler
#pragma weak dDMA1CH2_IRQHandler = null_handler
#pragma weak dma1_channel3_isr = null_handler
#pragma weak DMA1CH4_IRQHandler = null_handler
#pragma weak dma1_channel5_isr = null_handler
#pragma weak dma1_channel6_isr = null_handler
#pragma weak DMA1CH7_IRQHandler = null_handler
#pragma weak ADC1_2_IRQHandler = null_handler
//#pragma weak CAN1_TX_IRQHandler = null_handler	// USB_HP_CAN_TX_IRQHandler = null_handler
//#pragma weak CAN1_RX0_IRQHandler = null_handler	//USB_LP_CAN_RX0_IRQHandler = null_handler
//#pragma weak CAN1_RX1_IRQHandler = null_handler	//CAN_RX1_Handler = null_handle
#pragma weak CAN_SCE_Handler = null_handler
#pragma weak exti9_5_isr = null_handler
#pragma weak p1_TIM1_BRK_IRQHandler = null_handler
#pragma weak p1_TIM1_UP_IRQHandler = null_handler
#pragma weak p1_TIM1_TRG_COM_IRQHandler = null_handler
#pragma weak p1_TIM1_CC_IRQHandler = null_handler
#pragma weak TIM2_IRQHandler = null_handler
#pragma weak tim3_isr = null_handler
#pragma weak TIM4_IRQHandler = null_handler
#pragma weak I2C1_EV_IRQHandler = null_handler
#pragma weak I2C1_ER_IRQHandler_ten = null_handler
#pragma weak I2C2_EV_IRQHandler = null_handler
#pragma weak I2C2_ER_IRQHandler = null_handler
#pragma weak SPI1_TEN_IRQHandler = null_handler	//spi1_isr = null_handler
#pragma weak spi2_isr = null_handler
#pragma weak USART1_IRQHandler = null_handler
#pragma weak USART2_IRQHandler = null_handler
#pragma weak USART3_IRQHandler = null_handler
#pragma weak exti15_10_isr = null_handler
#pragma weak rtc_alarm_isr = null_handler
#pragma weak usb_wakeup_isr = null_handler
#pragma weak tim8_brk_isr = null_handler
#pragma weak tim8_up_isr = null_handler
#pragma weak tim8_trg_com_isr = null_handler
#pragma weak tim8_cc_isr = null_handler
#pragma weak ADC3_IRQHandler = null_handler
#pragma weak FSMC_IRQHandler_ten = null_handler	//fsmc_isr = null_handler
#pragma weak sdio_isr = null_handler
#pragma weak TIM5_IRQHandler = null_handler
#pragma weak spi3_isr = null_handler
#pragma weak usart4_isr = null_handler
#pragma weak usart5_isr = null_handler
#pragma weak TIM6_IRQHandler = null_handler
#pragma weak TIM7_IRQHandler = null_handler
#pragma weak dma2_channel1_isr = null_handler
#pragma weak dma2_channel2_isr = null_handler
#pragma weak DMA2CH3_IRQHandler = null_handler
#pragma weak dma2_channel4_5_isr = null_handler
