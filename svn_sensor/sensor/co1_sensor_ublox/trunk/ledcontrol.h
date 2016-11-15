/******************************************************************************
* File Name          : ledcontrol.h
* Date First Issued  : 05/11/2014
* Board              : Sensor
* Description        : Flashing the leds
*******************************************************************************/

#ifndef __LEDCONTROL	
#define __LEDCONTROL

/******************************************************************************/
void LeftREDsetmode(int m, int t);
/* @brief 	: Set mode and time for GREEN led
 * @param	: m: 0 = off, 1 = on, 2 = flash with time period t (ms)
 * @param	: t = time ticks in ms
*******************************************************************************/
void LEDsetup(void);
/* @brief	:Add structs for coundown timing
 * ************************************************************************************** */

#endif 

