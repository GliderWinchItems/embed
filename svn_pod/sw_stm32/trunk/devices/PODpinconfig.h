/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : PODgpiopins.h
* Hackeroo           : deh
* Date First Issued  : 05/20/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Configure gpio port pins used, but not used for (hardware) functions
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PODPIN
#define __PODPIN

/* Includes ------------------------------------------------------------------*/
#include "libopenstm32/common.h"	// Has things like 'u16' defined
#include "libopenstm32/gpio.h"

/* Subroutines */
/******************************************************************************/
void PODgpiopins_Config(void);
/* @brief	: Configure gpio pins
 ******************************************************************************/
void PODgpiopins_default(void);
/* @brief	: Set pins to low power (default setting)
 ******************************************************************************/
void PA0_reconfig(char x);
/* @param	: 0 = output; not-zero = floating input
 * @brief	: Reconfigure PA0 
 ******************************************************************************/
void configure_pin_pod ( volatile u32 * p, int n);
/* @example	: configure_pin ( (volatile u32 *)GPIOD, 12); // Configures PD12 for pushpull output
 * @brief	: configure the pin push/pull output
 * @param	: pointer to port base
 * @param	: bit in port to set or reset
 **************************************************************************** */
void configure_pin_in_pd ( volatile u32 * p, int n, char pd);
/* @example	: configure_pin_pd ( (volatile u32 *)GPIOD, 15, 0); // Configures PD15 for input, pull-down
 * @brief	: configure the pin input, pull-up/pull-down
 * WARNING: Do not forget (volatile u32 *)!
 * @param	: pointer to port base
 * @param	: bit in port to set or reset
 * @pararm	: 0 = pull-down, 1 = pull-up
 **************************************************************************************************************** */

/* Macros for inline code */

/* Note: 
Switches and on-board LEDs are OFF when the pin is high, 
and power regulators are OFF when the pin is low.  

'-CS' or '-RESET' means NOT CS, NOT RESET, etc. 
*/

/* ----------------------------- PORTA -------------------------------------- */
	//  PA0- - POD_box (external) LED: gpio out
#define PA0_WKUP_hi	GPIO_BSRR(GPIOA)=(1<<0);	// Set bit
#define PA0_WKUP_low	GPIO_BRR (GPIOA)=(1<<0);	// Reset bit
	//  PA4 - AD7799_2 /CS: gpio out
#define AD7799_2_CS_hi	GPIO_BSRR(GPIOA)=(1<<4);	// Set bit
#define AD7799_2_CS_low	GPIO_BRR (GPIOA)=(1<<4);	// Reset bit
	//  PA8 - 3.3v SD Card regulator enable: gpio out
#define SDCARDREG_on	GPIO_BSRR(GPIOA)=(1<<8);	// Set bit
#define SDCARDREG_off	GPIO_BRR (GPIOA)=(1<<8);	// Reset bit
/* ----------------------------- PORTB -------------------------------------- */
	// PB5	3.3v regulator enable--XBee : gpio out
#define XBEEREG_on	GPIO_BSRR(GPIOB)=(1<<5);	// Set bit
#define XBEEREG_off	GPIO_BRR (GPIOB)=(1<<5);	// Reset bit
	// PB8	XBee /DTR/SLEEP_RQ: gpio_out 
#define XBEESLEEPRQ_hi	GPIO_BSRR(GPIOB)=(1<<8);	// Set bit
#define XBEESLEEPRQ_low	GPIO_BRR (GPIOB)=(1<<8);	// Reset bit
	// PB9	XBee /RESET: gpio_out 
#define XBEE_RESET_hi	GPIO_BSRR(GPIOB)=(1<<9);	// Set bit
#define XBEE_RESET_low	GPIO_BRR (GPIOB)=(1<<9);	// Reset bit
	// PB10	AD7799_1 /CS:gpio_out
#define AD7799_1_CS_hi	GPIO_BSRR(GPIOB)=(1<<10);	// Set bit
#define AD7799_1_CS_low	GPIO_BRR (GPIOB)=(1<<10);	// Reset bit
	// PB12	SD_CARD_CD/DAT3/CS:SPI2_NSS
#define SDCARD_CS_hi	GPIO_BSRR(GPIOB)=(1<<12);	// Set bit
#define SDCARD_CS_low	GPIO_BRR (GPIOB)=(1<<12);	// Reset bit

/* ----------------------------- PORTC -------------------------------------- */
	// PC4	Top Cell V ADC divider enable: gpio
#define TOPCELLADC_on		GPIO_BSRR(GPIOC)=(1<<4);	// Set bit
#define	TOPCELLADC_off		GPIO_BRR (GPIOC)=(1<<4);	// Reset bit
	// PC5	Bottom Cell V ADC divider enable: gpio
#define BOTTOMCELLADC_on	GPIO_BSRR(GPIOC)=(1<<5);	// Set bit
#define	BOTTMCELLADC_off	GPIO_BRR (GPIOC)=(1<<5);	// Reset bit

/* ----------------------------- PORTD -------------------------------------- */
	// PD1	TXCO Vcc switch:gpio out
#define TXCOSW_off		GPIO_BSRR(GPIOD)=(1<<1);	// Set bit
#define TXCOSW_on		GPIO_BRR (GPIOD)=(1<<1);	// Reset bit
	// PD7	MAX3232 Vcc switch: gpio out
#define MAX3232SW_off		GPIO_BSRR(GPIOD)=(1<<7);	// Set bit
#define MAX3232SW_on		GPIO_BRR (GPIOD)=(1<<7);	// Reset bit
	// PD10	5V regulator enable--Analog--strain gauge & AD7799: gpio_out
#define STRAINGAUGEPWR_on	GPIO_BSRR(GPIOD)=(1<<10);	// Set bit
#define STRAINGAUGEPWR_off	GPIO_BRR (GPIOD)=(1<<10);	// Reset bit
	// PD14	5v regulator enable--Encoders & GPS : gpio
#define ENCODERGPSPWR_on	GPIO_BSRR(GPIOD)=(1<<14);	// Set bit
#define ENCODERGPSPWR_off	GPIO_BRR (GPIOD)=(1<<14);	// Reset bit

/* ----------------------------- PORTE -------------------------------------- */
	// PE2	EXTERNAL LED FET-PAD3: gpio out
#define EXTERNALLED_on	GPIO_BSRR(GPIOE)=(1<<2);	// Set bit
#define EXTERNALLED_off	GPIO_BRR (GPIOE)=(1<<2);	// Reset bit
	// PE3	LED43_1: gpio out
#define LED43_1_off	GPIO_BSRR(GPIOE)=(1<<3);	// Set bit
#define LED43_1_on	GPIO_BRR (GPIOE)=(1<<3);	// Reset bit
#define LED3_off	GPIO_BSRR(GPIOE)=(1<<3);	// Set bit
#define LED3_on		GPIO_BRR (GPIOE)=(1<<3);	// Reset bit
	// PE4	LED43_2: gpio out
#define LED43_2_off	GPIO_BSRR(GPIOE)=(1<<4);	// Set bit
#define LED43_2_on	GPIO_BRR (GPIOE)=(1<<4);	// Reset bit
#define LED4_off	GPIO_BSRR(GPIOE)=(1<<4);	// Set bit
#define LED4_on		GPIO_BRR (GPIOE)=(1<<4);	// Reset bit
	// PE5	LED65_1: gpio out
#define LED65_1_off	GPIO_BSRR(GPIOE)=(1<<5);	// Set bit
#define LED65_1_on	GPIO_BRR (GPIOE)=(1<<5);	// Reset bit
#define LED5_off	GPIO_BSRR(GPIOE)=(1<<5);	// Set bit
#define LED5_on		GPIO_BRR (GPIOE)=(1<<5);	// Reset bit
	// PE6	LED65_2: gpio out
#define LED65_2_off	GPIO_BSRR(GPIOE)=(1<<6);	// Set bit
#define LED65_2_on	GPIO_BRR (GPIOE)=(1<<6);	// Reset bit
#define LED6_off	GPIO_BSRR(GPIOE)=(1<<6);	// Set bit
#define LED6_on		GPIO_BRR (GPIOE)=(1<<6);	// Reset bit
	// PE7	3.2v Regulator EN, Analog: gpio
#define ANALOGREG_on	GPIO_BSRR(GPIOE)=(1<<7);	// Set bit
#define ANALOGREG_off	GPIO_BRR (GPIOE)=(1<<7);	// Reset bit
	// PE15	AD7799 Vcc switch: gpio out
#define ADC7799VCCSW_off	GPIO_BSRR(GPIOE)=(1<<15);	// Set bit
#define ADC7799VCCSW_on		GPIO_BRR (GPIOE)=(1<<15);	// Reset bit
	// LED 43_1,_2 and LED 65_1, _2 *all* off or on
#define LEDSALL_off	GPIO_BSRR(GPIOE)=(0x000f<<3);	// Set bit
#define LEDSALL_on	GPIO_BRR (GPIOE)=(0x000f<<3);	// Reset bit

#define LED3	3	// LED43_1 bit
#define LED4	4	// LED43_2 bit
#define LED5	5	// LED65_1 bit
#define LED6	6	// LED65_2 bit

#define LEDGREEN1	3	// LED43_1 bit
#define LEDRED		4	// LED43_2 bit
#define LEDGREEN2	5	// LED65_1 bit
#define LEDYELLOW	6	// LED65_2 bit


	// Individual special purpose pads
#define GPSONOFF_BIT	10		// PAD27 = Port E Bit 10
#define GPSONOFF_IDR	GPIOE_IDR	// ON/OFF switch sensing
#define GPSONOFF_CONFIG (volatile u32 *)GPIOE


#endif 


