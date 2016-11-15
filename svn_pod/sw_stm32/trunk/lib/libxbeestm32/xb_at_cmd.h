/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : xb_at_cmd.h
* Hackee             : deh
* Date First Issued  : 03/24/2012
* Board              : STM32F103VxT6_pod_mm with XBee
* Description        : Send AT command
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __XB_AT_CMD
#define __XB_AT_CMD

#include "libopenstm32/common.h"

/* ************************************************************/
void xb_send_AT_cmd(char * p);
/* @brief	: Send command(s) to XB
 * @param	: p = pointer to string to send
***************************************************************/


#endif 

