/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : common_time.c
* Hackerees          : deh
* Date First Issued  : 01/26/2013
* Board              : STM32F103RxT6_pod_mm
* Description        : Olimex CO: time related "stuff"
*******************************************************************************/

//#include "p1_common.h"

#include "common_misc.h"
#include "common_time.h"

volatile struct ALLTIMECOUNTS	strAlltime;	// All the time stuff lumped into this

char cGPStype;	// GPS type

