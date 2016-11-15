/******************************************************************************
* File Name          : can_cnvt.h
* Date First Issued  : 09/09/2014
* Board              : Discovery F4
* Description        : Convert payload bytes to/from shorts and longs
*******************************************************************************/


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAN_CNVT
#define __CAN_CNVT

#include "libopencm3/cm3/common.h"
#include "common_can.h"

/****************************************************************************** */
u16 can_cnvt_u16(struct CANRCVBUF* pcan, unsigned int n);
s16 can_cnvt_s16(struct CANRCVBUF* pcan, unsigned int n);
/* @brief 	: Convert bytes from payload to short
 * @param	: pcan = pointer to CAN msg 
 * @param	: n = start byte in payload 
 * @return	: short
*******************************************************************************/
u32 can_cnvt_u32(struct CANRCVBUF* pcan, unsigned int n);
s32 can_cnvt_s32(struct CANRCVBUF* pcan, unsigned int n);
/* @brief 	: Convert bytes from payload to int
 * @param	: pcan = pointer to CAN msg 
 * @param	: n = start byte in payload 
 * @return	: int
*******************************************************************************/
void can_cnvt_put_16(struct CANRCVBUF* pcan, unsigned int n, u16 w);
/* @brief 	: Store word in payload (unsigned, signed)
 * @param	: pcan = pointer to CAN msg 
 * @param	: n = start byte in payload 
 * @param	: w = short or unsigned short to be stored
*******************************************************************************/
void can_cnvt_put_32(struct CANRCVBUF* pcan, unsigned int n, u32 w);
/* @brief 	: Store word in payload (unsigned, signed)
 * @param	: pcan = pointer to CAN msg 
 * @param	: n = start byte in payload 
 * @param	: w = int or unsigned int to be stored
*******************************************************************************/

#endif

