/******************************************************************************
* File Name          : cancmd_paycnvt.h
* Date First Issued  : 03/01/2015
* Board              :
* Description        : 
*******************************************************************************/
#include "tmpstruct.h"
#include "common_can.h"

#ifndef __CMD_PAYCNVT
#define __CMD_PAYCNVT

/******************************************************************************/
int32_t cancmd_paycnvt_to(struct CANRCVBUF* pcan, void* pele, uint32_t id);
/* @brief	: Convert parameter element to CAN msg payload bytes
 * @param	: pcan = pointer in CAN msg
 * @param	: pele = pointer to element in struct
 * @param	: id = parameter id number
 ******************************************************************************/
int32_t cancmd_paycnvt_from(struct CANRCVBUF* pcan, void* pele, uint32_t id);
/* @brief	: Convert CAN msg payload bytes from parameter to element in struct
 * @param	: pcan = pointer in CAN msg
 * @param	: pele = pointer to element in struct
 * @param	: id = parameter id number
 * @return	: 0 = OK;
 ******************************************************************************/

#endif
