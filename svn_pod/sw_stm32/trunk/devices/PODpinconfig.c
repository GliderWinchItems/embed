/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : PODgpiopins.c
* Hackeroo           : deh
* Date First Issued  : 05/20/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Configure gpio port pins used, but not used for (hardware) functions
*******************************************************************************/
/*
06-28-2012 Add: 'configure_pin_in_pd' for using PAD33 with ON/OFF switch
*/
#include "libopenstm32/rcc.h"
#include "libopenstm32/gpio.h"

#include "PODpinconfig.h"

/******************************************************************************
 * void PODgpiopins_Config(void);
 * @brief	: Configure gpio pins
 ******************************************************************************/
void PODgpiopins_Config(void)
{
/* NOTE: The following statement is located in '../lib/libmiscstm32/clocksetup.c' & 'clockspecifysetup.c' */
//	RCC_APB2ENR |= 0x7c;	// Enable A thru E


/* ----------------------------- PORTA -------------------------------------- */
	//  PA0- - POD_box (external) LED: gpio out
//	GPIO_CRL(GPIOA) &= ~((0x000f ) << (4*0));	// Clear CNF reset bit 01 = Floating input (reset state)
//	GPIO_CRL(GPIOA) |=  (( (GPIO_CNF_OUTPUT_PUSHPULL<<2) | (GPIO_MODE_OUTPUT_50_MHZ) ) << (4*0));
	//  PA4 - AD7799_2 /CS: gpio out
	GPIO_CRL(GPIOA) &= ~((0x000f ) << (4*4));	// Clear CNF reset bit 01 = Floating input (reset state)
	GPIO_CRL(GPIOA) |=  (( (GPIO_CNF_OUTPUT_PUSHPULL<<2) | (GPIO_MODE_OUTPUT_50_MHZ) ) << (4*4));
	//  PA7 - SPI1_MOSI: gpio out
	GPIO_CRL(GPIOA) &= ~((0x000f ) << (4*7));	// Clear CNF reset bit 01 = Floating input (reset state)
	GPIO_CRL(GPIOA) |=  (( (GPIO_CNF_OUTPUT_PUSHPULL<<2) | (GPIO_MODE_OUTPUT_50_MHZ) ) << (4*7));
	
	//  PA8 - 3.3v SD Card reg enable: gpio out
	GPIO_CRH(GPIOA) &= ~((0x000f ) << (4*0));	// Clear CNF reset bit 01 = Floating input (reset state)
	GPIO_CRH(GPIOA) |=  (( (GPIO_CNF_OUTPUT_PUSHPULL<<2 | GPIO_MODE_OUTPUT_50_MHZ) << (4*0) ) );	
/* ----------------------------- PORTB -------------------------------------- */
	// PB5	3.3v reg enable--XBee : gpio out
	GPIO_CRL(GPIOB) &= ~((0x000f ) << (4*5));	// Clear CNF reset bit 01 = Floating input (reset state)
	GPIO_CRL(GPIOB) |=  (( (GPIO_CNF_OUTPUT_PUSHPULL<<2 | GPIO_MODE_OUTPUT_50_MHZ) << (4*5)));	
	// PB8	XBee /DTR/SLEEP_RQ: gpio_out 
	// PB9	XBee /RESET: gpio_out 
	// PB10	AD7799_1 /CS:gpio_out
	// PB12	SD_CARD_CD/DAT3/CS:SPI2_NSS
	GPIO_CRH(GPIOB) &=  ~(  ((0x000f) << (4*0)) | ((0x000f) << (4*1)) | ((0x000f) << (4*2)) | ((0x000f ) << (4*4)) );

	GPIO_CRH(GPIOB) |=      (((GPIO_CNF_OUTPUT_PUSHPULL<<2) | GPIO_MODE_OUTPUT_50_MHZ) << (4*0)) |\
				(((GPIO_CNF_OUTPUT_PUSHPULL<<2) | GPIO_MODE_OUTPUT_50_MHZ) << (4*1)) |\
				(((GPIO_CNF_OUTPUT_PUSHPULL<<2) | GPIO_MODE_OUTPUT_50_MHZ) << (4*2)) |\
				(((GPIO_CNF_OUTPUT_PUSHPULL<<2) | GPIO_MODE_OUTPUT_50_MHZ) << (4*4)) ;
/* ----------------------------- PORTC -------------------------------------- */
	// PC4	Top Cell V ADC divider enable: gpio
	// PC5	Bottom Cell V ADC divider enable: gpio
	GPIO_CRL(GPIOC) &=  ~( ( 0x000f<<(4*4) ) | ( 0x000f<<(4*5) ) );

	GPIO_CRL(GPIOC) |=	(((GPIO_CNF_OUTPUT_PUSHPULL<<2) | GPIO_MODE_OUTPUT_2_MHZ) << (4*4)) |\
				(((GPIO_CNF_OUTPUT_PUSHPULL<<2) | GPIO_MODE_OUTPUT_2_MHZ) << (4*5)) ;

/* ----------------------------- PORTD -------------------------------------- */
	// PD1	TXCO Vcc switch:gpio out
	// PD7	MAX3232 Vcc switch: gpio out
	GPIO_CRL(GPIOD) &= ~( ( 0x000f<<(4*1) ) | ( 0x000f<<(4*7) ) );

	GPIO_CRL(GPIOD) |=  	(((GPIO_CNF_OUTPUT_PUSHPULL<<2) | GPIO_MODE_OUTPUT_50_MHZ) << (4*1)) |\
				(((GPIO_CNF_OUTPUT_PUSHPULL<<2) | GPIO_MODE_OUTPUT_50_MHZ) << (4*7)) ;


	// PD10	5V reg enable--Analog--strain guage & AD7799: gpio_out
	// PD14	5v reg enable--Encoders & GPS : gpio
	GPIO_CRH(GPIOD) &= ~( ((0x000f ) << (4*2)) | ((0x000f ) << (4*6)) );

	GPIO_CRH(GPIOD) |= 	( ((GPIO_CNF_OUTPUT_PUSHPULL<<2 | GPIO_MODE_OUTPUT_50_MHZ) << (4*2)) |\
				  ((GPIO_CNF_OUTPUT_PUSHPULL<<2 | GPIO_MODE_OUTPUT_50_MHZ) << (4*6)) );

/* ----------------------------- PORTE -------------------------------------- */
	// PE2	EXTERNAL LED FET-PAD3: gpio out
	// PE3	LED43_1: gpio out
	// PE4	LED43_2: gpio out
	// PE5	LED65_1: gpio out
	// PE6	LED65_2: gpio out
	// PE7	3.2 Regulator EN, Analog: gpio
	GPIO_CRL(GPIOE) &=     ~(((0x000f ) << (4*2)) | ((0x000f ) << (4*3)) | ((0x000f ) << (4*4)) |\
			   	 ((0x000f ) << (4*5)) | ((0x000f ) << (4*6)) | ((0x000f ) << (4*7)) );


	GPIO_CRL(GPIOE) |=	(((GPIO_CNF_OUTPUT_PUSHPULL<<2) | GPIO_MODE_OUTPUT_50_MHZ) << (4*2)) |\
				(((GPIO_CNF_OUTPUT_PUSHPULL<<2) | GPIO_MODE_OUTPUT_50_MHZ) << (4*3)) |\
				(((GPIO_CNF_OUTPUT_PUSHPULL<<2) | GPIO_MODE_OUTPUT_50_MHZ) << (4*4)) |\
				(((GPIO_CNF_OUTPUT_PUSHPULL<<2) | GPIO_MODE_OUTPUT_50_MHZ) << (4*5)) |\
				(((GPIO_CNF_OUTPUT_PUSHPULL<<2) | GPIO_MODE_OUTPUT_50_MHZ) << (4*6)) |\
				(((GPIO_CNF_OUTPUT_PUSHPULL<<2) | GPIO_MODE_OUTPUT_50_MHZ) << (4*7)) ;

	
	// PE15	AD7799 Vcc switch: gpio out
	GPIO_CRH(GPIOE) &=  (~((0x000f ) << (4*7)));	// Clear CNF reset bit 01 = Floating input (reset state)
	GPIO_CRH(GPIOE) |=  (( (GPIO_CNF_OUTPUT_PUSHPULL<<2) | (GPIO_MODE_OUTPUT_50_MHZ) ) << (4*7));	


	return;
}
/******************************************************************************
 * void PODgpiopins_default(void);
 * @brief	: Set pins to low power (default setting)
 ******************************************************************************/
void PODgpiopins_default(void)
{
/* ----------------------------- PORTA -------------------------------------- */
	//  PA4 - AD7799_2 /CS: gpio out
	AD7799_2_CS_hi	// Set bit
	//  PA8 - 3.3v SD Card regulator enable: gpio out
	SDCARDREG_off	// Reset bit
/* ----------------------------- PORTB -------------------------------------- */
	// PB5	3.3v regulator enable--XBee : gpio out
	XBEEREG_off	// Reset bit
	// PB8	XBee /DTR/SLEEP_RQ: gpio_out 
	XBEESLEEPRQ_hi	// Set bit
	// PB9	XBee /RESET: gpio_out 
	XBEE_RESET_low	// Reset bit
	// PB10	AD7799_1 /CS:gpio_out
	AD7799_2_CS_hi	// Set bit
/* ----------------------------- PORTC -------------------------------------- */
	// PC4	Top Cell V ADC divider enable: gpio
	TOPCELLADC_off	// Reset bit
	// PC5	Bottom Cell V ADC divider enable: gpio
	BOTTMCELLADC_off// Reset bit

/* ----------------------------- PORTD -------------------------------------- */
	// PD1	TXCO Vcc switch:gpio out
	TXCOSW_off	// Set bit
	// PD7	MAX3232 Vcc switch: gpio out
	MAX3232SW_off	// Set bit
	// PD10	5V regulator enable--Analog--strain guage & AD7799: gpio_out
	STRAINGAUGEPWR_off	// Set bit
	// PD14	5v regulator enable--Encoders & GPS : gpio
	ENCODERGPSPWR_off	// Reset bit

/* ----------------------------- PORTE -------------------------------------- */
	// PE2	EXTERNAL LED FET-PAD3: gpio out
	EXTERNALLED_off	// Reset bit
	// PE3	LED43_1: gpio out
	// PE7	3.3 Regulator EN, Analog: gpio
	ANALOGREG_off	// Reset bit
	// PE15	AD7799 Vcc switch: gpio out
	ADC7799VCCSW_off	// Set bit
	// LED 43_1,_2 and LED 65_1, _2 *all* off
	LEDSALL_off

	return;
}
/******************************************************************************
 * void PA0_reconfig(char x);
 * @param	: 0 = output; not-zero = floating input
 * @brief	: Reconfigure PA0 
 ******************************************************************************/
void PA0_reconfig(char x)
{
	if (x == 0)
	{ // Here, setup for output
		//  PA0- - POD_box (external) LED: gpio out
		GPIO_CRL(GPIOA) &= ~((0x000f ) << (4*0));	// Clear CNF
		GPIO_CRL(GPIOA) |=  (( (GPIO_CNF_OUTPUT_PUSHPULL<<2) | (GPIO_MODE_OUTPUT_50_MHZ) ) << (4*0));		
	}
	else
	{ // Here, setup for input
		//  PA0- - POD_box (external) pushbutton input
		GPIO_CRL(GPIOA) &= ~((0x000f ) << (4*0));	// Clear CNF
		GPIO_CRL(GPIOA) |=  (( (GPIO_CNF_INPUT_FLOAT<<2) | (GPIO_MODE_INPUT) ) << (4*0));
	}
	return;
}
/* ************************************************************************************************************** 
 * void configure_pin_pod ( volatile u32 * p, int n);
 * @example	: configure_pin ( (volatile u32 *)GPIOD, 12); // Configures PD12 for pushpull output
 * @brief	: configure the pin push/pull output
 * @param	: pointer to port base
 * @param	: bit in port to set or reset
 **************************************************************************************************************** */
void configure_pin_pod ( volatile u32 * p, int n)
{
		
	if (n >= 8)
	{ // Here, the high byte register (CRH), else low byte register (CRL)
		p = p + 0x01;	// point to high register
		n -= 8;		// adjust shift count 
	}

	/* Reset CNF bits */
	*p &= ~((0x000f ) << ( 4 * n ));	// Clear CNF reset bit 01 = Floating input (reset state)

	/* Set for pushpull output */
	*p |=  (( (GPIO_CNF_OUTPUT_PUSHPULL<<2) | (GPIO_MODE_OUTPUT_50_MHZ) ) << ( 4 * n ));	

	return;		
}

/* ************************************************************************************************************** 
 * void configure_pin_in_pd ( volatile u32 * p, int n, char pd);
 * @example	: configure_pin_pd ( (volatile u32 *)GPIOD, 15, 0); // Configures PD15 for input, pull-down
 * @brief	: configure the pin input, pull-up/pull-down
 * @param	: pointer to port base
 * @param	: bit in port to set or reset
 * @pararm	: 0 = pull-down, 1 = pull-up
 **************************************************************************************************************** */
void configure_pin_in_pd ( volatile u32 * p, int n, char pd)
{		
	/* Set output bit register (ODR) p 159 */
	// Mask out ODR bit, then or in the new bit (pd) *.
	p += 0X03;	// Point to ODR register
	*p = ( *p & ~(2 << n) ) | ( *p | ((pd & 0x01) << n) );
	p -= 0x03;	// Back to original

	if (n >= 8)
	{ // Here, the high byte register (CRH), ('else' = low byte register (CRL))
		p = p + 0x01;	// point to high register
		n -= 8;		// adjust shift count 
	}

	/* Reset CNF bits and Mode bits */
	*p &= ~((0x000f ) << ( 4 * n ));	// Clear CNF and mode

	/* Set for input, pull-up/pull-down */
	*p |=  (( (GPIO_CNF_INPUT_PULL_UPDOWN<<2) | (GPIO_MODE_INPUT) ) << ( 4 * n ));	

	return;		
}

/*
README.pins

05/19/2011

STM32F103VxT6_POD PC board layout--

Main Peripheral usage--
SPI1	AD7799_1 & AD7799_2: two sigma-delta strain gauge ADCs
SPI2	SDIO/MMC card
USART1	Serial RS-232 port (RJ-45)
USART2*	XBee module
UART4	Serial RS-232 port (RJ-45)


STMF103VxT6  Port|Pin usage
* = remapped function
@ = test point, or connection pad
( ) = function not used, also indented line
[] = comments

PA
0	Wakeup:	WKUP				(ADC123_IN0) (USART2_CTS)(TIM2_ETR)(TIM5_ETR)(TIM8_ETR)
1	SPARE RJ-45: 	gpio in			(ADC123_IN1) (USART2_RTS)(TIM2_CH1)(TIM5_CH1)
2	BEEPER FET: 	TIM2_CH2		(ADC123_IN2) (USART2_TX) (TIM5_CH2)
3	Thermistor: 	ADC123_IN3		(USART2_RX) (TIM2_CH3)(TIM5_CH3)
4       AD7799_2 /CS:	gpio_out		(ADC12_IN4 ) (USART2_CK)(TIM2_CH4)(TIM5_CH4)(DAC_OUT1)(SPI1_NSS)
5	AD7799(s) SCK :	SPI1_SCK  		(ADC123IN5)(DAC_OUT2)
6	AD7799(s) DOUT/RDY:SPI1_MISO 		(ADC12_IN6)(TIM3_CH1)(TIM8_BKIN*)
7	AD7799(s) DIN:	SPI1_MOSI 		(ADC12_IN7)(TIM3_CH2)(TIM8_CH1N*)
8	3.3v SD Card reg enable: gpio out	(TIM1_CH1) (USART1_CK)
9	USART1_TX:  	 			(TIM1_CH2 )
10	USART1_RX: 				(TIM1_CH3)
11	CAN_RX:					(USB-DM) (USART1_CTS) (TIM1_CH4) 
12	CAN_TX:					(USB-DP) (USART1_RTS) (TIM1_ETR) 
13	JTMS
14	JTCK
15	JTDI

PB
0	Bottom Cell V adc:ADC12_IN8
1	7v+ (Top Cell) adc:ADC12_IN9
2	BOOT1 (grounded)
3	JDTO  					(TIM2_CH2*)(SPI3_SCK)(SPI1_SCK*)
4	JTRST 					(TIM3_CH1*)(SPI3_MISO)(SPI1_MISO*)
5	3.3v reg enable--XBee : gpio		(TIM3 CH2*)(SPI3_MOSI)(SPI1_MOSI*)(I2C1_SMBAI)
6	PAD34					(USART1_TX*) (TIM4_CH1)(I2C1_SCL)
7	PAD35					(USART1_RX*) (TIM4_CH2)(I2C1_SDA)
8	XBee /DTR/SLEEP_RQ: gpio_out 		(CANRX*)(I2C2_SMBAI)
9	XBee /RESET: gpio_out 			(CANTX*)
10	AD7799_1 /CS:gpio_out			(USART3_TX)(TIM2_CH3*)(I2C2_SCL)(I2C1_SDA*)
11	pad:					(USART3_RX)(TIM2_CH4*)(I2C2_SDA)
12	SD_CARD_CD/DAT3/CS:SPI2_NSS  		(USART3_CK)(TIM1_BKIN)
13	SD_CARD_CLK/SCLK:SPI2_SCK  		(USART3_CTS)
14	SD_CARD_Dat0/D0:SPI2_MISO 		(USART3_RTS)
15	SD_CARD_CMD/DI:	SPI2_MOSI

PC
0	ACCEL X:	ADC12_IN10
1	ACCEL Y:	ADC12_IN11
2	ACCEL Z:	ADC12_IN12
3	OP_AMP_OUT:	ADC12_IN13
4	Top Cell V ADC divider enable: gpio	(ADC12_IN14)
5	Bottom Cell V ADC divider enable: gpio	(ADC12_IN15)
6	Encoder_2 chan B: TIM3_CH1*	(TIM8_CH1)
7	Encoder_2 chan A: TIM3_CH2*	(TIM8_CH2)
8					(TIM8_CH3)(TIM3_CH3*)
9	GPS_1PPS: TIM8_CH4		(TIM3_CH4*)
10	UART4_TX: 			(SIDO_D2)(USART3_TX)-
11	UART4_RX: 			(SDIO_D3)(USART3_RX)
12	GPS_RX|RS485_DO: UART5_TX 	(SDIO_CK)(USART3_CK*)
13	RTC (out) connect to PE8 (in)	(ANTI-TAMP)
14	32KHz xtal: OSC32_IN
15	32KHz xtal: OSC32_OUT


PD
0	External TXCO:	OSC_IN* 	(CAN_RX*)
1	TXCO Vcc switch:gpio out	(OSC_OUT*)(CAN_RX*)
2	GPS_TX|RS485_DI:UART5_RX 	(TIM3_ETR)(SDIO_CMD)
3	Xbee /CTS:	USART2_CTS*
4	Xbee /RTS:	USART2_RTS*
5	Xbee DIN/CONFIG:USART2_TX*
6	Xbee DOUT: 	USART2_RX*
7	MAX3232 Vcc switch: gpio out 	(USART2_CK*)
8.	PAD31 : gpio or USART3_TX*
9	PAD32 : gpio or USART3_RX*
10	5V reg enable--Analog--strain guage & AD7799: gpio_out	(USART3_CK*)
11	CAN_RS: gpio out		(USART3_CTS*)
12	Encoder_1 chan B:TIM4_CH1*	(USART3_RTS*)
13	Enocder_1 chan A:TIM4_CH2*
14	5v reg enable--Encoders & GPS : gpio (TIM4_CH3*)
15	PAD33				(TIM4_CH4*)


PE
0	
1	
2	EXTERNAL LED FET-PAD3: gpio out	(TRACE_CK)
3	LED43_1: gpio out
4	LED43_2: gpio out
5	LED65_1: gpio out
6	LED65_2: gpio out
7	3.3 Regulator EN, Analog: gpio
8	PAD25 :   gpio 			(TIM1_CH1N*)
9	PAD26 or RTC(MCO): gpio in  TIM1_CH1*
10	PAD27:   gpio			(TIM1_CH2N*)
11	PAD28:   gpio			(TIM1_CH2* )
12	PAD29:   gpio			(TIM1_CH3N*)
13	PAD30:   gpio			(TIM1_CH3* )
14	PAD31:   gpio			(TIM1_CH4* )
15	AD7799 Vcc switch: gpio out	(TIM1_BKIN*)

*/
