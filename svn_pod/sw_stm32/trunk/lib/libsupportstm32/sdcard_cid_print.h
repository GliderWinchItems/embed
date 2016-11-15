/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : sdcard_cid_print.h
* Hackeroo           : deh
* Date First Issued  : 08/29/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Extraction of sd card cid fields for version 1 sd cards
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SDCARD_CID_PRINT
#define __SDCARD_CID_PRINT

/*****************************************************************************************/
void sdcard_cid_print(char* p);
/* @brief	: printf the CID fields 
* @param	: pointer to CID byte array (16 bytes)
*****************************************************************************************/

/* Shameless hack to make it work on POD without changing it for Olimex */
extern char sdcard_cid_print_POD	; // 0 = USART2; 1 = USART1

#endif 

