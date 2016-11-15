/******************************************************************************
* File Name          : DISCpinconfig.c
* Date First Issued  : 02/29/2012
* Board              : STM32 Discovery
* Description        : Configure gpio port pins used, but not used for (hardware) functions
*******************************************************************************/
#include "../libopencm3/stm32/f4/rcc.h"
#include "../libopencm3/stm32/f4/gpio.h"
#include "DISCpinconfig.h"

/******************************************************************************
 * void DISCgpiopins_Config(void);
 * @brief	: Configure gpio pins
 ******************************************************************************/
void DISCgpiopins_Config(void)
{
/* ----------------------------- PORTA -------------------------------------- */
/* ----------------------------- PORTB -------------------------------------- */
/* ----------------------------- PORTC -------------------------------------- */
/* ----------------------------- PORTD -------------------------------------- */
	RCC_AHB1ENR |= (1<<3);	// Enable port D clocking (p 110, p 148)

	/* PD12,13,14,15 are the green, orange, red, blue LEDs on the Discovery board */
	GPIOD_MODER    =   (GPIOD_MODER   & (~(0x00ff << 24))) | (0x0055 << 24);	// Mode = General purpose output
	GPIOD_OTYPER   =   (GPIOD_OTYPER  & (~(0x000f << 12))) | (0x0000 << 12);	// Type output = push-pull
	GPIOD_OSPEEDR  =   (GPIOD_OSPEEDR & (~(0x00ff << 24))) | (0x00aa << 24);	// Pin speed = Speed 50 MHz
	GPIOD_PUPDR    =   (GPIOD_PUPDR   & (~(0x00ff << 24))) | (0x0000 << 24);	// Pull up/down = none.

	return;
}
/******************************************************************************
 * void DISCgpiopins_default(void);
 * @brief	: Set pins to low power (default setting)
 ******************************************************************************/
void DISCgpiopins_default(void)
{
/* ----------------------------- PORTA -------------------------------------- */
/* ----------------------------- PORTB -------------------------------------- */
/* ----------------------------- PORTC -------------------------------------- */
/* ----------------------------- PORTD -------------------------------------- */
/* ----------------------------- PORTE -------------------------------------- */
	return;
}
/******************************************************************************
 * void f4gpiopins_Config(volatile u32 * p, u16 pinnumber, struct PINCONFIG * s  );
 * @param	: See comments on each in source code
 * @brief	: Configure one gpio pin 
 ******************************************************************************/
void f4gpiopins_Config(volatile u32 * p, u16 pinnumber, struct PINCONFIG * s)
{

	/* Make sure bus clocking is enabled so that gpio registers can be accessed (p 110) */
	u32 x = ((u32)p - (u32)GPIOA) >> 10;	// Shift count for enabling RCC_AHB1ENR
	RCC_AHB1ENR |= ((1 << x) & 0x01ff);	// Enable port clocking

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
