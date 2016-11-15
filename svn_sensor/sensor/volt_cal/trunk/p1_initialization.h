/******************************************************************************
* File Name          : p1_initialization_vcal.h
* Date First Issued  : 07/03/2015
* Board              : STM32F103VxT6_pod_mm (USART1)
* Description        : Initialization for winch tension with POD board
*******************************************************************************/
/*
08-30-2011


*/
#ifndef __P1_INITIALIZATION_VCAL
#define __P1_INITIALIZATION_VCAL

#define POWERUPDELAY1	5000 	// Powerup delay (0.1 millisecond) SD card, AD7799
#define POWERUPDELAY2	50 	// Powerup delay (0.1 millisecond) ADC

/******************************************************************************/
int p1_initialization_vcal(void);
/* @brief 	: Initialize everything needed for all operationg modes
 * @return	: 0 = OK; not zero = error
*******************************************************************************/


#endif

