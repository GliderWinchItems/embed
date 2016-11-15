/******************************************************************************
* File Name          : canmsg_send_pay_type.h
* Date First Issued  : 05/13/1025
* Board              : F103
* Description        : Send payload by payload type
*******************************************************************************/
#include <stdint.h>

#ifndef __CANSENDPAYTYPE
#define __CANSENDPAYTYPE

/******************************************************************************/
int canmsg_send_pay_type_FF(struct CANRCVBUF* p, float f);
/* @brief 	: Convert payload & send (assume CAN id already set)
 * @param	: f = float to be converted
 * @return	: 0 = OK; nonzero = error
*******************************************************************************/
int canmsg_send_pay_type_U32_U32toU32_U32(struct CANRCVBUF* p, uint32_t ui1, uint32_t ui2);
/* @brief 	: Convert payload & send (assume CAN id already set)
 * @param	: ui1 = [0]-[3]
 * @param	: ui2 = [4]-[7]
 * @return	: 0 = OK; nonzero = error
*******************************************************************************/
int canmsg_send_pay_type_S32_S32toS32_S32(struct CANRCVBUF* p, int32_t i1, int32_t i2);
/* @brief 	: Convert payload & send (assume CAN id already set)
 * @param	: i1 = [0]-[3]
 * @param	: i2 = [4]-[7]
 * @return	: 0 = OK; nonzero = error
*******************************************************************************/
int canmsg_send_pay_type_U16toU16(struct CANRCVBUF* p, uint16_t us);
/* @brief 	: Convert payload & send (assume CAN id already set)
 * @param	: us = [0]-[1]
 * @return	: 0 = OK; nonzero = error
*******************************************************************************/
int canmsg_send_pay_type_S16toS16(struct CANRCVBUF* p, int16_t ss);
/* @brief 	: Convert payload & send (assume CAN id already set)
 * @param	: ss = [0]-[1]
 * @return	: 0 = OK; nonzero = error
*******************************************************************************/
int canmsg_send_pay_type_U32toU32(struct CANRCVBUF* p, uint32_t ui);
/* @brief 	: Convert payload & send (assume CAN id already set)
 * @param	: UI = [0]-[3]
 * @return	: 0 = OK; nonzero = error
*******************************************************************************/

#endif

