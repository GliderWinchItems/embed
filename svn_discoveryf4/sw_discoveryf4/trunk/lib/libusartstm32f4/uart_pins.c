/******************************************************************************
* File Name          : uart_pins.c
* Date First Issued  : 11/06/2013
* Board              : Discovery F4 (F405 or F407)
* Description        : Edit and check pins for uart
*******************************************************************************/
#include "libopencm3/stm32/memorymap.h"
#include "libopencm3/cm3/common.h"
#include "libopencm3/stm32/f4/usart.h"
#include "libopencm3/stm32/usart.h"
#include "libopencm3/stm32/f4/gpio.h"
#include "panic_leds.h"
#include "libopencm3/stm32/f4/rcc.h"
#include "usartx_setbaud.h"
#include "DISCpinconfig.h"
#include "uart_pins.h"

extern unsigned int	pclk1_freq;	// Freq (Hz) of APB1 bus (see ../lib/libmiscstm32/clockspecifysetup.c)
extern unsigned int	pclk2_freq;	// Freq (Hz) of APB2 bus (see ../lib/libmiscstm32/clockspecifysetup.c)

/******************************************************************************
 * int uart_pins(u32 iuart, struct UARTPINS* pu);
 * @brief	: Set up GPIO pins for UART (and enable port clocking)
 * @param	: p = pointer to register base
 * @param	: pu = pointer to struct with return values
 * @return	: Edit check--0 = OK; not OK - panic_leds(8)
******************************************************************************/	
int uart_pins(u32 iuart, struct UARTPINS* pu)
{

	struct PINCONFIG	pin_uarttx = { \
		GPIO_MODE_AF,		// mode: alternate function
		GPIO_OTYPE_PP, 		// output type: push-pull 		
		GPIO_OSPEED_100MHZ, 	// speed: highest drive level
		GPIO_PUPD_NONE, 	// pull up/down: none
		0 };			// Alternate function code: to be filled in

	struct PINCONFIG	pin_uartrx = { \
		GPIO_MODE_AF,		// mode: alternate function
		0, 			// output type: 		
		0,		 	// speed: highest drive level
		GPIO_PUPD_PULLUP, 	// pull up/down: pullup
		0 };			// Alternate function code: to be filled in

	u32	rxpin = 0;
	u32	txpin = 0;
	u32	af = 0;	// Alternate function
	u32	txport;
	u32	rxport;

	/* THIS IS BOARD SPECIFIC */
	// Set ports, pins, alternate function code, bus clock freq, and enable peripheral clock
	switch (iuart)
	{
		case USART1:
			rxport = GPIOA; txport = GPIOA; txpin = 9; rxpin = 10; af = 7; pu->pclk = pclk2_freq;
			RCC_AHB1ENR |= 0x01;	// Port A
//			rxport = GPIOB; txport = GPIOB; txpin = 6; rxpin =  7; af = 7; pu->pclk = pclk2_freq;
//			RCC_AHB1ENR |= 0x02;	// Port B
			RCC_APB2ENR |= (1<<4); // Enable peripheral clock
			pu->irqnumber = UART1_IRQ_NUMBER;
			break;

		case (u32)USART2:
			rxport = GPIOA; txport = GPIOA; txpin = 2; rxpin =  3; af = 7; pu->pclk = pclk1_freq;
			RCC_AHB1ENR |= 0x01;	// Port A
//			rxport = GPIOD; txport = GPIOD; txpin = 5; rxpin =  6; af = 7; pu->pclk = pclk1_freq;
//			RCC_AHB1ENR |= 0x08;	// Port D
			RCC_APB1ENR |= (1<<17); // Enable peripheral clock
			pu->irqnumber = UART2_IRQ_NUMBER;
			break;
			
		case (u32)USART3:
			rxport = GPIOB; txport = GPIOB; txpin = 10; rxpin = 11; af = 7; pu->pclk = pclk1_freq;
			RCC_AHB1ENR |= 0x02;	// Port B
//			rxport = GPIOD; txport = GPIOD; txpin =  8; rxpin =  9; af = 7; pu->pclk = pclk1_freq;
//			RCC_AHB1ENR |= 0x08;	// Port D
			RCC_APB1ENR |= (1<<18); // Enable peripheral clock
			pu->irqnumber = UART3_IRQ_NUMBER;
			break;

		case (u32)UART4:
			rxport = GPIOA; txport = GPIOA; txpin =  0; rxpin =  1; af = 8; pu->pclk = pclk1_freq;
			RCC_AHB1ENR |= 0x01;	// Port A
//			rxport = GPIOC; txport = GPIOC; txpin = 10; rxpin = 11; af = 8; pu->pclk = pclk1_freq;
//			RCC_AHB1ENR |= 0x04;	// Port C
			RCC_APB1ENR |= (1<<19); // Enable peripheral clock
			pu->irqnumber = UART4_IRQ_NUMBER;
			break;

		case (u32)UART5:
			txport = GPIOC; rxport = GPIOD; txpin = 12; rxpin =  2; af = 8; pu->pclk = pclk1_freq;
			RCC_AHB1ENR |= 0x06;	// Port C & D
			RCC_APB1ENR |= (1<<20); // Enable peripheral clock
			pu->irqnumber = UART5_IRQ_NUMBER;
			break;

		case (u32)USART6:
			txport = GPIOC; rxport = GPIOC; txpin =  6; rxpin =  7; af = 8; pu->pclk = pclk2_freq;
			RCC_AHB1ENR |= 0x04;	// Port C
			RCC_APB2ENR |= (1<<5); 	// Enable peripheral clock
			pu->irqnumber = UART6_IRQ_NUMBER;			
			break;

		default:
			return -1;	// Pin assignments: Shouldn't happen
	}

	/* Configure tx pin */
	pin_uarttx.afrl = af;
	f4gpiopins_Config ((volatile u32*)txport, txpin, (struct PINCONFIG*)&pin_uarttx);

	/* Configure rx pin */
	pin_uartrx.afrl = af;
	f4gpiopins_Config ((volatile u32*)rxport, rxpin, (struct PINCONFIG*)&pin_uartrx);

	return 0;
}
