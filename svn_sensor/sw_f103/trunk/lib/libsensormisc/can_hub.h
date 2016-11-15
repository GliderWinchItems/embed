/******************************************************************************
* File Name          : can_hub.h
* Date First Issued  : 04/19/2015
* Board              : stm32
* Description        : hub-server for CAN functions
*******************************************************************************/

#ifndef __CAN_HUB
#define __CAN_HUB

#include "../../../../../svn_common/trunk/common_can.h"
#include <stdint.h>
#include "common_misc.h"

#define CAN_HUB_BUFFSIZE	32	// Number of CAN msgs in a port buffer

/* Pointers and buffer for each port participating in the hub. */
struct CANHUB 
{
	struct CANHUB* pnext;		// Pointer to previous struct in list
	struct CANRCVBUF* pPut;		// Pointer for adding msgs
	struct CANRCVBUF* pGet;		// Pointer for removing msgs
	struct CANRCVBUF* pEnd;		// Pointer to "end" of buffer
	struct CANRCVBUF can[CAN_HUB_BUFFSIZE];
};

/******************************************************************************/
struct CANHUB* can_hub_add_func(void);
/* @brief	: Add a function port 
 * @return	: Return pointer to the port buffer/struct; zero = failed.
 ******************************************************************************/
void can_hub_begin(void);
/* @brief	: Add any CAN msgs coming in from CAN hardware to local buffers
 ******************************************************************************/
void can_hub_send(struct CANRCVBUF* pcan, struct CANHUB* pptr);
/* @brief	: Send CAN msg into hub
 * @param	: pcan = pointer to CAN msg to be sent
 * @param	: pprt = hub port buffer struct pointer
 ******************************************************************************/
struct CANRCVBUF* can_hub_get( struct CANHUB* pptr);
/* @brief	: Send a CAN msg
 * @param	: port = port struct pointer
 * @return	: pprt = hub port buffer struct pointer
 ******************************************************************************/
uint32_t can_hub_end(void);
/* @brief	: return if any msgs added
 * @return	: 0 = none added; 1 = might need another pass
 ******************************************************************************/

#endif

