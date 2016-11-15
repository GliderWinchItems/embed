/******************************************************************************
* File Name          : uart_pinsf3.c
* Date First Issued  : 01/24/2015
* Board              : Discovery F303 F373
* Description        : Edit and check pins for uart
*******************************************************************************/
#include "libopencm3/stm32/memorymap.h"
#include "libopencm3/cm3/common.h"
#include "libopencm3/stm32/usart.h"
#include "libopencm3/stm32/gpio.h"
#include "panic_ledsDf3.h"
#include "libopencm3/stm32/rcc.h"
#include "usartx_setbaud.h"
#include "f3DISCpinconfig.h"
#include "uart_pinsf3.h"

extern unsigned int	pclk1_freq;	// Freq (Hz) of APB1 bus (see ../lib/libmiscstm32/clockspecifysetup.c)
extern unsigned int	pclk2_freq;	// Freq (Hz) of APB2 bus (see ../lib/libmiscstm32/clockspecifysetup.c)

/******************************************************************************
 * int uart_pinsf3(u32 iuart, struct UARTPINS* pu);
 * @brief	: Set up GPIO pins for UART (and enable port clocking)
 * @param	: p = pointer to register base
 * @param	: pu = pointer to struct with return values
 * @return	: Edit check--0 = OK; not OK - panic_leds(8)
******************************************************************************/	
int uart_pinsf3(u32 iuart, struct UARTPINS* pu)
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
/* F373 only has USART1,2,3; F303 adds UART4,5 (no DMA on 5) */
	// Set ports, pins, alternate function code, bus clock freq, and enable peripheral clock
#ifdef STM32F373
#pragma message ("uart_pinsf3.c -- PIN SELECTION FOR STM32F373")
	switch (iuart)
	{
		case USART1:	// F373
			rxport = GPIOA; txport = GPIOA; txpin = 9; rxpin = 10; af = 7; pu->pclk = pclk2_freq;
			RCC_AHBENR |= RCC_AHBENR_IOPAEN;	// Port A
//			rxport = GPIOB; txport = GPIOB; txpin = 6; rxpin =  7; af = 7; pu->pclk = pclk2_freq;
//			RCC_AHBENR |= RCC_AHBENR_IOPBEN;	// Port B
//			rxport = GPIOC; txport = GPIOC; txpin = 4; rxpin =  5; af = 7; pu->pclk = pclk2_freq;
//			RCC_AHBENR |= RCC_AHBENR_IOPCEN;	// Port C
//			rxport = GPIOE; txport = GPIOCE txpin = 0; rxpin =  1; af = 7; pu->pclk = pclk2_freq;
//			RCC_AHBENR |= RCC_AHBENR_IOPEEN;	// Port E
			RCC_APB2ENR |= RCC_APB2ENR_USART1EN; // Enable peripheral clock
			pu->irqnumber = UART1_IRQ_NUMBER;
			break;

		case (u32)USART2:	// F373
			rxport = GPIOA; txport = GPIOA; txpin = 2; rxpin =  3; af = 7; pu->pclk = pclk1_freq;
			RCC_AHBENR |= RCC_AHBENR_IOPAEN;	// Port A
//			rxport = GPIOB; txport = GPIOB; txpin = 3; rxpin =  4; af = 7; pu->pclk = pclk1_freq;//CANrx,tx
//			RCC_AHBENR |= RCC_AHBENR_IOPBEN;	// Port B
//			rxport = GPIOD; txport = GPIOD; txpin = 5; rxpin =  6; af = 7; pu->pclk = pclk1_freq;
//			RCC_AHBENR |= RCC_AHBENR_IOPDEN;	// Port D
			RCC_APB1ENR |= RCC_APB1ENR_USART2EN; // Enable peripheral clock
			pu->irqnumber = UART2_IRQ_NUMBER;
			break;
			
		case (u32)USART3:	// F373
			rxport = GPIOC; txport = GPIOC; txpin = 10; rxpin = 11; af = 7; pu->pclk = pclk1_freq;
			RCC_AHBENR |= RCC_AHBENR_IOPCEN;	// Port C
//			rxport = GPIOE; txport = GPIOB; txpin =  10; rxpin =  15; af = 7; pu->pclk = pclk1_freq;
//			RCC_AHBENR |= RCC_AHBENR_IOPBEN;	// Port B
//			RCC_AHBENR |= RCC_AHBENR_IOPEEN;	// Port E
//			rxport = GPIOD; txport = GPIOD; txpin =  5; rxpin =  6; af = 7; pu->pclk = pclk1_freq;
//			RCC_AHBENR |= RCC_AHBENR_IOPDEN;	// Port D
			RCC_APB1ENR |= RCC_APB1ENR_USART3EN; // Enable peripheral clock
			pu->irqnumber = UART3_IRQ_NUMBER;
			break;

		default:
			return -1;	// Pin assignments: Shouldn't happen
	}
#elif STM32F303
#pragma message ("uart_pinsf3.c -- PIN SELECTION FOR STM32F303")
	switch (iuart)
	{
		case USART1:	// F303
			rxport = GPIOA; txport = GPIOA; txpin = 9; rxpin = 10; af = 7; pu->pclk = pclk2_freq;
			RCC_AHBENR |= RCC_AHBENR_IOPAEN;	// Port A
//			rxport = GPIOB; txport = GPIOB; txpin = 6; rxpin =  7; af = 7; pu->pclk = pclk2_freq;
//			RCC_AHBENR |= RCC_AHBENR_IOPBEN;	// Port B
//			rxport = GPIOC; txport = GPIOC; txpin = 4; rxpin =  5; af = 7; pu->pclk = pclk2_freq;
//			RCC_AHBENR |= RCC_AHBENR_IOPCEN;	// Port C
//			rxport = GPIOE; txport = GPIOCE txpin = 0; rxpin =  1; af = 7; pu->pclk = pclk2_freq;
//			RCC_AHBENR |= RCC_AHBENR_IOPEEN;	// Port E

			RCC_APB2ENR |= RCC_APB2ENR_USART1EN; // Enable peripheral clock
			pu->irqnumber = UART1_IRQ_NUMBER;
			break;

		case (u32)USART2:	// F303
			rxport = GPIOA; txport = GPIOA; txpin = 2; rxpin =  3; af = 7; pu->pclk = pclk1_freq;
			RCC_AHBENR |= RCC_AHBENR_IOPAEN;	// Port A
//			rxport = GPIOA; txport = GPIOA; txpin = 14; rxpin =  15; af = 7; pu->pclk = pclk1_freq;
//			RCC_AHBENR |= RCC_AHBENR_IOPAEN;	// Port A
//			rxport = GPIOB; txport = GPIOB; txpin = 3; rxpin =  4; af = 7; pu->pclk = pclk1_freq;//CANrx,tx
//			RCC_AHBENR |= RCC_AHBENR_IOPBEN;	// Port B
//			rxport = GPIOD; txport = GPIOD; txpin = 5; rxpin =  6; af = 7; pu->pclk = pclk1_freq;
//			RCC_AHBENR |= RCC_AHBENR_IOPDEN;	// Port D
			RCC_APB1ENR |= RCC_APB1ENR_USART2EN; // Enable peripheral clock
			pu->irqnumber = UART2_IRQ_NUMBER;
			break;
			
		case (u32)USART3:	// F303
//			rxport = GPIOC; txport = GPIOC; txpin = 10; rxpin = 11; af = 7; pu->pclk = pclk1_freq;
//			RCC_AHBENR |= RCC_AHBENR_IOPCEN;	// Port C
//			rxport = GPIOE; txport = GPIOB; txpin =  10; rxpin =  11; af = 7; pu->pclk = pclk1_freq;
//			RCC_AHBENR |= RCC_AHBENR_IOPBEN;	// Port B
//			rxport = GPIOE; txport = GPIOB; txpin =  10; rxpin =  15; af = 7; pu->pclk = pclk1_freq;
//			RCC_AHBENR |= RCC_AHBENR_IOPBEN;	// Port B
//			RCC_AHBENR |= RCC_AHBENR_IOPEEN;	// Port E
			rxport = GPIOD; txport = GPIOD; txpin =  8; rxpin =  9; af = 7; pu->pclk = pclk1_freq;
			RCC_AHBENR |= RCC_AHBENR_IOPDEN;	// Port D
			RCC_APB1ENR |= RCC_APB1ENR_USART3EN; // Enable peripheral clock
			pu->irqnumber = UART3_IRQ_NUMBER;
			break;

		case (u32)UART4:	// F303
			rxport = GPIOC; txport = GPIOC; txpin = 10; rxpin = 11; af = 5; pu->pclk = pclk1_freq;
			RCC_AHBENR |= RCC_AHBENR_IOPCEN;	// Port C
			RCC_APB1ENR |= RCC_APB1ENR_USART4EN; // Enable peripheral clock
			pu->irqnumber = UART4_IRQ_NUMBER;
			break;

		case (u32)UART5:	// F303
			txport = GPIOC; rxport = GPIOD; txpin = 12; rxpin =  2; af = 5; pu->pclk = pclk1_freq;
			RCC_AHBENR |= RCC_AHBENR_IOPCEN | RCC_AHBENR_IOPDEN;	// Port C & D
			RCC_APB1ENR |= RCC_APB1ENR_USART5EN; 		// Enable peripheral clock
			pu->irqnumber = UART5_IRQ_NUMBER;
			break;

		default:
			return -1;	// Pin assignments: Shouldn't happen
	}

#else
  #  error "either STM32F303 or STM32F373 must be specified for USART/UART pin selection, e.g., export STM32TYPE=STM32F373"
#endif


	/* Configure tx pin */
	pin_uarttx.afrl = af;
	f3gpiopins_Config ((volatile u32*)txport, txpin, (struct PINCONFIG*)&pin_uarttx);

	/* Configure rx pin */
	pin_uartrx.afrl = af;
	f3gpiopins_Config ((volatile u32*)rxport, rxpin, (struct PINCONFIG*)&pin_uartrx);

	return 0;
}
