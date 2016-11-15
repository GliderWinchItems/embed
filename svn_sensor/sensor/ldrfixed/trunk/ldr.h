/*******************************************************************************
* File Name          : ldrfixed.h
* Date First Issued  : 07/24/2013
* Board              : sensor board RxT6 w STM32F103RGT6
* Description        : Fixed CAN program loader for sensor units
*******************************************************************************/

#ifndef __LDRFIXED
#define __LDRFIXED

#include "common_can.h"

/* Each node on the CAN bus gets a unit number */
#define IAMUNITNUMBER	CAN_UNITID_LDR	// Fixed loader (when no valid eeprom unitid is present)




#endif 

