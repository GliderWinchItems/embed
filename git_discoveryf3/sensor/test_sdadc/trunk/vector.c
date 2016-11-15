/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2010 Piotr Esden-Tempski <piotr@esden.net>,
 * Copyright (C) 2012 chrysn <chrysn@fsfe.org>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#include <libopencm3/cm3/scb.h>

/* Symbols exported by the linker script(s): */
extern unsigned _data_loadaddr, _data, _edata, _ebss, _stack;

typedef void (*funcp_t) (void);
extern funcp_t __preinit_array_start, __preinit_array_end;
extern funcp_t __init_array_start, __init_array_end;
extern funcp_t __fini_array_start, __fini_array_end;

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
void WEAK memory_manage_fault_handler(void);
void WEAK bus_fault_handler(void);
void WEAK usage_fault_handler(void);
void WEAK sv_call_handler(void);
void WEAK debug_monitor_handler(void);
void WEAK pend_sv_handler(void);
void WEAK SYSTICK_IRQHandler(void); // sys_tick_handler(void);

void WEAK wwdg_isr(void);	/* 00 */
void WEAK pvd_isr(void);	/* 01 */
void WEAK tamper_isr(void);	/* 02 */
void WEAK rtc_wkup_isr(void);	/* 03 */
void WEAK flash_isr(void);	/* 04 */
void WEAK rcc_isr(void);	/* 05 */
void WEAK exti0_isr(void);	/* 06 */
void WEAK exti1_isr(void);	/* 07 */
void WEAK exti2_isr(void);	/* 08 */
void WEAK exti3_isr(void);	/* 09 */
void WEAK exti4_isr(void);	/* 10 */
void WEAK DMA1CH1_IRQHandler(void);	/* 11 */
void WEAK DMA1CH2_IRQHandler(void);	/* 12 */
void WEAK DMA1CH3_IRQHandler(void);	/* 13 */
void WEAK DMA1CH4_IRQHandler(void);	/* 14 */
void WEAK DMA1CH5_IRQHandler(void);	/* 15 */
void WEAK DMA1CH6_IRQHandler(void);	/* 16 */
void WEAK DMA1CH7_IRQHandler(void);	/* 17 */
void WEAK adc1_2_isr(void);	/* 18 */
void WEAK usb_hp_can1_tx_isr(void);	/* 19 */
void WEAK usb_lp_can1_rx0_isr(void);	/* 20 */
void WEAK can1_rx1_isr(void);	/* 21 */
void WEAK can1_sce_isr(void);	/* 22 */
void WEAK exti9_5_isr(void);	/* 23 */
void WEAK tim15_isr(void);	/* 24 */
void WEAK tim16_isr(void);	/* 25 */
void WEAK tim17_isr(void);	/* 26 */
void WEAK tim18_isr(void);	/* 27 */
void WEAK tim2_isr(void);	/* 28 */
void WEAK tim3_isr(void);	/* 29 */
void WEAK tim4_isr(void);	/* 30 */
void WEAK i2c1_ev_isr(void);	/* 31 */
void WEAK i2c1_er_isr(void);	/* 32 */
void WEAK i2c2_ev_isr(void);	/* 33 */
void WEAK i2c2_er_isr(void);	/* 34 */
void WEAK SPI1_IRQHandler(void);	/* 35 */
void WEAK SPI2_IRQHandler(void);	/* 36 */
void WEAK USART1_IRQHandler(void);	/* 37 */
void WEAK USART2_IRQHandler(void);	/* 38 */
void WEAK USART3_IRQHandler(void);	/* 39 */
void WEAK exti15_10_isr(void);	/* 40 */
void WEAK rtc_alarm_isr(void);	/* 41 */
void WEAK usb_wakeup_isr(void);	/* 42 */
void WEAK tim12_isr(void);	/* 43 */
void WEAK tim13_isr(void);	/* 44 */
void WEAK tim14_isr(void);	/* 45 */
void WEAK tim8_cc_isr(void);	/* 46 */
void WEAK adc3_isr(void);	/* 47 */
void WEAK fsmc_isr(void);	/* 48 */
void WEAK sdio_isr(void);	/* 49 */
void WEAK tim5_isr(void);	/* 50 */
void WEAK spi3_isr(void);	/* 51 */
void WEAK UART4_IRQHandler(void);	/* 52 */
void WEAK UART5_IRQHandler(void);	/* 53 */
void WEAK tim6_isr(void);	/* 54 */
void WEAK tim7_isr(void);	/* 55 */
void WEAK DMA2CH1_IRQHandler(void);	/* 56 */
void WEAK DMA2CH2_IRQHandler(void);	/* 57 */
void WEAK DMA2CH3_IRQHandler(void);	/* 58 */
void WEAK DMA2CH4_IRQHandler(void);	/* 59 */
void WEAK DMA2CH5_IRQHandler(void);	/* 60 */
void WEAK SDADC1_IRQHandler(void);	/* 61 */
void WEAK SDADC2_IRQHandler(void);	/* 62 */
void WEAK SDADC3_IRQHandler(void);	/* 63 */
void WEAK comp1_2_3_isr(void);	/* 64 */
void WEAK comp4_5_6_isr(void);	/* 65 */
void WEAK comp7_isr(void);	/* 66 */
void WEAK reserve1_isr(void);	/* 67 */
void WEAK reserve2_isr(void);	/* 68 */
void WEAK reserve3_isr(void);	/* 69 */
void WEAK reserve4_isr(void);	/* 70 */
void WEAK reserve5_isr(void);	/* 71 */
void WEAK reserve6_isr(void);	/* 72 */
void WEAK reserve7_isr(void);	/* 73 */
void WEAK usb_hp_isr(void);	/* 74 */
void WEAK usb_lp_isr(void);	/* 75 */
void WEAK usb_wakup_isr(void);	/* 76 */
void WEAK reserve8_isr(void);	/* 77 */
void WEAK tim19_isr(void);	/* 78 */
void WEAK reserve9_isr(void);	/* 79 */
void WEAK reserve10_isr(void);	/* 80 */
void WEAK fpu_isr(void);	/* 81 */

__attribute__ ((section(".vectors")))
void (*const vector_table[]) (void) = {
	(void *)(0x20000000+0x8000),	/* 00 Put stack at top */
	 Reset_Handler,			/* 04 Startup code */
	 nmi_handler,			/* 08 CSS linked to NMI vector */
	 hard_fault_handler,		/* 0C Hard fault */
	 memory_manage_fault_handler, 	/* 10 MPU fault */
	 bus_fault_handler,           	/* 14 Prefetch or memory access error */
	 usage_fault_handler,         	/* 18 Undefined instruction or illegal state */
	 0,				/* 1C */
	 0,				/* 20 */
         0,				/* 24 */
	 0,				/* 28 */
	 sv_call_handler,		/* 2C Sys service call via SWI instruction */
	 debug_monitor_handler,       	/* 30 Debug monitor */
	 0,				/* 34 */
	 pend_sv_handler,		/* 38 Pendable request for system service */
	 SYSTICK_IRQHandler,		/* 3C Famous systick timer */

/* NOTE: "+" is for F303 addition; "-" F303 only; F373 is first, then F303 */
  wwdg_isr,		/* 00  Window Watchdog  0x40 */
  pvd_isr,		/* 01  Power voltage detector  */
  tamper_isr,		/* 02  Tamper & Timestamp  */
  rtc_wkup_isr,		/* 03  RTC  */
  flash_isr,		/* 04  Global interrupt flash  */
  rcc_isr,		/* 05  Global interrupt rcc  */
  exti0_isr,		/* 06  EXTI line 0  */
  exti1_isr,		/* 07  EXTI line 1  */
  exti2_isr,		/* 08  EXTI line 2  */
  exti3_isr,		/* 09  EXTI line 3  */
  exti4_isr,		/* 10  EXTI line 4  */
  DMA1CH1_IRQHandler,	/* 11  DMA1 channel 1  */
  DMA1CH2_IRQHandler,	/* 12  DMA1 channel 2  */
  DMA1CH3_IRQHandler,	/* 13  DMA1 channel 3  */
  DMA1CH4_IRQHandler,	/* 14  DMA1 channel 4  */
  DMA1CH5_IRQHandler,	/* 15  DMA1 channel 5  */
  DMA1CH6_IRQHandler,	/* 16  DMA1 channel 6  */
  DMA1CH7_IRQHandler,	/* 17  DMA1 channel 7  */
  adc1_2_isr,		/* 18  ADC1 & ADC2+  */
  usb_hp_can1_tx_isr,	/* 19  CAN1 TX ; USB_HP+  */
  usb_lp_can1_rx0_isr,	/* 20  CAN1 RX0; USB_LP+  */
  can1_rx1_isr,		/* 21  CAN1 RX1;  */
  can1_sce_isr,		/* 22  CAN1 SCE;  */
  exti9_5_isr,		/* 23  EXTI lines [9:5]  */
  tim15_isr,		/* 24  TIM15; TIM1_BRK+  */
  tim16_isr,		/* 25  TIM16; TIM1_UP+  */
  tim17_isr,		/* 26  TIM17; TIM1_TRG_COM  */
  tim18_isr,		/* 27  TIM18 & DAC2 underrun; TIM1_CC-  */
  tim2_isr,		/* 28  TIM2  */
  tim3_isr,		/* 29  TIM3  */
  tim4_isr,		/* 30  TIM4  */
  i2c1_ev_isr,		/* 31  I2C1_EV & EXTI lines [3:2]  */
  i2c1_er_isr,		/* 32  I2C1_ER  */
  i2c2_ev_isr,		/* 33  I2C2_EV & EXTI lines [4:2]  */
  i2c2_er_isr,		/* 34  I2C2_ER  */
  SPI1_IRQHandler,	/* 35  SPI1  */
  SPI2_IRQHandler,	/* 36  SPI2  */
  USART1_IRQHandler,	/* 37  USART1 & EXTI25  */
  USART2_IRQHandler,	/* 38  USART1 & EXTI26  */
  USART3_IRQHandler,	/* 39  USART1 & EXTI28  */
  exti15_10_isr,	/* 40  EXTI lines [15:10]  */
  rtc_alarm_isr,	/* 41  RTC_ALARM  */
  usb_wakeup_isr,	/* 42  CEC; USBWAKEUP (EXTI line 18)-  */
  tim12_isr,		/* 43  TIM12; TIM8_BRK-  */
  tim13_isr,		/* 44  TIM13; TIM8_UP-  */
  tim14_isr,		/* 45  TIM14; TIM8_TRG_COM  */
  tim8_cc_isr,		/* 46  -----; TIM8_CC-  */
  adc3_isr,		/* 47  -----; ADC3-  */
  fsmc_isr,		/* 48  Reserved (both)  */
  sdio_isr,		/* 49  Reserved (both)  */
  tim5_isr,		/* 50  TIM5; -----  */
  spi3_isr,		/* 51  SPI3  */
  UART4_IRQHandler,	/* 52  ------; UART4 & EXTI line 34  */
  UART5_IRQHandler,	/* 53  ------; UART5 & EXTI line 35  */
  tim6_isr,		/* 54  TIM6 & DAC1 underrun  */
  tim7_isr,		/* 55  TIM7  */
  DMA2CH1_IRQHandler,	/* 56  DMA2 channel 1  */
  DMA2CH2_IRQHandler,	/* 57  DMA2 channel 2  */
  DMA2CH3_IRQHandler,	/* 58  DMA2 channel 3  */
  DMA2CH4_IRQHandler,	/* 59  DMA2 channel 4  */
  DMA2CH5_IRQHandler,	/* 60  DMA2 channel 5  */
  SDADC1_IRQHandler,	/* 61  SDADC1; ADC4-  */
  SDADC2_IRQHandler,	/* 62  SDADC2; -----  */
  SDADC3_IRQHandler,	/* 63  SDADC3; -----  */
  comp1_2_3_isr,	/* 64  COMP 1,2 & EXTI lines 21,22; COMP 1,2,3 & EXTI lines 21,22,29  */
  comp4_5_6_isr,	/* 65  --------; COMP 4,5,6 & EXTI lines 30,31,32  */
  comp7_isr,		/* 66  --------; COMP 7 & EXTI line 33  */
  reserve1_isr,		/* 67  Reserved  */
  reserve2_isr,		/* 68  Reserved  */
  reserve3_isr,		/* 69  Reserved  */
  reserve4_isr,		/* 70  Reserved  */
  reserve5_isr,		/* 71  Reserved  */
  reserve6_isr,		/* 72  Reserved  */
  reserve7_isr,		/* 73  Reserved  */
  usb_hp_isr,		/* 74  USB_HP  */
  usb_lp_isr,		/* 75  USB_LP  */
  usb_wakup_isr,	/* 76  USB wakeup; USB wakup RMP & EXTI line 18  */
  reserve8_isr,		/* 77  Reserved  */
  tim19_isr,		/* 78  TIM19; -----  */
  reserve9_isr,		/* 79  Reserved  */
  reserve10_isr,	/* 80  Reserved  */
  fpu_isr,		/* 81  Floating pt unit  */
};

/* System interrupt traps. */
void hard_fault_handler(void)	{while (1) ;}
void memory_manage_fault_handler(void){while (1);}
void bus_fault_handler(void)	{while (1);}
void usage_fault_handler(void)	{while (1);}
void null_handler(void)		{while (1);}
void sv_call_handler(void)	{while (1);}
void debug_monitor_handler(void){while (1);}
void pend_sv_handler(void)	{while (1);}
void blocking_handler(void)	{while (1);}

void __attribute__ ((weak, naked)) Reset_Handler(void)
{
	volatile unsigned *src, *dest;
	funcp_t *fp;

	for (src = &_data_loadaddr, dest = &_data;
		dest < &_edata;
		src++, dest++) {
		*dest = *src;
	}

	while (dest < &_ebss) {
		*dest++ = 0;
	}

	/* Ensure 8-byte alignment of stack pointer on interrupts */
	/* Enabled by default on most Cortex-M parts, but not M3 r1 */
	SCB_CCR |= SCB_CCR_STKALIGN;

	/* might be provided by platform specific vector.c */
	/* Enable access to Floating-Point coprocessor. */
	SCB_CPACR |= SCB_CPACR_FULL * (SCB_CPACR_CP10 | SCB_CPACR_CP11);

	/* Constructors. */
	for (fp = &__preinit_array_start; fp < &__preinit_array_end; fp++) {
		(*fp)();
	}
	for (fp = &__init_array_start; fp < &__init_array_end; fp++) {
		(*fp)();
	}

	/* Call the application's entry point. */
	main();

	/* Destructors. */
	for (fp = &__fini_array_start; fp < &__fini_array_end; fp++) {
		(*fp)();
	}

}


/* The following #include has a list, of the following form-- 
void Default_Handler00(void) { Default_HandlerCode = 00; panic_leds_local(PANIC_CODE); }
*/
#include "default.c"

/* These need some review */
#pragma weak nmi_handler = null_handler
//#pragma weak hard_fault_handler = blocking_handler
//#pragma weak debug_monitor_handler = null_handler
//#pragma weak sv_call_handler = null_handler
//#pragma weak pend_sv_handler = null_handler
#pragma weak SYSTICK_IRQHandler = null_handler

/* NVIC list--trap bogus interrupts with an identifier code. 
   Each Default_Handler loads the numeric NVIC code. */
#pragma weak wwdg_isr = Default_Handler00
#pragma weak pvd_isr = Default_Handler01
#pragma weak tamper_isr = Default_Handler02
#pragma weak rtc_wkup_isr = Default_Handler03
#pragma weak flash_isr = Default_Handler04
#pragma weak rcc_isr = Default_Handler05
#pragma weak exti0_isr = Default_Handler06
#pragma weak exti1_isr = Default_Handler07
#pragma weak exti2_isr = Default_Handler08
#pragma weak exti3_isr = Default_Handler09
#pragma weak exti4_isr = Default_Handler10
#pragma weak DMA1CH1_IRQHandler = Default_Handler11
#pragma weak DMA1CH2_IRQHandler = Default_Handler12
#pragma weak DMA1CH3_IRQHandler = Default_Handler13
#pragma weak DMA1CH4_IRQHandler = Default_Handler14
#pragma weak DMA1CH5_IRQHandler = Default_Handler15
#pragma weak DMA1CH6_IRQHandler = Default_Handler16
#pragma weak DMA1CH7_IRQHandler = Default_Handler17
#pragma weak adc1_2_isr = Default_Handler18
#pragma weak usb_hp_can1_tx_isr = Default_Handler19
#pragma weak usb_lp_can1_rx0_isr = Default_Handler20
#pragma weak can1_rx1_isr = Default_Handler21
#pragma weak can1_sce_isr = Default_Handler22
#pragma weak exti9_5_isr = Default_Handler23
#pragma weak tim15_isr = Default_Handler24
#pragma weak tim16_isr = Default_Handler25
#pragma weak tim17_isr = Default_Handler26
#pragma weak tim18_isr = Default_Handler27
#pragma weak tim2_isr = Default_Handler28
#pragma weak tim3_isr = Default_Handler29
#pragma weak tim4_isr = Default_Handler30
#pragma weak i2c1_ev_isr = Default_Handler31
#pragma weak i2c1_er_isr = Default_Handler32
#pragma weak i2c2_ev_isr = Default_Handler33
#pragma weak i2c2_er_isr = Default_Handler34
#pragma weak SPI1_IRQHandler = Default_Handler35
#pragma weak SPI2_IRQHandler = Default_Handler36
#pragma weak USART1_IRQHandler = Default_Handler37
#pragma weak USART2_IRQHandler = Default_Handler38
#pragma weak USART3_IRQHandler = Default_Handler39
#pragma weak exti15_10_isr = Default_Handler40
#pragma weak rtc_alarm_isr = Default_Handler41
#pragma weak usb_wakeup_isr = Default_Handler42
#pragma weak tim12_isr = Default_Handler43
#pragma weak tim13_isr = Default_Handler44
#pragma weak tim14_isr = Default_Handler45
#pragma weak tim8_cc_isr = Default_Handler46
#pragma weak adc3_isr = Default_Handler47
#pragma weak fsmc_isr = Default_Handler48
#pragma weak sdio_isr = Default_Handler49
#pragma weak tim5_isr = Default_Handler50
#pragma weak spi3_isr = Default_Handler51
#pragma weak UART4_IRQHandler = Default_Handler52
#pragma weak UART5_IRQHandler = Default_Handler53
#pragma weak tim6_isr = Default_Handler54
#pragma weak tim7_isr = Default_Handler55
#pragma weak DMA2CH1_IRQHandler = Default_Handler56
#pragma weak DMA2CH2_IRQHandler = Default_Handler57
#pragma weak DMA2CH3_IRQHandler = Default_Handler58
#pragma weak DMA2CH4_IRQHandler = Default_Handler59
#pragma weak DMA2CH5_IRQHandler = Default_Handler60
#pragma weak SDADC1_IRQHandler = Default_Handler61
#pragma weak SDADC2_IRQHandler = Default_Handler62
#pragma weak SDADC3_IRQHandler = Default_Handler63
#pragma weak comp1_2_3_isr = Default_Handler64
#pragma weak comp4_5_6_isr = Default_Handler65
#pragma weak comp7_isr = Default_Handler66
#pragma weak reserve1_isr = Default_Handler67
#pragma weak reserve2_isr = Default_Handler68
#pragma weak reserve3_isr = Default_Handler69
#pragma weak reserve4_isr = Default_Handler70
#pragma weak reserve5_isr = Default_Handler71
#pragma weak reserve6_isr = Default_Handler72
#pragma weak reserve7_isr = Default_Handler73
#pragma weak usb_hp_isr = Default_Handler74
#pragma weak usb_lp_isr = Default_Handler75
#pragma weak usb_wakup_isr = Default_Handler76
#pragma weak reserve8_isr = Default_Handler77
#pragma weak tim19_isr = Default_Handler78
#pragma weak reserve9_isr = Default_Handler79
#pragma weak reserve10_isr = Default_Handler80
#pragma weak fpu_isr = Default_Handler81

