/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : eeprom_can.c
* Author             : deh
* Date First Issued  : 07/25/2013
* Board              : RxT6
* Description        : eeprom use for CAN
*******************************************************************************/
#include "eeprom_can.h"
#include "rw_eeprom.h"


/* **************************************************************************************
 * struct TWO32 eeprom_get(u32 offset);
 * @brief	: retrieve a 32b word from the eeprom
 * @param	; offset = byte offset from beginning of eeprom
 * @return	: a = word; b = complement of word
 * ************************************************************************************** */
struct TWO32 eeprom_get(u32 offset)
{
	int eeprom_ret;
	struct TWO32 x;

	while ((eeprom_ret=eeprom_busy()) > 0);  // Wait for sequence to complete
	eeprom_ret=eeprom_read(offset,(char*)&x, 8); // Read 32b value and complement
	while ((eeprom_ret=eeprom_busy()) > 0); // Wait for sequence to complete


	return x;
}
/* **************************************************************************************
 * int eeprom_put(u32 offset,u32 *p);
 * @brief	: write a 32b word and its complement
 * @param	: offset = byte count offset from beginning of eeprom
 * @param	: p = pointer to word to be written
 * @return	: 0 = OK; not 0 = bad
 * ************************************************************************************** */
int eeprom_put(u32 offset,u32 *p)
{
	int eeprom_ret;
	struct TWO32 x;
	x.a.u32 = *p;
	x.b.u32 = ~x.a.u32;

	eeprom_ret=eeprom_write(offset,(char *)&x, 8);	// Write word and complement
	while ((eeprom_ret=eeprom_busy()) > 0); // Wait for sequence to complete

	return eeprom_ret;
}

