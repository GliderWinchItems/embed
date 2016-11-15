/******************************************************************************
* File Name          : hd44780.h
* Date First Issued  : 10/30/2015
* Board              : 
* Description        : LCD manipulation via PCF8574 I2C to parallel
*******************************************************************************/
/*

*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HC44780_1
#define __HC44780_1

#include <stdint.h>

/******************************************************************************/
void hd44780_init(uint8_t address);
/* @brief	: Init I2C and LCD
 * @param	: address = I2C address (not including R/W bit)
*******************************************************************************/
void sendLCDchar(uint8_t data);
/* @brief	: Send ?
 * @param	: data = byte to be sent (in 4 bit mode)
*******************************************************************************/
void hd44780_home(void);
/* @brief	: Return cursor to home position
*******************************************************************************/
void hd44780_setline(uint8_t line);
/* @brief	: Set cursor at beginning of line
 * @param	: line = line number (0 or 1)
*******************************************************************************/
void hd44780_backlight(uint8_t onoff);
/* @brief	: Set back light (is set at next char written)
 * @param	: onoff: 0 = off; not zero = on
*******************************************************************************/
void hd44780_puts(char* p, uint8_t line);
/* @brief	: Load string into LCD line
 * @param	: p = pointer to char string (zero terminated)
 * @param	: line = line number (0 or 1)
*******************************************************************************/

#endif

