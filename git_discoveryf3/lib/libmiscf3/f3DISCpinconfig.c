/******************************************************************************
* File Name          : f3DISCpinconfig.c
* Date First Issued  : 01/20/2016
* Board              : STM32F373
* Description        : Configure gpio port pins used, but not used for (hardware) functions
*******************************************************************************/
#include <stdint.h>

#include "../libopencm3/stm32/rcc.h"
#include "../libopencm3/stm32/gpio.h"
#include "f3DISCpinconfig.h"

/******************************************************************************
 * void f3DISCgpiopins_Config(void);
 * @brief	: Configure gpio pins
 ******************************************************************************/
void f3DISCgpiopins_Config(void)
{

	return;
}
/******************************************************************************
 * void f3DISCgpiopins_default(void);
 * @brief	: Set pins to low power (default setting)
 ******************************************************************************/
void f3DISCgpiopins_default(void)
{
/* ----------------------------- PORTA -------------------------------------- */
/* ----------------------------- PORTB -------------------------------------- */
/* ----------------------------- PORTC -------------------------------------- */
/* ----------------------------- PORTD -------------------------------------- */
/* ----------------------------- PORTE -------------------------------------- */
	return;
}
/******************************************************************************
 * void f3gpiopins_Config(volatile u32 *p, u16 pinnumber, struct PINCONFIG * s  );
 * @param	: See comments on each in source code
 * @brief	: Configure one gpio pin 
 ******************************************************************************/
void f3gpiopins_Config(volatile u32 *p, u16 pinnumber, struct PINCONFIG * s)
{

	/* Make sure bus clocking is enabled so that gpio registers can be accessed (p 110) */
	u32 x = (((u32)p - (u32)GPIOA) >> 10) + 17;	// Shift count for enabling RCC_AHB1ENR
	RCC_AHBENR |= (1 << x);	// Enable port clocking

/* MODE
These bits are written by software to configure the I/O direction mode.
00: Input (reset state)
01: General purpose output mode
10: Alternate function mode
11: Analog mode
*/
	GPIO_MODER(p)  &= ~(0x03 << (pinnumber << 1));		// Reset old mode values
	GPIO_MODER(p)  |= ((s->mode & 0x03) << (pinnumber << 1));	// Add new values

/* TYPE GPIO_OTYPER
These bits are written by software to configure the output type of the I/O port.
0: Output push-pull (reset state)
1: Output open-drain
*/
	GPIO_MODER(p+1) &= ~(0x01 << pinnumber);		// Reset old type bit
	GPIO_MODER(p+1) |=  ((s->type & 0x01) << pinnumber);// Add new type bit

/* SPEED GPIO_OSPEEDR
These bits are written by software to configure the I/O output speed.
00: 2 MHz Low speed
01: 25 MHz Medium speed
10: 50 MHz Fast speed
11: 100 MHz High speed on 30 pF (80 MHz Output max speed on 15 pF)
*/

	GPIO_MODER(p+2) &= ~(0x03 << (pinnumber << 1));		// Reset old speed values
	GPIO_MODER(p+2) |= ((s->speed & 0x03) << (pinnumber << 1));// Add new values

/* PULL UP/DOWN GPIO_PUPDR
These bits are written by software to configure the I/O pull-up or pull-down
00: No pull-up, pull-down
01: Pull-up
10: Pull-down
11: Reserved
*/
	GPIO_MODER(p+3) &= ~(0x03 << (pinnumber << 1));		// Reset old pupdn values
	GPIO_MODER(p+3) |= ((s->pupdn & 0x03) << (pinnumber << 1));	// Add new values

/*
These bits are written by software to configure alternate function I/Os
AFRLy & AFRHy selection:
                                  1000: AF8
0000: AF0
                                  1001: AF9
0001: AF1
                                  1010: AF10
0010: AF2
                                  1011: AF11
0011: AF3
                                  1100: AF12
0100: AF4
                                  1101: AF13
0101: AF5
                                  1110: AF14
0110: AF6
                                  1111: AF15
0111: AF7

GPIO_AFRL
*/
	if (pinnumber >= 8)
	{ // Here, the high register (pins 8 - 15) (p 152)
		pinnumber -= 8;		// adjust shift count 
		p++;			// Point to high register
	}

	GPIO_MODER(p+8) &= ~(0x0f << (pinnumber << 2));			// Remove old AF settings
	GPIO_MODER(p+8) |=  ((s->afrl & 0x0f) << (pinnumber << 2));	// Add new settings

	return;
}
