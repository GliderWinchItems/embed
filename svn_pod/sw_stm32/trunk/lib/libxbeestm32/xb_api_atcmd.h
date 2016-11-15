/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : xb_api_atcmd.h
* Hackee             : deh
* Date First Issued  : 03/25/2012
* Board              : STM32F103VxT6_pod_mm with XBee
* Description        : In API mode send AT command to the module
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __XB_ATCMD
#define __XB_ATCMD

#include "libopenstm32/common.h"

/* ************************************************************/
int xb_api_atcmd(char * p);
/* @brief	: Prepare frame and send AT cmd, e.g. xb_api_atcmd("DL00000FFF");
 * @argument	: p = pointer to string with command
 * @return	: 0 = OK; -1 = string too long
***************************************************************/
int xb_api_queue(char * q);
/* @brief	: Prepare frame and queue AT cmd, e.g. xb_api_atcmd("DL00000FFF");
 * @argument	: p = pointer to string with command
 * @return	: 0 = OK; -1 = string too long
***************************************************************/



#endif 

