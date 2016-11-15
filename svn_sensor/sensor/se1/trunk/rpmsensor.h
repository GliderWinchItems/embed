/******************************************************************************
* File Name          : rpmsensor.h
* Date First Issued  : 07/04/2013
* Board              : RxT6
* Description        : RPM sensing
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RPMSENSOR
#define __RPMSENSOR

/* Includes ------------------------------------------------------------------*/
#include "libopenstm32/common.h"	// Has things like 'u16' defined
#include "libopenstm32/gpio.h"
#include "libopenstm32/spi.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/rtc.h"

union TIMCAPTURE64
{
	unsigned short	us[4];
	unsigned int	ui[2];
	unsigned long long ll;
};

struct TIMCAPTRET32
{
	unsigned int 	ic;	// 32b capture tick count
	unsigned int	flg;	// running count of ic's
//	unsigned int	cnt;	// not used in this implementation
};

/******************************************************************************/
void rpmsensor_init(void);
/* @brief 	: Initialize TIM4_CH4 and routines to measure rpm
*******************************************************************************/
struct TIMCAPTRET32 Tim4_inputcapture_ui(void);
/* @brief	: Lock interrupts
 * @return	: Current timer count and flag counter in a struct
*******************************************************************************/

extern void 	(*systickHIpriority2X_ptr)(void);	// SYSTICK handler (very high priority)
extern void 	(*systickLOpriority2_ptr)(void);	// SYSTICK handler (low high priority) continuation--1/2048th
extern void 	(*systickLOpriority2X_ptr)(void);	// SYSTICK handler (low high priority) continuation--1/64th

extern u32 rpm;


#endif 

