/******************************************************************************
* File Name          : p1_initialization.h
* Date First Issued  : 07/03/2015
* Board              : STM32F103VxT6_pod_mm (USART1)
* Description        : Initialization for winch tension with POD board
*******************************************************************************/
/*
08-30-2011


*/
#ifndef __P1_INITIALIZATION
#define __P1_INITIALIZATION

#define POWERUPDELAY1	5000 	// Powerup delay (0.1 millisecond) SD card, AD7799
#define POWERUPDELAY2	50 	// Powerup delay (0.1 millisecond) ADC

/******************************************************************************/
void p1_initialization(void);
/* @brief 	: Initialize everything needed for all operationg modes
*******************************************************************************/
void p1_initialization_active(void);
/* @brief 	: Complete initialization required for active mode
*******************************************************************************/
void p1_initialization_deepsleep(void);
/* @brief 	: Complete initialization required for deepsleep mode
*******************************************************************************/
extern unsigned char AD7799_num;

#endif

