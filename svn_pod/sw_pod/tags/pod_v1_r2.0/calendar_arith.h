/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : calendar_arith.h
* Author             : deh
* Date First Issued  : 09/17/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Calendar & RTC tick count
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __P1_CALENDAR_ARITH
#define __P1_CALENDAR_ARITH

/******************************************************************************/
struct tm *calendar_pkt_to_tm(struct TIMESTAMPGP1 * strTS);
/* @brief	: Convert GPS extracted time to Linux struct tm
 * @param	: Pointer to GPS timestamp
 * @return	: Pointer to static with Linux tm struct
*******************************************************************************/

#endif 
