/* ****************************************************************************************
10/05/2012 Modified F4 from data in stm32vector.txt
*******************************************************************************************/

/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2010 Piotr Esden-Tempski <piotr@esden.net>
 * Copyright (C) 2011 Fergus Noble <fergusnoble@gmail.com>
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
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#define WEAK __attribute__ ((weak))

/* Symbols exported by the linker script(s): */
extern unsigned _data_loadaddr, _data, _edata, _ebss, _stack;

void main(void);
void reset_handler(void);
void blocking_handler(void);
void null_handler(void);

void WEAK reset_handler(void);
void WEAK nmi_handler(void);
void WEAK hard_fault_handler(void);
void WEAK mem_manage_handler(void);
void WEAK bus_fault_handler(void);
void WEAK usage_fault_handler(void);
void WEAK sv_call_handler(void);
void WEAK debug_monitor_handler(void);
void WEAK pend_sv_handler(void);
void WEAK sys_tick_handler(void);

void WEAK   nonmaskableint_isr(void);  /* -14,      !< 2 non maskable interrupt                                          */
void WEAK   memosymanagement_irr(void);/* -12,      !< 4 cortex-m4 memory management interrupt                           */
void WEAK   busfault_isr(void);        /* -11,      !< 5 cortex-m4 bus fault interrupt                                   */
void WEAK   usagefault_isr(void);      /* -10,      !< 6 cortex-m4 usage fault interrupt                                 */
void WEAK   svcall_isr(void);          /* -5,       !< 11 cortex-m4 sv call interrupt                                    */
void WEAK   debugmonitos_irr(void);    /* -4,       !< 12 cortex-m4 debug monitor interrupt                              */
void WEAK   pendsv_isr(void);          /* -2,       !< 14 cortex-m4 pend sv interrupt                                    */
void WEAK   systick_isr(void);         /* -1,       !< 15 cortex-m4 system tick interrupt                                */
void WEAK   wwdg_isr(void);            /* 0,        !< window watchdog interrupt                                         */
void WEAK   pvd_isr(void);             /* 1,        !< pvd through exti line detection interrupt                         */
void WEAK   tampes_stamp_irr(void);    /* 2,        !< tamper and timestamp interrupts through the exti line 19          */
void WEAK   stc_wkup_irr(void);        /* 3,        !< rtc wakeup interrupt through the exti line 20                     */
void WEAK   flash_isr(void);           /* 4,        !< flash global interrupt                                            */
void WEAK   scc_irr(void);             /* 5,        !< rcc global interrupt                                              */
void WEAK   exti0_isr(void);           /* 6,        !< exti line0 interrupt                                              */
void WEAK   exti1_isr(void);           /* 7,        !< exti line1 interrupt                                              */
void WEAK   exti2_ts_isr(void);        /* 8,        !< exti line2 interrupt and touch sense interrupt                    */
void WEAK   exti3_isr(void);           /* 9,        !< exti line3 interrupt                                              */
void WEAK   exti4_isr(void);           /* 10,       !< exti line4 interrupt                                              */
void WEAK   dma1_channel1_isr(void);   /* 11,       !< dma1 channel 1 interrupt                                          */
void WEAK   dma1_channel2_isr(void);   /* 12,       !< dma1 channel 2 interrupt                                          */
void WEAK   dma1_channel3_isr(void);   /* 13,       !< dma1 channel 3 interrupt                                          */
void WEAK   dma1_channel4_isr(void);   /* 14,       !< dma1 channel 4 interrupt                                          */
void WEAK   dma1_channel5_isr(void);   /* 15,       !< dma1 channel 5 interrupt                                          */
void WEAK   dma1_channel6_isr(void);   /* 16,       !< dma1 channel 6 interrupt                                          */
void WEAK   dma1_channel7_isr(void);   /* 17,       !< dma1 channel 7 interrupt                                          */
void WEAK   adc1_isr(void);            /* 18,       !< adc1 interrupts                                                   */
void WEAK   can1_tx_isr(void);         /* 19,       !< can1 tx interrupt                                                  */
void WEAK   can1_sx0_irr(void);        /* 20,       !< can1 rx0 interrupt                                                 */
void WEAK   can1_sx1_irr(void);        /* 21,       !< can1 rx1 interrupt                                                 */
void WEAK   can1_sce_isr(void);        /* 22,       !< can1 sce interrupt                                                 */
void WEAK   exti9_5_isr(void);         /* 23,       !< external line[9:5] interrupts                                     */
void WEAK   tim15_isr(void);           /* 24,       !< tim15 global interrupt                                            */
void WEAK   tim16_isr(void);           /* 25,       !< tim16 global interrupt                                            */
void WEAK   tim17_isr(void);           /* 26,       !< tim17 global interrupt                                            */
void WEAK   tim18_dac2_isr(void);      /* 27,       !< tim18 global interrupt and dac2 underrun interrupt                */
void WEAK   tim2_isr(void);            /* 28,       !< tim2 global interrupt                                             */
void WEAK   tim3_isr(void);            /* 29,       !< tim3 global interrupt                                             */
void WEAK   tim4_isr(void);            /* 30,       !< tim4 global interrupt                                             */
void WEAK   i2c1_ev_isr(void);         /* 31,       !< i2c1 event interrupt                                              */
void WEAK   i2c1_es_irr(void);         /* 32,       !< i2c1 error interrupt                                              */
void WEAK   i2c2_ev_isr(void);         /* 33,       !< i2c2 event interrupt                                              */
void WEAK   i2c2_es_irr(void);         /* 34,       !< i2c2 error interrupt                                              */  
void WEAK   spi1_isr(void);            /* 35,       !< spi1 global interrupt                                             */
void WEAK   spi2_isr(void);            /* 36,       !< spi2 global interrupt                                             */
void WEAK   usast1_irr(void);          /* 37,       !< usart1 global interrupt                                           */
void WEAK   usast2_irr(void);          /* 38,       !< usart2 global interrupt                                           */
void WEAK   usast3_irr(void);          /* 39,       !< usart3 global interrupt                                           */
void WEAK   exti15_10_isr(void);       /* 40,       !< external line[15:10] interrupts                                   */
void WEAK   stc_alarm_irr(void);       /* 41,       !< rtc alarm (a and b) through exti line interrupt                   */
void WEAK   cec_isr(void);             /* 42,       !< cec interrupt                                                     */    
void WEAK   tim12_isr(void);           /* 43,       !< tim12 global interrupt                                            */
void WEAK   tim13_isr(void);           /* 44,       !< tim13 global interrupt                                            */
void WEAK   tim14_isr(void);           /* 45,       !< tim14 global interrupt                                            */
void WEAK   tim5_isr(void);            /* 50,       !< tim5 global interrupt                                             */
void WEAK   spi3_isr(void);            /* 51,       !< spi3 global interrupt                                             */
void WEAK   tim6_dac1_isr(void);       /* 54,       !< tim6 global and dac1 cahnnel1 & cahnnel2 underrun error interrupts*/
void WEAK   tim7_isr(void);            /* 55,       !< tim7 global interrupt                                             */
void WEAK   dma2_channel1_isr(void);   /* 56,       !< dma2 channel 1 global interrupt                                   */
void WEAK   dma2_channel2_isr(void);   /* 57,       !< dma2 channel 2 global interrupt                                   */
void WEAK   dma2_channel3_isr(void);   /* 58,       !< dma2 channel 3 global interrupt                                   */
void WEAK   dma2_channel4_isr(void);   /* 59,       !< dma2 channel 4 global interrupt                                   */
void WEAK   dma2_channel5_isr(void);   /* 60,       !< dma2 channel 5 global interrupt                                   */
void WEAK   sdadc1_isr(void);          /* 61,       !< adc sigma delta 1 global interrupt                                */
void WEAK   sdadc2_isr(void);          /* 62,       !< adc sigma delta 2 global interrupt                                */
void WEAK   sdadc3_isr(void);          /* 63,       !< adc sigma delta 1 global interrupt                                */
void WEAK   comp_isr(void);            /* 64,       !< comp1 and comp2 global interrupt                                  */
void WEAK   usb_hp_isr(void);          /* 74,       !< usb high priority global interrupt                                */
void WEAK   usb_lp_isr(void);          /* 75,       !< usb low priority global interrupt                                 */
void WEAK   usbwakeup_isr(void);       /* 76,       !< usb wakeup interrupt                                              */
void WEAK   tim19_isr(void);           /* 78,       !< tim19 global interrupt                                            */
void WEAK   fpu_isr(void);             /* 81        !< floating point interrupt                                          */

__attribute__ ((section(".vectors")))
void (*const vector_table[]) (void) = {
	(void *)&_stack,
	reset_handler,

/* The following from the libopencm3 f4 */
//	nmi_handler,
//	hard_fault_handler,
//	mem_manage_handler,
//	bus_fault_handler,
//	usage_fault_handler,
//	0, 0, 0, 0,		/* Reserved */
//	sv_call_handler,
//	debug_monitor_handler,
//	0,			/* Reserved */
//	pend_sv_handler,
//	sys_tick_handler,



  nonmaskableint_isr,        /* -14,      !< 2 non maskable interrupt                                          */
  0,
  memosymanagement_irr,      /* -12,      !< 4 cortex-m4 memory management interrupt                           */
  busfault_isr,              /* -11,      !< 5 cortex-m4 bus fault interrupt                                   */
  usagefault_isr,            /* -10,      !< 6 cortex-m4 usage fault interrupt                                 */
  0,0,0,0,
  svcall_isr,                /* -5,       !< 11 cortex-m4 sv call interrupt                                    */
  debugmonitos_irr,          /* -4,       !< 12 cortex-m4 debug monitor interrupt                              */
  0,
  pendsv_isr,                /* -2,       !< 14 cortex-m4 pend sv interrupt                                    */
  systick_isr,               /* -1,       !< 15 cortex-m4 system tick interrupt                                */

  wwdg_isr,                  /* 0,        !< window watchdog interrupt                                         */
  pvd_isr,                   /* 1,        !< pvd through exti line detection interrupt                         */
  tampes_stamp_irr,          /* 2,        !< tamper and timestamp interrupts through the exti line 19          */
  stc_wkup_irr,              /* 3,        !< rtc wakeup interrupt through the exti line 20                     */
  flash_isr,                 /* 4,        !< flash global interrupt                                            */
  scc_irr,                   /* 5,        !< rcc global interrupt                                              */
  exti0_isr,                 /* 6,        !< exti line0 interrupt                                              */
  exti1_isr,                 /* 7,        !< exti line1 interrupt                                              */
  exti2_ts_isr,              /* 8,        !< exti line2 interrupt and touch sense interrupt                    */
  exti3_isr,                 /* 9,        !< exti line3 interrupt                                              */
  exti4_isr,                 /* 10,       !< exti line4 interrupt                                              */
  dma1_channel1_isr,         /* 11,       !< dma1 channel 1 interrupt                                          */
  dma1_channel2_isr,         /* 12,       !< dma1 channel 2 interrupt                                          */
  dma1_channel3_isr,         /* 13,       !< dma1 channel 3 interrupt                                          */
  dma1_channel4_isr,         /* 14,       !< dma1 channel 4 interrupt                                          */
  dma1_channel5_isr,         /* 15,       !< dma1 channel 5 interrupt                                          */
  dma1_channel6_isr,         /* 16,       !< dma1 channel 6 interrupt                                          */
  dma1_channel7_isr,         /* 17,       !< dma1 channel 7 interrupt                                          */
  adc1_isr,                  /* 18,       !< adc1 interrupts                                                   */
  can1_tx_isr,               /* 19,       !< can1 tx interrupt                                                  */
  can1_sx0_irr,              /* 20,       !< can1 rx0 interrupt                                                 */
  can1_sx1_irr,              /* 21,       !< can1 rx1 interrupt                                                 */
  can1_sce_isr,              /* 22,       !< can1 sce interrupt                                                 */
  exti9_5_isr,               /* 23,       !< external line[9:5] interrupts                                     */
  tim15_isr,                 /* 24,       !< tim15 global interrupt                                            */
  tim16_isr,                 /* 25,       !< tim16 global interrupt                                            */
  tim17_isr,                 /* 26,       !< tim17 global interrupt                                            */
  tim18_dac2_isr,            /* 27,       !< tim18 global interrupt and dac2 underrun interrupt                */
  tim2_isr,                  /* 28,       !< tim2 global interrupt                                             */
  tim3_isr,                  /* 29,       !< tim3 global interrupt                                             */
  tim4_isr,                  /* 30,       !< tim4 global interrupt                                             */
  i2c1_ev_isr,               /* 31,       !< i2c1 event interrupt                                              */
  i2c1_es_irr,               /* 32,       !< i2c1 error interrupt                                              */
  i2c2_ev_isr,               /* 33,       !< i2c2 event interrupt                                              */
  i2c2_es_irr,               /* 34,       !< i2c2 error interrupt                                              */  
  spi1_isr,                  /* 35,       !< spi1 global interrupt                                             */
  spi2_isr,                  /* 36,       !< spi2 global interrupt                                             */
  usast1_irr,                /* 37,       !< usart1 global interrupt                                           */
  usast2_irr,                /* 38,       !< usart2 global interrupt                                           */
  usast3_irr,                /* 39,       !< usart3 global interrupt                                           */
  exti15_10_isr,             /* 40,       !< external line[15:10] interrupts                                   */
  stc_alarm_irr,             /* 41,       !< rtc alarm (a and b) through exti line interrupt                   */
  cec_isr,                   /* 42,       !< cec interrupt                                                     */    
  tim12_isr,                 /* 43,       !< tim12 global interrupt                                            */
  tim13_isr,                 /* 44,       !< tim13 global interrupt                                            */
  tim14_isr,                 /* 45,       !< tim14 global interrupt                                            */
  tim5_isr,                  /* 50,       !< tim5 global interrupt                                             */
  spi3_isr,                  /* 51,       !< spi3 global interrupt                                             */
  tim6_dac1_isr,             /* 54,       !< tim6 global and dac1 cahnnel1 & cahnnel2 underrun error interrupts*/
  tim7_isr,                  /* 55,       !< tim7 global interrupt                                             */
  dma2_channel1_isr,         /* 56,       !< dma2 channel 1 global interrupt                                   */
  dma2_channel2_isr,         /* 57,       !< dma2 channel 2 global interrupt                                   */
  dma2_channel3_isr,         /* 58,       !< dma2 channel 3 global interrupt                                   */
  dma2_channel4_isr,         /* 59,       !< dma2 channel 4 global interrupt                                   */
  dma2_channel5_isr,         /* 60,       !< dma2 channel 5 global interrupt                                   */
  sdadc1_isr,                /* 61,       !< adc sigma delta 1 global interrupt                                */
  sdadc2_isr,                /* 62,       !< adc sigma delta 2 global interrupt                                */
  sdadc3_isr,                /* 63,       !< adc sigma delta 1 global interrupt                                */
  comp_isr,                  /* 64,       !< comp1 and comp2 global interrupt                                  */
  usb_hp_isr,                /* 74,       !< usb high priority global interrupt                                */
  usb_lp_isr,                /* 75,       !< usb low priority global interrupt                                 */
  usbwakeup_isr,             /* 76,       !< usb wakeup interrupt                                              */
  tim19_isr,                 /* 78,       !< tim19 global interrupt                                            */
  fpu_isr,                   /* 81        !< floating point interrupt                                          */
};

void reset_handler(void)
{
	volatile unsigned *src, *dest;

	__asm__("MSR msp, %0" : : "r"(&_stack));

	for (src = &_data_loadaddr, dest = &_data; dest < &_edata; src++, dest++)
		*dest = *src;

	while (dest < &_ebss)
		*dest++ = 0;

	/* Call the application's entry point. */
	main();
}

void blocking_handler(void)
{
	while (1) ;
}

void null_handler(void)
{
	/* Do nothing. */
}

#pragma weak  nonmaskableint_isr = weak null_handler
#pragma weak  memosymanagement_irr = weak null_handler
#pragma weak  busfault_isr = weak null_handler
#pragma weak  usagefault_isr = weak null_handler
#pragma weak  svcall_isr = weak null_handler
#pragma weak  debugmonitos_irr = weak null_handler
#pragma weak  pendsv_isr = weak null_handler
#pragma weak  systick_isr = weak null_handler
#pragma weak  wwdg_isr = weak null_handler
#pragma weak  pvd_isr = weak null_handler
#pragma weak  tampes_stamp_irr = weak null_handler
#pragma weak  stc_wkup_irr = weak null_handler
#pragma weak  flash_isr = weak null_handler
#pragma weak  scc_irr = weak null_handler
#pragma weak  exti0_isr = weak null_handler
#pragma weak  exti1_isr = weak null_handler
#pragma weak  exti2_ts_isr = weak null_handler
#pragma weak  exti3_isr = weak null_handler
#pragma weak  exti4_isr = weak null_handler
#pragma weak  dma1_channel1_isr = weak null_handler
#pragma weak  dma1_channel2_isr = weak null_handler
#pragma weak  dma1_channel3_isr = weak null_handler
#pragma weak  dma1_channel4_isr = weak null_handler
#pragma weak  dma1_channel5_isr = weak null_handler
#pragma weak  dma1_channel6_isr = weak null_handler
#pragma weak  dma1_channel7_isr = weak null_handler
#pragma weak  adc1_isr = weak null_handler
#pragma weak  can1_tx_isr = weak null_handler
#pragma weak  can1_sx0_irr = weak null_handler
#pragma weak  can1_sx1_irr = weak null_handler
#pragma weak  can1_sce_isr = weak null_handler
#pragma weak  exti9_5_isr = weak null_handler
#pragma weak  tim15_isr = weak null_handler
#pragma weak  tim16_isr = weak null_handler
#pragma weak  tim17_isr = weak null_handler
#pragma weak  tim18_dac2_isr = weak null_handler
#pragma weak  tim2_isr = weak null_handler
#pragma weak  tim3_isr = weak null_handler
#pragma weak  tim4_isr = weak null_handler
#pragma weak  i2c1_ev_isr = weak null_handler
#pragma weak  i2c1_es_irr = weak null_handler
#pragma weak  i2c2_ev_isr = weak null_handler
#pragma weak  i2c2_es_irr = weak null_handler
#pragma weak  spi1_isr = weak null_handler
#pragma weak  spi2_isr = weak null_handler
#pragma weak  usast1_irr = weak null_handler
#pragma weak  usast2_irr = weak null_handler
#pragma weak  usast3_irr = weak null_handler
#pragma weak  exti15_10_isr = weak null_handler
#pragma weak  stc_alarm_irr = weak null_handler
#pragma weak  cec_isr = weak null_handler
#pragma weak  tim12_isr = weak null_handler
#pragma weak  tim13_isr = weak null_handler
#pragma weak  tim14_isr = weak null_handler
#pragma weak  tim5_isr = weak null_handler
#pragma weak  spi3_isr = weak null_handler
#pragma weak  tim6_dac1_isr = weak null_handler
#pragma weak  tim7_isr = weak null_handler
#pragma weak  dma2_channel1_isr = weak null_handler
#pragma weak  dma2_channel2_isr = weak null_handler
#pragma weak  dma2_channel3_isr = weak null_handler
#pragma weak  dma2_channel4_isr = weak null_handler
#pragma weak  dma2_channel5_isr = weak null_handler
#pragma weak  sdadc1_isr = weak null_handler
#pragma weak  sdadc2_isr = weak null_handler
#pragma weak  sdadc3_isr = weak null_handler
#pragma weak  comp_isr = weak null_handler
#pragma weak  usb_hp_isr = weak null_handler
#pragma weak  usb_lp_isr = weak null_handler
#pragma weak  usbwakeup_isr = weak null_handler
#pragma weak  tim19_isr = weak null_handler
#pragma weak  fpu_isr = weak null_handler
