/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : packet_mgr.h
* Hackeroos          : deh
* Date First Issued  : 12/23/2012
* Board              : STM32F103VxT6_pod_mm
* Description        : Routines for managing sensor packet buffers
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SENSOR_PACKET_MGR
#define __SENSOR_PACKET_MGR
#include "../libopenstm32/common.h"	// Has things like 'u16' defined
#include "common_lsensormisc.h"
#include "packet_compression.h"
#include "common_misc.h"
#include "common_can.h"

#define NUMPACKETBUF	4	// Number of packets buffered for one CAN ID
#define MAXNUMCANIDBUF	20	// Max allowable msg ID's allowed 

/*******************************************************************************/
void packet_mgr_add(struct CANRCVTIMBUF * pcan);
/* @brief 	: Buffer a reading
 * @param	: pcan--pointer to struct with: time, can ID, can data
 * @return	: None
*******************************************************************************/
struct PKT_PTR packet_mgr_get_next(void);
/* @brief 	: Get a point & ct to a packet buffer that is complete
 * @param	: pb--pointer to control block
 * @return	: pointer & ct: NULL & 0 = No data ready
*******************************************************************************/

#endif 
