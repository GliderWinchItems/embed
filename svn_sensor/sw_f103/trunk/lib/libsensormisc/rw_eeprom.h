/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : rw_eeprom.h
* Authorship         : deh
* Date First Issued  : 05/25/2013
* Board              : ../svn_sensor/hw/trunk/eagle/f103R/RxT6
* Description        : Handles sequence for read/writing eeprogm (using spi1eeprom.c)
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RWEEPROM
#define __RWEEPROM

/* Includes ------------------------------------------------------------------*/
#include "libopenstm32/common.h"	// Has things like 'u16' defined

#define EEPROM_BLOCKSIZE	

/******************************************************************************/
int rw_eeprom_init(void);
/*  @return	: zero = success; not zero = failed
 *  @brief 	: Initialize SPI1 for eeprom
*******************************************************************************/
int eeprom_read(u16 address, char *p, int count);
/* @brief	: Read from eeprom
 * @param	: address--eeprom address to start read (0-8191)
 * @param	: p--pointer to buffer 
 * @param	: count--number of bytes to read (1-8192)
 * @return	: zero = OK. not zero = failed
******************************************************************************/
int eeprom_write(u16 address, char *p, int count);
/* @brief	: Write to eeprom
 * @param	: address--eeprom address to start write (0-8191)
 * @param	: p--pointer to buffer 
 * @param	: count--number of bytes to write (1-8192)
 * @return	: zero = OK. not zero = failed
******************************************************************************/
int eeprom_busy(void);
/* @brief	: Check if a read or write operation is in progress
 * @return	: zero = not busy. not zero = busy;
******************************************************************************/
unsigned int eeprom_read_status_reg(void);
/* @brief	: Read the status register
 * @return	: high byte = 0 for OK, 0xff bad; low byte = status register
******************************************************************************/
unsigned int eeprom_write_status_reg(unsigned char new);
/* @brief	: Write a byte to the status register
 * @return	: 0 = OK; 0xff00 busy; 0xfe00 = timedout
******************************************************************************/
unsigned int eeprom_write_enable_latch(char onoff);
/* @brief	: Set write-enable-latch on, or off
 * @param	: onoff: 0 = off, not zero = on
 * @return	: 0 = OK; 0xff00 busy; 0xfe00 = timedout
******************************************************************************/


extern	char eeprom_status_register;	// Status register (last read)
extern	unsigned int debug1; 		// Count busy loops for a write

#endif

