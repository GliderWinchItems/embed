/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : pwrctl_v2b.h
* Generator          : deh
* Date First Issued  : 07/03/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Power management routines
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PWRCTRL
#define __PWRCTRL

/* Includes ------------------------------------------------------------------*/



/******************************************************************************/
void Powerdown_to_standby_v2b(void);
/* @brief	: Setup and then go to STANDBY low power mode (see p 68 Ref Manual)
 * @param	: Set ALR = CNT + uiInc (uiInc in TR_CLK tick time counts, e.g. 32768Hz/16)
 ******************************************************************************/
#endif 
