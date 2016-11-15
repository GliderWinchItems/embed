/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : eeprom_can.h
* Author             : deh
* Date First Issued  : 07/25/2013
* Board              : RxT6
* Description        : eeprom use for CAN
*******************************************************************************/

#ifndef __EEPROM_CAN
#define __EEPROM_CAN

#include "common_misc.h"
#include "common_can.h"
#include "common_eeprom.h"

/* **************************************************************************************/
struct TWO32 eeprom_get(u32 offset);
/* @brief	: retrieve a 32b word from the eeprom
 * @param	; offset = byte offset from beginning of eeprom
 * @return	: a = word; b = complement of word
 * ************************************************************************************** */
int eeprom_put(u32 offset,u32 *p);
/* @brief	: write a 32b word and its complement
 * @param	: offset = byte count offset from beginning of eeprom
 * @param	: p = pointer to word to be written
 * @return	: 0 = OK; not 0 = bad
 * ************************************************************************************** */


#endif 

