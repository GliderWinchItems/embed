/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : 32KHz.h
* Hackeroos          : deh
* Date First Issued  : 06/24/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : 32KHz osc/clock
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __32KHZ
#define __32KHZ

/* Includes ------------------------------------------------------------------*/
#include "libopenstm32/common.h"	// Has things like 'u16' defined
#include "libopenstm32/gpio.h"
#include "libopenstm32/spi.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/rtc.h"

#define	RTC_CLK_ORDER		15		// 32768KHz = (1<<15)
#define PRL_DIVIDE_ORDER	4		// (Divide by 16)
#define PRL_DIVIDER	(1<<PRL_DIVIDE_ORDER)	// RTCCLK divider
#define ALR_INC_ORDER	(RTC_CLK_ORDER-PRL_DIVIDE_ORDER)	// (Divide by 2048)
#define ALR_INCREMENT	(1<<ALR_INC_ORDER)	// Number of pre-scaled clocks for one second
#define	ALR_EXTENSION		BKP_DR1		// Backup register that holds CNT register extension

/* Note: The lower four bits of the priority byte are not used.
 The higher the priority number, the lower the priority */
#define RTC_PRIORITY		0x20	// Interrupt priority for RTC interrupt

/*
When the RTC interrupt reaches the point where the interrupt has been serviced, but there
are more routines that need to be run, the RTC handler sets the interrupt pending bit
for TIM6 in the NVIC.  This interrupt is set to a low priority level.  The RTC handler
returns from interrupt, and the TIM6 interrupt then is serviced (unless there are other
higher level interrupts pending).  The TIM6 interrupt handler then calls the routines
which execute and return, and the returing TIM6 handler resets that interrupt level
active bit.
*/

/* We don't have TIM6 on this processor so we will use the interrupt */
#define TIM6_IRQ_PRIORITY	0xE0	// Interrupt priority for TIM6

/* The following is used for loading the RTC registers */
struct RTCREG
{
	unsigned int prl;	// Prescaler divide count (-1 adjustment made in load routine)
	unsigned int cnt;	// Counter register
	unsigned int alr;	// Alarm register
};



/******************************************************************************/
unsigned int Reset_and_RTC_init(void);
/* @brief	: Setup RTC after coming out of a RESET
 * @return	: RCC_CSR register (at the time this routine begins), see p. 109
 ******************************************************************************/
void RTC_reg_load(struct RTCREG * strRtc);
/* @param	: Pointer to struct with values to be loaded into PRL, CNT, ALR
 * @brief	: Setup RTC registers (PRL is decremented by 1 before loading)
 * @brief	: DR1 used for saving the increment to ALR register for 1 sec ticks
 ******************************************************************************/
void RTC_reg_read(struct RTCREG * strRtc);
/* @param	: Pointer to struct into which values are stored
 * @brief	: Read RTC registers and store in struct
 ******************************************************************************/
void RTC_reg_load_alr(unsigned int uiAlr);
/* @param	: Value to loaded in RTC alarm register
 * @brief	: Set RTC alarm register
 ******************************************************************************/
unsigned int RTC_reg_read_cnt(void);
/* @brief	: Read RTC CNT (counter that counts TR_CLK)
 * @return	: CNT register as a 32 bit word
 ******************************************************************************/
void RTC_reg_load_prl(unsigned int uiPrl);
/* @param	: Value to loaded in RTC prescalar reload register
 * @brief	: Set up the prescalar
 ******************************************************************************/



#endif 
