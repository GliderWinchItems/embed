/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : xb_api_tx.h
* Hackee             : deh
* Date First Issued  : 03/24/2012
* Board              : STM32F103VxT6_pod_mm with XBee
* Description        : Transmit with API mode
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __XB_API_TX
#define __XB_API_TX

#include "libopenstm32/common.h"

/* ************************************************************/
void xb_send_frame(char * p, int count);
/* @brief	: Prepare frame and send
 * @argument	: p = pointer to bytes to be sent
 * @argument	: count = number of bytes in frame less header
***************************************************************/
void xb_tx16(u16 addr, char * q, u16 count);
/* @brief	: Send RF packet, 16 bit addressing
 * @argument	: addr = address (0xffff = broadcast)
 * @argument	: q = pointer to bytes to be sent
 * @argument	: count = number of data bytes to send (addr, et al., not included)
***************************************************************/
void xb_tx64(unsigned long long addr, char * q, int count);
/* @brief	: Send RF packet, 16 bit addressing
 * @argument	: addr = address (0xffff = broadcast)
 * @argument	: q = pointer to bytes to be sent
 * @argument	: count = number of data bytes to send (addr, et al., not included)
 ***************************************************************/


#endif 

