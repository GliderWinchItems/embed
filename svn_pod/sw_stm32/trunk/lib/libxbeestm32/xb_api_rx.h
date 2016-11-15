/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : xb_api_rx.h
* Hackee             : deh
* Date First Issued  : 03/24/2012
* Board              : STM32F103VxT6_pod_mm with XBee
* Description        : Receive with API mode
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __XB_API_RX
#define __XB_API_RX

#include "libopenstm32/common.h"

/* ************************************************************/
int xb_getframe(char *p, u16 size);
/* @brief	: Send command(s) to XB
 * @param	: p = pointer to buffer to receive frame
 * @param	: size = number of bytes in buffer 
 * @return	: byte count of frame received--
 *		:   0 = frame not ready
 * 		:   n = actual byte count of frame
 *		:   - = error in frame
***************************************************************/
int xb_api_at(char * pAT);
/* @brief	: Send AT command to module (when in api mode)
 * @argument	: pointer to string with AT command, e.g. "DL 1")
 ***************************************************************/
#endif 

