/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : se3.c
* Author             : deh
* Date First Issued  : 07/19/2013
* Board              : sensor board RxT6 w STM32F103RGT6
* Description        : sensor w shaft encoder
*******************************************************************************/

#ifndef __SE3
#define __SE3

#include "common_can.h"

/* Each node on the CAN bus gets a unit number */
#define IAMUNITNUMBER	CAN_UNITID_SE3	// Sensor board #3 with photosensor, loggable ID
//#define IAMUNITNUMBER	CAN_UNITID_GPSDT	// DEBUG




#endif 

