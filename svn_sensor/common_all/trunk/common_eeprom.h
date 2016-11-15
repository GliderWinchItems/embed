/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : common_eeprom.h
* Author             : deh
* Date First Issued  : 07/25/2013
* Board              : RxT6
* Description        : eeprom use for CAN
*******************************************************************************/

#ifndef __EEPROM_COMMON_CAN
#define __EEPROM_COMMON_CAN

#include "common_misc.h"
#include "common_can.h"

/* Offset from the beginning of the eeprom.  Words and complement stored (8 bytes) */
// 'eeprom_can.c' reads and writes 32b wordsand their complement.

#define EEPROM_IAMUNITNUMBER	0	// Unit ID 
#define EEPROM_NEXT		4	// Whatever is next


#endif 

