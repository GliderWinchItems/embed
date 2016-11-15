/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : PPMadjust.h
* Hackeroo          : deh
* Date First Issued  : 07/21/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Pediodic adjusting 32 KHz osc time-base for temp & offset
*******************************************************************************/
/* NOTE:

*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PPMADJUST
#define __PPMADJUST

#define	PPMADJUSTTIMEINCREMENT	32768*60	// 60 seconds between updates

/******************************************************************************/
void PPMadjust(void);
/*  @brief	: Do a time adjustment check
*******************************************************************************/


#endif
