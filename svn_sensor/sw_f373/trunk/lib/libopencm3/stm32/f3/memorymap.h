/* ****************************************************************************************
10/05/2012 DEH Modified F4 -> F3
*******************************************************************************************/

/*
 * This file is part of the libopencm3 project.
 *
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

#ifndef LIBOPENCM3_MEMORYMAP_H
#define LIBOPENCM3_MEMORYMAP_H

#include <libopencm3/cm3/memorymap.h>

/* --- STM32F3 specific peripheral definitions ----------------------------- */

/* Memory map for all busses */
#define PERIPH_BASE			0x40000000			 /* F3 */
#define PERIPH_BASE_APB1		(PERIPH_BASE + 0x00000)		 /* F3 */
#define PERIPH_BASE_APB2		(PERIPH_BASE + 0x10000)		 /* F3 */
#define PERIPH_BASE_AHB1		(PERIPH_BASE + 0x20000)		 /* F3 */
#define PERIPH_BASE_AHB2		0x48000000			 /* F3 */

/* Register boundary addresses */

/* APB1 */
#define TIM2_BASE			(PERIPH_BASE_APB1 + 0x0000)	/* F3 */
#define TIM3_BASE			(PERIPH_BASE_APB1 + 0x0400)	/* F3 */
#define TIM4_BASE			(PERIPH_BASE_APB1 + 0x0800)	/* F3 */
#define TIM5_BASE			(PERIPH_BASE_APB1 + 0x0C00)	/* F3 */
#define TIM6_BASE			(PERIPH_BASE_APB1 + 0x1000)	/* F3 */
#define TIM7_BASE			(PERIPH_BASE_APB1 + 0x1400)	/* F3 */
#define TIM12_BASE			(PERIPH_BASE_APB1 + 0x1800)	/* F3 */
#define TIM13_BASE			(PERIPH_BASE_APB1 + 0x1C00)	/* F3 */
#define TIM14_BASE			(PERIPH_BASE_APB1 + 0x2000)	/* F3 */
/* Reserved (PERIPH_BASE_APB1 + 0x2400)-(PERIPH_BASE_APB1 + 0x27FF) */	/* F3 */
#define RTC_BASE			(PERIPH_BASE_APB1 + 0x2800)	/* F3 */
#define WWDG_BASE			(PERIPH_BASE_APB1 + 0x2C00)	/* F3 */
#define IWDG_BASE			(PERIPH_BASE_APB1 + 0x3000)	/* F3 */
/* Reserved (PERIPH_BASE_APB1 + 0x3400)-(PERIPH_BASE_APB1 + 0x37FF) */	/* F3 */
#define SPI2_I2S_BASE			(PERIPH_BASE_APB1 + 0x3800)	/* F3 */
#define SPI3_I2S_BASE			(PERIPH_BASE_APB1 + 0x3C00)	/* F3 */
/* Reserved (PERIPH_BASE_APB1 + 0x4000)-(PERIPH_BASE_APB1 + 0x43FF) */	/* F3 */
#define USART2_BASE			(PERIPH_BASE_APB1 + 0x4400)	/* F3 */
#define USART3_BASE			(PERIPH_BASE_APB1 + 0x4800)	/* F3 */
/* Reserved (PERIPH_BASE_APB1 + 0x4C00)-(PERIPH_BASE_APB1 + 0x53FF) */	/* F3 */
#define I2C1_BASE			(PERIPH_BASE_APB1 + 0x5400)	/* F3 */
#define I2C2_BASE			(PERIPH_BASE_APB1 + 0x5800)	/* F3 */
#define USB_OTG_FS_BASE			(PERIPH_BASE_APB1 + 0x5C00)	/* F3 */
#define USB_PACKET_SRAM_BASE		(PERIPH_BASE_APB1 + 0x6000)	/* F3 */
#define BX_CAN1_BASE			(PERIPH_BASE_APB1 + 0x6400)	/* F3 */
/* Reserved (PERIPH_BASE_APB1 + 0x6800)-(PERIPH_BASE_APB1 + 0x6FFF) */	/* F3 */
#define POWER_CONTROL_BASE		(PERIPH_BASE_APB1 + 0x7000)	/* F3 */
#define DAC1_BASE			(PERIPH_BASE_APB1 + 0x7400)	/* F3 */
#define CEC_BASE			(PERIPH_BASE_APB1 + 0x7800)	/* F3 */
/* Reserved (PERIPH_BASE_APB1 + 0x7C00)-(PERIPH_BASE_APB1 + 0x97FF) */	/* F3 */
#define DAC2_BASE			(PERIPH_BASE_APB1 + 0x9800)	/* F3 */
#define TIM14_BASE			(PERIPH_BASE_APB1 + 0x9C00)	/* F3 */


/* APB2 */
#define SYSCFG_BASE			(PERIPH_BASE_APB2 + 0x0000)	/* F3 */
#define EXTI_BASE			(PERIPH_BASE_APB2 + 0x0400)	/* F3 */
/* Reserved (PERIPH_BASE_APB2 + 0x0800)-(PERIPH_BASE_APB2 + 0x1FFF) */	/* F3 */
#define ADC1_BASE			(PERIPH_BASE_APB2 + 0x2400)	/* F3 */
/* Reserved				(PERIPH_BASE_APB2 + 0x2800) */	/* F3 */
#define SPI1_I2S1_BASE			(PERIPH_BASE_APB2 + 0x3000)	/* F3 */
/* Reserved				(PERIPH_BASE_APB2 + 0x3400) */	/* F3 */
#define USART1_BASE			(PERIPH_BASE_APB2 + 0x3800)	/* F3 */
/* Reserved				(PERIPH_BASE_APB2 + 0x3C00) */	/* F3 */
#define TIM15_BASE			(PERIPH_BASE_APB2 + 0x4000)	/* F3 */
#define TIM16_BASE			(PERIPH_BASE_APB2 + 0x4400)	/* F3 */
#define TIM17_BASE			(PERIPH_BASE_APB2 + 0x4800)	/* F3 */
/* Reserved (PERIPH_BASE_APB2 + 0x4C00)-(PERIPH_BASE_APB2 + 0x5BFF) */	/* F3 */
#define TIM19_BASE			(PERIPH_BASE_APB2 + 0x5C00)	/* F3 */
#define SDADC1_BASE			(PERIPH_BASE_APB2 + 0x6000)	/* F3 */
#define SDADC2_BASE			(PERIPH_BASE_APB2 + 0x6400)	/* F3 */
#define SDADC3_BASE			(PERIPH_BASE_APB2 + 0x6800)	/* F3 */


/* AHB1 */
#define DMA1_BASE			(PERIPH_BASE_AHB1 + 0x0000)	/* F3 */
#define DMA2_BASE			(PERIPH_BASE_AHB1 + 0x0400)	/* F3 */
/* Reserved 				(PERIPH_BASE_AHB1 + 0x0800) */	/* F3 */
#define RCC_BASE			(PERIPH_BASE_AHB1 + 0x1000)	/* F3 */
/* Reserved 				(PERIPH_BASE_AHB1 + 0x1400) */	/* F3 */
#define FLASH_MEM_INTERFACE_BASE	(PERIPH_BASE_AHB1 + 0x2000)	/* F3 */
/* Reserved 				(PERIPH_BASE_AHB1 + 0x2400) */	/* F3 */
#define CRC_BASE			(PERIPH_BASE_AHB1 + 0x3000)	/* F3 */
/* Reserved 				(PERIPH_BASE_AHB1 + 0x3400) */	/* F3 */
#define TSC_BASE			(PERIPH_BASE_AHB1 + 0x4000)	/* F3 */



/* AHB2 */
#define GPIO_PORT_A_BASE		(PERIPH_BASE_AHB2 + 0x0000)	/* F3 */
#define GPIO_PORT_B_BASE		(PERIPH_BASE_AHB2 + 0x0400)	/* F3 */
#define GPIO_PORT_C_BASE		(PERIPH_BASE_AHB2 + 0x0800)	/* F3 */
#define GPIO_PORT_D_BASE		(PERIPH_BASE_AHB2 + 0x0C00)	/* F3 */
#define GPIO_PORT_E_BASE		(PERIPH_BASE_AHB2 + 0x1000)	/* F3 */
#define GPIO_PORT_F_BASE		(PERIPH_BASE_AHB2 + 0x1400)	/* F3 */



#endif
