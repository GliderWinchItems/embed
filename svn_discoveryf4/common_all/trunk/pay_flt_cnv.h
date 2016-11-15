/******************************************************************************
* File Name          : pay_flt_cnv.h
* Date First Issued  : 01/15/015
* Board              : little endian
* Description        : Conversion to/from float to half-float IEEE-754
*******************************************************************************/

#ifndef __PAYTOLOATCNV
#define __PAYTOLOATCNV

#include <stdint.h>

//#include "common.h"


/******************************************************************************/
float payhalffptofloat(uint8_t *p);
/* @brief 	: Convert CAN payload bytes holding half-float to float
 * @param	: p = pointer to payload start byte 
 * @return	: float
******************************************************************************/
void floattopayhalffp(uint8_t *p, float f);
/* @brief 	: Convert float to CAN payload bytes holding half-float representation
 * @param	: p = pointer to payload start byte 
 * @param	: f = single precision float to be converted to payload
******************************************************************************/
void floattopay3qtrfp(uint8_t *p, float f);
/* @brief 	: Convert float to CAN payload bytes holding 3/4 fp representation
 * @param	: p = pointer to payload start byte 
 * @param	: f = single precision float to be converted to payload
******************************************************************************/
float pay3qtrfptofloat(uint8_t *p);
/* @brief 	: Convert CAN payload bytes holding 3/4 fp representation to float
 * @param	: p = pointer to payload start byte 
 * @return	: float
*******************************************************************************/
void floattopaysinglefp(uint8_t *p, float f);
/* @brief 	: Convert float to CAN payload bytes holding single precision representation
 * @param	: p = pointer to payload start byte 
 * @param	: f = single precision float to be converted to payload
*******************************************************************************/
float paysinglefptofloat(uint8_t *p);
/* @brief 	: Convert float to CAN payload bytes holding single precision representation
 * @param	: p = pointer to payload start byte 
 * @return	: float
*******************************************************************************/

#endif
