/******************************************************************************
* File Name          : triac_ctl.c
* Date First Issued  : 08/07/2015
* Board              : f103
* Description        : Control for triac
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TRIAC_CTL
#define __TRIAC_CTL

#include <stdint.h>

/******************************************************************************/
void triac_ctl_init(void);
/* @brief	: Initialize 
*******************************************************************************/
void triac_ctl_poll(void);
/* @brief	: Update triac control loop
*******************************************************************************/

#endif 
