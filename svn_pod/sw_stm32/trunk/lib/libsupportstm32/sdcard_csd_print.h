/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : sdcard_csd_print.h
* Hackeroo           : deh
* Date First Issued  : 08/28/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Extraction of sd card csd fields for version 1 sd cards
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SDCARD_CSD_PRINT
#define __SDCARD_CSD_PRINT

/*****************************************************************************************/
void sdcard_csd_print(char* p);
/* @brief	: printf the CSD fields 
* @param	: pointer to CSD byte array (16 bytes)
*****************************************************************************************/

/* Shameless hack to make it work on POD without changing it for Olimex */
extern char sdcard_csd_print_POD	; // 0 = USART2; 1 = USART1

#endif 

