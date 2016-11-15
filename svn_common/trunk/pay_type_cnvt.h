/******************************************************************************
* File Name          : pay_type_cnvt.h
* Date First Issued  : 03/23/2015
* Board              : little endian
* Description        : Payload type conversion routines
*******************************************************************************/

#ifndef __PAYTYPECONVERT
#define __PAYTYPECONVERT

#include <stdint.h>
#include "common_can.h"

/* structs for returning multi-value payload conversions. */
struct TWOFLOATS
{
	float	f1;
	float	f2;
};
struct TWOUINT32_T
{
	uint32_t ui1;
	uint32_t ui2;
};
struct TWOINT32_T
{
	int32_t i1;
	int32_t i2;
};
struct U8_U32
{
	uint8_t   u8;
	uint32_t  u32;
};
struct U8_S32
{
	uint8_t   u8;
	int32_t  s32;
};
struct U8_U8_U32
{
	uint8_t   u8a;
	uint8_t   u8b;
	uint32_t  u32;
};
struct U8_U8_S32
{
	uint8_t   u8a;
	uint8_t   u8b;
	int32_t  s32;
};
struct U8_U8_FF
{
	uint8_t   u8a;
	uint8_t   u8b;
	float	  f;
};

/******************************************************************************/
void pay_type_cnvt_FloattoPay_FF(uint8_t *p, float f);
/* @brief 	: Convert float to payload type FF
 * @param	: p = pointer to payload start byte 
 * @param	: f = float to be converted
*******************************************************************************/
float pay_type_cnvt_Pay_FFtoFloat(uint8_t *p);
/* @brief 	: Convert payload type FF to float
 * @param	: p = pointer to payload start byte 
 * @return	: float
*******************************************************************************/
void pay_type_cnvt_FloattoPay_xxFF(uint8_t *p, float f);
/* @brief 	: Convert float to payload type xxFF
 * @param	: p = pointer to payload start byte 
 * @param	: f = float to be converted
*******************************************************************************/
float pay_type_cnvt_Pay_xxFFtoFloat(uint8_t *p);
/* @brief 	: Convert payload type xxFF to float
 * @param	: p = pointer to payload start byte 
 * @return	: float
*******************************************************************************/
void pay_type_cnvt_U32toPay_U32(uint8_t *p, uint32_t ui);
/* @brief 	: Convert uint32_t to payload type U32
 * @param	: p = pointer to payload start byte 
 * @param	: ui = unsigned int to be converted
*******************************************************************************/
uint32_t pay_type_cnvt_Pay_U32toU32(uint8_t *p);
/* @brief 	: Payload type U32 to uint32_t
 * @param	: p = pointer to payload start byte 
 * @return	: uint32_t
*******************************************************************************/
void pay_type_cnvt_S32toPay_S32(uint8_t *p, int32_t i);
/* @brief 	: Convert int32_t to payload type S32
 * @param	: p = pointer to payload start byte 
 * @param	: i = signed int to be converted
*******************************************************************************/
int32_t pay_type_cnvt_Pay_S32toS32(uint8_t *p);
/* @brief 	: Payload type S32 to int32_t
 * @param	: p = pointer to payload start byte 
 * @return	: int32_t
*******************************************************************************/
void pay_type_cnvt_U32toPay_xxU32(uint8_t *p, uint32_t ui);
/* @brief 	: Convert uint32_t to payload type xxU32
 * @param	: p = pointer to payload start byte 
 * @param	: ui = unsigned int to be converted
*******************************************************************************/
uint32_t pay_type_cnvt_Pay_xxU32toU32(uint8_t *p);
/* @brief 	: Payload type xxU32 to uint32_t/
 * @param	: p = pointer to payload start byte 
 * @return	: uint32_t
*******************************************************************************/
void pay_type_cnvt_S32toPay_xxS32(uint8_t *p, int32_t i);
/* @brief 	: Convert int32_t to payload type xxS32
 * @param	: p = pointer to payload start byte 
 * @param	: i = signed int to be converted
*******************************************************************************/
int32_t pay_type_cnvt_Pay_xxS32toS32(uint8_t *p);
/* @brief 	: Payload type xxU32 to uint32_t
 * @param	: p = pointer to payload start byte 
 * @return	: int32_t
*******************************************************************************/
void pay_type_cnvt_FF_FFtoPay_FF_FF(uint8_t *p, float f1, float f2);
/* @brief 	: Convert two Full float to payload type FF_FF
 * @param	: p = pointer to payload start byte 
 * @param	: f1 = float that goes in payload [0]-[3]
 * @param	: f2 = float that goes in payload [4]-[7]
*******************************************************************************/
struct TWOFLOATS pay_type_cnvt_Pay_FF_FFtoFF_FF(uint8_t *p);
/* @brief 	: Convert payload type FF_FF to two Full Float
 * @param	: p = pointer to payload start byte 
 * @return	: struct with--
 * 		: struct.f1 = float from payload [0]-[3]
 * 		: struct.f2 = float from payload [4]-[7]
*******************************************************************************/
void pay_type_cnvt_U32_U32toPay_U32_U32(uint8_t *p, uint32_t ui1, uint32_t ui2);
/* @brief 	: Convert two uint32_t to payload type U32_U32
 * @param	: p = pointer to payload start byte 
 * @param	: ui1 = uint32_t that goes in payload [0]-[3]
 * @param	: ui2 = uint32_t that goes in payload [4]-[7]
*******************************************************************************/
struct TWOUINT32_T pay_type_cnvt_Pay_U32_U32toU32_U32(uint8_t *p);
/* @brief 	: Convert payload type U32_U32 to two uint32_t
 * @param	: p = pointer to payload start byte 
 * @return	: struct with--
 * 		: struct.ui1 = uint32_t from payload [0]-[3]
 * 		: struct.ui2 = uint32_t from payload [4]-[7]
*******************************************************************************/
void pay_type_cnvt_S32_S32toPay_S32_S32(uint8_t *p, int32_t i1, int32_t i2);
/* @brief 	: Convert two int32_t to payload type S32_S32
 * @param	: p = pointer to payload start byte 
 * @param	: i1 = int32_t that goes in payload [0]-[3]
 * @param	: i2 = int32_t that goes in payload [4]-[7]
*******************************************************************************/
struct TWOINT32_T pay_type_cnvt_Pay_S32_S32toS32_S32(uint8_t *p);
/* @brief 	: Convert payload type S32_S32 to two int32_t
 * @param	: p = pointer to payload start byte 
 * @return	: struct with--
 * 		: struct.i1 = int32_t from payload [0]-[3]
 * 		: struct.i2 = int32_t from payload [4]-[7]
*******************************************************************************/
void pay_type_cnvt_U8_U32toPay_U8_U32(uint8_t *p, uint8_t u8, uint32_t u32);
/* @brief 	: Convert uint8_t and uint32_t to payload type U8_U32
 * @param	: p = pointer to payload start byte 
 * @param	: u8  = uint8_t  that goes in payload [0]
 * @param	: u32 = uint32_t that goes in payload [1]-[4]
*******************************************************************************/
struct U8_U32 pay_type_cnvt_Pay_U8_U32toU8_U32(uint8_t *p);
/* @brief 	: Convert payload type U8_U32 to uint9_t and uint32_t
 * @param	: p = pointer to payload start byte 
 * @return	: struct with--
 * 		: struct.u8  = uint8_t  from payload [0]
 * 		: struct.u32 = uint32_t from payload [1]-[4]
*******************************************************************************/
void pay_type_cnvt_U8_S32toPay_U8_S32(uint8_t *p, uint8_t u8, int32_t s32);
/* @brief 	: Convert uint8_t and int32_t to payload type U8_S32
 * @param	: p = pointer to payload start byte 
 * @param	: u8  = uint8_t that goes in payload [0]
 * @param	: s32 = int32_t that goes in payload [1]-[4]
*******************************************************************************/
struct U8_S32 pay_type_cnvt_Pay_U8_S32toU8_S32(uint8_t *p);
/* @brief 	: Convert payload type U8_S32 to uint8_t and int32_t
 * @param	: p = pointer to payload start byte 
 * @return	: struct with--
 * 		: struct.u8  = uint8_t from payload [0]
 * 		: struct.u32 = int32_t from payload [1]-[4]
*******************************************************************************/
void pay_type_cnvt_U8_U8_U32toPay_U8_U8_U32(uint8_t *p, uint8_t u8a,  uint8_t u8b, uint32_t u32);
/* @brief 	: Convert uint8_t, uint8_t and uint32_t to payload type U8_U32
 * @param	: p = pointer to payload start byte 
 * @param	: u8a  = uint8_t  that goes in payload [0]
 * @param	: u8b  = uint8_t  that goes in payload [1]
 * @param	: u32 = uint32_t  that goes in payload [2]-[5]
*******************************************************************************/
struct U8_U8_U32 pay_type_cnvt_Pay_U8_U8_U32toU8_U8_U32(uint8_t *p);
/* @brief 	: Convert payload type U8_U8_U32 to uint8_t, uint8_t and uint32_t
 * @param	: p = pointer to payload start byte 
 * @return	: struct with--
 * 		: struct.u8a  = uint8_t  from payload [0]
 * 		: struct.u8b  = uint8_t  from payload [1]
 * 		: struct.u32 = uint32_t from payload [2]-[5]
*******************************************************************************/
void pay_type_cnvt_U8_U8_S32toPay_U8_U8_S32(uint8_t *p, uint8_t u8a,  uint8_t u8b, int32_t s32);
/* @brief 	: Convert uint8_t uint8_t and uint32_t to payload type U8_U8_U32
 * @param	: p = pointer to payload start byte 
 * @param	: u8a  = uint8_t  that goes in payload [0]
 * @param	: u8b  = uint8_t  that goes in payload [1]
 * @param	: s32  = int32_t  that goes in payload [2]-[5]
*******************************************************************************/
struct U8_U8_S32 pay_type_cnvt_Pay_U8_U8_S32toU8_U8_U32(uint8_t *p);
/* @brief 	: Convert payload type U8_U8_S32 to uint8_t, uint8_t and int32_t
 * @param	: p = pointer to payload start byte 
 * @return	: struct with--
 * 		: struct.u8a  = uint8_t  from payload [0]
 * 		: struct.u8b  = uint8_t  from payload [1]
 * 		: struct.s32 =  int32_t from payload [2]-[5]
*******************************************************************************/
void pay_type_cnvt_U8_U8_FFtoPay_U8_U8_FF(uint8_t *p, uint8_t u8a,  uint8_t u8b, float f);
/* @brief 	: Convert uint8_t uint8_t and float to payload type U8_U8_FF
 * @param	: p = pointer to payload start byte 
 * @param	: u8a  = uint8_t  that goes in payload [0]
 * @param	: u8b  = uint8_t  that goes in payload [1]
 * @param	: f    = float   that goes in payload [2]-[5]
*******************************************************************************/
struct U8_U8_FF pay_type_cnvt_Pay_U8_U8_FFtoU8_U8_FF(uint8_t *p);
/* @brief 	: Convert payload type U8_U8_FF to uint8_t, uint8_t and float
 * @param	: p = pointer to payload start byte 
 * @return	: struct with--
 * 		: struct.u8a  = uint8_t  from payload [0]
 * 		: struct.u8b  = uint8_t  from payload [1]
 * 		: struct.     = float    from payload [2]-[5]
*******************************************************************************/
void pay_type_cnvt_FloattoPay_xFF(uint8_t *p, float f);
/* @brief 	: Convert float to payload type xFF
 * @param	: p = pointer to payload start byte 
 * @param	: f = float to be converted
*******************************************************************************/
float pay_type_cnvt_Pay_xFFtoFloat(uint8_t *p);
/* @brief 	: Convert payload type xFF to float
 * @param	: p = pointer to payload start byte 
 * @return	: float
*******************************************************************************/
float pay_type_cnvt_Pay_xFFtoFloat(uint8_t *p);
/* @brief 	: Convert payload type xFF to float
 * @param	: p = pointer to payload start byte 
 * @return	: float
*******************************************************************************/
void pay_type_cnvt_FloattoPay_HF(uint8_t *p, float f);
/* @brief 	: Convert float to payload type HF (Half Float)
 * @param	: p = pointer to payload start byte 
 * @param	: f = float to be converted
*******************************************************************************/
float pay_type_cnvt_Pay_HFtoFloat(uint8_t *p);
/* @brief 	: Convert payload type HF to float
 * @param	: p = pointer to payload start byte 
 * @return	: float
*******************************************************************************/
void pay_type_cnvt_FloattoPay_34F(uint8_t *p, float f);
/* @brief 	: Convert float to payload type 34F (3/4 Float)
 * @param	: p = pointer to payload start byte 
 * @param	: f = float to be converted
*******************************************************************************/
float pay_type_cnvt_Pay_34FtoFloat(uint8_t *p);
/* @brief 	: Convert payload type 34F to float
 * @param	: p = pointer to payload start byte 
 * @return	: float
*******************************************************************************/
void pay_type_cnvt_U16toPay_U16(uint8_t *p, uint16_t us);
/* @brief 	: Convert uint16_t to payload type U16 (one unsigned short)
 * @param	: p  = pointer to payload start byte 
 * @param	: us = unsigned short to be converted
*******************************************************************************/
uint16_t pay_type_cnvt_Pay_U16toU16(uint8_t *p);
/* @brief 	: Payload type U16 to uint16_t (one unsigned short)
 * @param	: p = pointer to payload start byte 
 * @return	: uint16_t
*******************************************************************************/
void pay_type_cnvt_S16toPay_S16(uint8_t *p, int16_t us);
/* @brief 	: Convert uint16_t to payload type S16 (one signed short)
 * @param	: p  = pointer to payload start byte 
 * @param	: us = signed short to be converted
*******************************************************************************/
int16_t pay_type_cnvt_Pay_S16toS16(uint8_t *p);
/* @brief 	: Payload type S16 to int16_t (one signed short)
 * @param	: p = pointer to payload start byte 
 * @return	: int16_t
*******************************************************************************/

#endif
