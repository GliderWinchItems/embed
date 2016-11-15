/******************************************************************************
* File Name          : irq_priorities_co1.h
* Date First Issued  : 08/14/2016
* Board              : Sensor
* Description        : Logging function
*******************************************************************************/
/* 
IRQ priorities that
*/

#ifndef __IRQ_PRIORITIES_CO1
#define __IRQ_PRIORITIES_CO1



/* Interrupt priorities - lower numbers are higher priority.  */
#define TIM4_PRIORITY_CO			0x20	//   ### Higher than CAN_RX1 (FIFO 1)
#define SYSTICK_PRIORITY_SE 			0X50	//   ### Lower than CAN_RX1 (FIFO 1)
#define TIM4_PRIORITY_SE			0x60	//   
#define TIM9_PRIORITY				0x80	//   1/2 ms timing: triggers CAN_poll_loop
#define NVIC_I2C2_EV_IRQ_PRIORITY_CO		0xA0	// tim4 gps 1 pps handling
#define NVIC_I2C1_EV_IRQ_PRIORITY_		0xC0	// Obsolete: used in canwinch_pod.c
#define NVIC_I2C1_ER_IRQ_PRIORITY		0xD0	// CAN poll loop 
#define NVIC_I2C2_ER_IRQ_PRIORITY_CO		0xE0	// can log writing SD card

#endif

