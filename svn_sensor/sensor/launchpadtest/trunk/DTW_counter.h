/******************************************************************************
* File Name          : DTW_counter.h
* Date First Issued  : 10/21/2013
* Board              : STM32
* Description        : Use of the DTW_CYCCNT counter
*******************************************************************************/
#ifndef __DTW_COUNTER
#define __DTW_COUNTER

#include "../libopenstm32/common.h"	// Has things like 'u16' defined

#define DTWTIME	(*(volatile unsigned int *)0xE0001004)	// Read DTW 32b system tick counter
/******************************************************************************/
void DTW_counter_init(void);
/* @brief 	: Setup the DTW counter so that it can be read
*******************************************************************************/

#endif 

