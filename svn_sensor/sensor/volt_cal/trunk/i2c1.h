/******************************************************************************
* File Name          : i2c1.h
* Date First Issued  : 10/27/2015
* Board              : 
* Description        : I2C for F103 (and others?)
*******************************************************************************/
/*

*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __I2C1_1
#define __I2C1_1

#include <stdint.h>

#define IC2BUFFERSIZE	2048	// Size of circular buffer for I2C sending
#define NVIC_I2C1_EV_IRQ_PRIORITY 0x90		// Priority for I2C1 EV

/******************************************************************************/
void i2c1_vcal_init(uint32_t sclclockrate);
/* @brief	: Initialize i2c1 for standard mode I2C
 * @param	: sclclockrate = SCL rate (Hz)
 ******************************************************************************/
void i2c1_vcal_start (uint16_t address);
/* @brief	: Address unit and start continuous write stream
 * @param	: address = I2C bus address of unit (NOT shifted for LSB R/W bit)
*******************************************************************************/
void i2c1_vcal_push(void* pnexti2 );
/* @brief	: Stack next function pointers
 * @param	: pnexti2 = pointer to next function
*******************************************************************************/
uint16_t i2c1_vcal_putbuf(uint8_t* p, uint16_t count);
/* @brief	: Load bytes into circular buffer
 * @param	: p = pointer to bytes to be loaded
 * @param	: count = number of bytes to load
 * @return	: count of number loaded
*******************************************************************************/
uint8_t i2c1_vcal_bufempty(void);
/* @brief	: Check if buffer is empty (all bytes sent)
 * @return	: 0 = empty; not zero = buf still has bytes to be sent
*******************************************************************************/



#endif

