/******************************************************************************
* File Name          : pay_fIX_cnv.h
* Date First Issued  : 01/15/015
* Board              : little endian
* Description        : Conversion to/from fixed types to payload bytes
*******************************************************************************/

#ifndef __PAYTOFIXCNV
#define __PAYTOFIXCNV

#include <stdint.h>

//#include "common.h"


/******************************************************************************/
void int32_ttopay(uint8_t *p, int32_t i);
/* @brief 	: Convert int32_t to CAN payload bytes
 * @param	: p = pointer to payload start byte 
 * @param	: i = int to be converted
*******************************************************************************/
int32_t paytoint32_t(uint8_t *p);
/* @brief 	: Convert CAN payload bytes holding a 4 bytes to an int32_t
 * @param	: p = pointer to payload start byte 
 * @return	: int
*******************************************************************************/
void uint32_ttopay(uint8_t *p, uint32_t i);
/* @brief 	: Convert uint32_t to CAN payload bytes
 * @param	: p = pointer to payload start byte 
 * @param	: i = int to be converted
*******************************************************************************/
uint32_t paytouint32_t(uint8_t *p);
/* @brief 	: Convert CAN payload bytes holding a 4 bytes to a uint32_t
 * @param	: p = pointer to payload start byte 
 * @return	: uint32_t
*******************************************************************************/
void int16_ttopay(uint8_t *p, int16_t i);
/* @brief 	: Convert int16_t to CAN payload bytes
 * @param	: p = pointer to payload start byte 
 * @param	: i = int to be converted
*******************************************************************************/
int paytoint16_t(uint8_t *p);
/* @brief 	: Convert CAN payload bytes holding a 4 bytes to an int16_t
 * @param	: p = pointer to payload start byte 
 * @return	: int
*******************************************************************************/
void uint16_ttopay(uint8_t *p, uint16_t i);
/* @brief 	: Convert int16_t to CAN payload bytes
 * @param	: p = pointer to payload start byte 
 * @param	: i = int to be converted
*******************************************************************************/
int paytouint32_t(uint8_t *p);
/* @brief 	: Convert CAN payload bytes holding a 4 bytes to an uint_t
 * @param	: p = pointer to payload start byte 
 * @return	: int
*******************************************************************************/
void uint32_ttopay3bytes(uint8_t *p, uint32_t ui);
/* @brief 	: Convert uint32_t to three CAN payload bytes
 * @param	: p = pointer to payload start byte 
 * @param	: ui = unsigned int to be converted
*******************************************************************************/
uint32_t pay3bytestouint32_t(uint8_t *p);
/* @brief 	: Convert CAN payload bytes holding a 3 bytes to an uint32_t
 * @param	: p = pointer to payload start byte 
 * @return	: int
*******************************************************************************/
void int32_ttopay3bytes(uint8_t *p, int32_t i);
/* @brief 	: Convert int32_t to three CAN payload bytes
 * @param	: p = pointer to payload start byte 
 * @param	: i = signed int to be converted
*******************************************************************************/
int pay3bytestouint32_t(uint8_t *p);
/* @brief 	: Convert CAN payload bytes holding a 3 bytes to an uint32_t
 * @param	: p = pointer to payload start byte 
 * @return	: int
*******************************************************************************/


#endif
