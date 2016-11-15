/******************************************************************************
* File Name          : tim_clk.h
* Date First Issued  : 08/06/2015
* Board              : f103
* Description        : Count seconds, minutes, hours, for yogurt.c
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TIM_CLK
#define __TIM_CLK

#include <stdint.h>

struct TIMCLK
{
	uint8_t	sc;	// Seconds
	uint8_t mn;	// Minutes
	uint8_t hr;	// Hours
};

/******************************************************************************/
void tim_clk_init(void);
/* @brief	: Initialize TIM3 that produces interrupts used for timing measurements
*******************************************************************************/
void tim_clk_zero(void);
/* @brief	: Set time count to zero
*******************************************************************************/
struct TIMCLK* tim_clk_hms_remaining(uint32_t ctr);
/* @brief	: Convert count of secs to hours minutes seconds
 * @return	: struct with hr mn sc
*******************************************************************************/
void tim_clk_counter(void);
/* @brief	: Counter tick
*******************************************************************************/



#endif 
