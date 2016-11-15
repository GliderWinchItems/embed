/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : p1_gps_time_linux.h
* Hackeroos          : deh
* Date First Issued  : 10/08/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Convert gps|rtc time ticks to linux time format
*******************************************************************************/
/*
01/28/2013 deh - Modified from original for POD
*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __P1_GPSTIMELINUX
#define __P1_GPSTIMELINUX

#include "common_time.h"

/******************************************************************************/
time_t gps_time_linux_init(struct TIMESTAMPGP1 * pts);
/* @brief	: Convert ascii time from GPS to linux time_t format
 * @param	: p - pointer to GPS line
 * @return	: 0 = record is not valid; not zero = time
 ******************************************************************************/

#endif 

